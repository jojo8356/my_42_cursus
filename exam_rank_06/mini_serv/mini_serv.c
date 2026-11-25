/*
 * mini_serv.c — 42 Exam Rank 06 (v2025)
 *
 * A non-blocking, multi-client TCP chat server (a simplified IRC server).
 * Clients connect on 127.0.0.1:<port>, are assigned incrementing ids, and
 * can broadcast messages to every other connected client.
 *
 * Implementation notes:
 *   - I/O multiplexing with select() (no threads, no poll()).
 *   - Per-client accumulation buffer so partial / fragmented messages are
 *     reassembled and lazy clients are never disconnected.
 *   - No #define preprocessor directives (subject constraint).
 *   - Only the allowed functions are used (see subject.txt).
 */

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/select.h>

/* ---- Global state -------------------------------------------------------- */

/* Maximum number of tracked clients (fd_set is limited to FD_SETSIZE). */
static int g_max_clients = 1024;

/* Master set of connected file descriptors (listening socket + clients). */
static fd_set g_master;
/* Snapshot used by select(). */
static fd_set g_read_set;

/* Per-client state, indexed by file descriptor. */
static int *g_ids;        /* assigned client id, -1 when not a client */
static char **g_bufs;     /* accumulation buffer for incomplete messages */
static int g_max_fd = 0;  /* highest file descriptor currently in use */
static int g_next_id = 0; /* next id to hand out to a new client */

/* Temporary scratch buffers. */
static char g_read_buf[1001];
static char g_write_buf[64];

/* ---- Small helpers ------------------------------------------------------- */

static void fatal_error(void)
{
    write(2, "Fatal error\n", 12);
    exit(1);
}

/* Send a message to every connected client except the author. */
static void broadcast_except(int author, const char *str)
{
    int len = (int)strlen(str);
    int fd;

    for (fd = 0; fd < g_max_clients; fd++)
    {
        if (fd == author)
            continue;
        if (g_ids[fd] != -1 && FD_ISSET(fd, &g_master))
            send(fd, str, len, 0);
    }
}

/*
 * Extract complete, newline-terminated messages from a client accumulation
 * buffer. For every extracted line, returns 1 and points *msg at a freshly
 * allocated NUL-terminated string (without the trailing newline). Returns 0
 * when no complete message is available, -1 on allocation failure.
 */
static int extract_message(char **buf, char **msg)
{
    char *newbuf;
    int i;

    *msg = NULL;
    if (*buf == NULL)
        return 0;

    i = 0;
    while ((*buf)[i] != '\0')
    {
        if ((*buf)[i] == '\n')
        {
            /* Everything after the newline stays in the buffer. */
            newbuf = calloc(1, strlen(*buf + i + 1) + 1);
            if (newbuf == NULL)
                return -1;
            strcpy(newbuf, *buf + i + 1);
            (*buf)[i] = '\0';
            *msg = *buf;
            *buf = newbuf;
            return 1;
        }
        i++;
    }
    return 0;
}

/* Append freshly received data to a client's accumulation buffer. */
static char *append_buffer(char *buf, const char *data)
{
    char *newbuf;
    int old_len = (buf != NULL) ? (int)strlen(buf) : 0;
    int data_len = (int)strlen(data);

    newbuf = malloc((size_t)old_len + (size_t)data_len + 1);
    if (newbuf == NULL)
        return NULL;
    newbuf[0] = '\0';
    if (buf != NULL)
        strcat(newbuf, buf);
    strcat(newbuf, data);
    free(buf);
    return newbuf;
}

/* ---- Client lifecycle ---------------------------------------------------- */

static void register_client(int fd)
{
    int id = g_next_id++;

    g_ids[fd] = id;
    g_bufs[fd] = NULL;
    if (fd > g_max_fd)
        g_max_fd = fd;
    FD_SET(fd, &g_master);

    sprintf(g_write_buf, "server: client %d just arrived\n", id);
    broadcast_except(fd, g_write_buf);
}

static void remove_client(int fd)
{
    sprintf(g_write_buf, "server: client %d just left\n", g_ids[fd]);
    broadcast_except(fd, g_write_buf);

    free(g_bufs[fd]);
    g_bufs[fd] = NULL;
    g_ids[fd] = -1;
    FD_CLR(fd, &g_master);
    close(fd);
}

/* Broadcast all complete messages currently buffered for a client. */
static void flush_messages(int fd)
{
    char *msg = NULL;

    while (extract_message(&(g_bufs[fd]), &msg) == 1)
    {
        /* Build the full prefixed line: "client <id>: <line>\n" */
        char *full = malloc(strlen(msg) + 32);
        if (full == NULL)
        {
            free(msg);
            fatal_error();
        }
        sprintf(full, "client %d: %s\n", g_ids[fd], msg);
        free(msg);
        broadcast_except(fd, full);
        free(full);
    }
}

/* ---- Server setup -------------------------------------------------------- */

static int create_listening_socket(int port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servaddr;

    if (sockfd < 0)
        fatal_error();

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    /* 127.0.0.1 as a 32-bit integer (0x7F000001). */
    servaddr.sin_addr.s_addr = htonl(2130706433);
    servaddr.sin_port = htons((uint16_t)port);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        fatal_error();
    if (listen(sockfd, 10) < 0)
        fatal_error();

    return sockfd;
}

/* ---- Entry point --------------------------------------------------------- */

int main(int ac, char **av)
{
    int sockfd;

    /* Argument validation. */
    if (ac != 2)
    {
        write(2, "Wrong number of arguments\n", 26);
        exit(1);
    }

    /* Allocate per-client tables. */
    g_ids = calloc((size_t)g_max_clients, sizeof(int));
    g_bufs = calloc((size_t)g_max_clients, sizeof(char *));
    if (g_ids == NULL || g_bufs == NULL)
        fatal_error();
    int i;
    for (i = 0; i < g_max_clients; i++)
    {
        g_ids[i] = -1;
        g_bufs[i] = NULL;
    }

    FD_ZERO(&g_master);
    sockfd = create_listening_socket(atoi(av[1]));
    FD_SET(sockfd, &g_master);
    if (sockfd > g_max_fd)
        g_max_fd = sockfd;

    /* Main event loop. */
    while (1)
    {
        g_read_set = g_master;

        if (select(g_max_fd + 1, &g_read_set, NULL, NULL, NULL) < 0)
            fatal_error();

        int fd;
        for (fd = 0; fd <= g_max_fd; fd++)
        {
            if (!FD_ISSET(fd, &g_read_set))
                continue;

            if (fd == sockfd)
            {
                /* New client connection. */
                struct sockaddr_in clientaddr;
                socklen_t addr_len = sizeof(clientaddr);
                int client_fd = accept(sockfd,
                                       (struct sockaddr *)&clientaddr,
                                       &addr_len);
                if (client_fd < 0)
                    continue;
                register_client(client_fd);
            }
            else
            {
                /* Existing client sent data. */
                int read_bytes = recv(fd, g_read_buf, 1000, 0);
                if (read_bytes <= 0)
                {
                    remove_client(fd);
                }
                else
                {
                    g_read_buf[read_bytes] = '\0';
                    char *merged = append_buffer(g_bufs[fd], g_read_buf);
                    if (merged == NULL)
                        fatal_error();
                    g_bufs[fd] = merged;
                    flush_messages(fd);
                }
            }
        }
    }

    /* Unreachable, but keep the compiler happy. */
    return 0;
}
