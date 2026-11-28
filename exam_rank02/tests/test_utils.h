#ifndef TEST_UTILS_H
# define TEST_UTILS_H

# ifndef _POSIX_C_SOURCE
#  define _POSIX_C_SOURCE 200809L
# endif

# include <stdio.h>
# include <unistd.h>

/* Redirect stdout to a temporary file while testing functions that print. */
typedef struct s_test_capture
{
    FILE    *file;
    int     saved_stdout;
    char    *buffer;
    size_t  capacity;
}   t_test_capture;

static inline int test_capture_begin(t_test_capture *capture,
        char *buffer, size_t capacity)
{
    if (!capture || !buffer || capacity == 0)
        return (-1);
    capture->file = NULL;
    capture->saved_stdout = -1;
    capture->buffer = buffer;
    capture->capacity = capacity;
    buffer[0] = '\0';
    if (fflush(stdout) == EOF)
        return (-1);
    capture->file = tmpfile();
    if (!capture->file)
        return (-1);
    capture->saved_stdout = dup(STDOUT_FILENO);
    if (capture->saved_stdout < 0
        || dup2(fileno(capture->file), STDOUT_FILENO) < 0)
    {
        if (capture->saved_stdout >= 0)
            close(capture->saved_stdout);
        fclose(capture->file);
        capture->file = NULL;
        capture->saved_stdout = -1;
        return (-1);
    }
    return (0);
}

static inline long test_capture_end(t_test_capture *capture)
{
    long length;
    size_t bytes_read;
    int restore_error;

    if (!capture || !capture->file || capture->saved_stdout < 0)
        return (-1);
    fflush(stdout);
    restore_error = dup2(capture->saved_stdout, STDOUT_FILENO) < 0;
    close(capture->saved_stdout);
    capture->saved_stdout = -1;
    if (fseek(capture->file, 0, SEEK_END) != 0)
    {
        fclose(capture->file);
        capture->file = NULL;
        return (-1);
    }
    length = ftell(capture->file);
    if (length < 0 || (unsigned long)length >= capture->capacity)
    {
        capture->buffer[0] = '\0';
        fclose(capture->file);
        capture->file = NULL;
        return (-1);
    }
    rewind(capture->file);
    bytes_read = fread(capture->buffer, 1, (size_t)length, capture->file);
    capture->buffer[bytes_read] = '\0';
    fclose(capture->file);
    capture->file = NULL;
    if (restore_error || bytes_read != (size_t)length)
        return (-1);
    return ((long)bytes_read);
}

#endif
