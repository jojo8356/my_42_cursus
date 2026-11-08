/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minitalk.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: student <student@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/28 10:00:00 by student           #+#    #+#             */
/*   Updated: 2026/10/28 10:00:00 by student          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINITALK_H
# define MINITALK_H

# include <signal.h>
# include <stdlib.h>
# include <unistd.h>

/*
** ==========================================================================
** minitalk : protocole de communication par signaux UNIX (SIGUSR1/SIGUSR2)
** ==========================================================================
** Un signal ne transportant aucune donnee, chaque octet du message est
** envoye bit par bit : 8 signaux par octet, du bit de poids fort au bit de
** poids faible. SIGUSR1 vaut "1", SIGUSR2 vaut "0". La fin de message est
** signalee par un octet nul ('\0').
**
** Apres CHAQUE bit recu, le serveur renvoie SIGUSR1 au client (ACK de flux) :
** le client n'envoie jamais le bit suivant avant que le precedent soit bien
** arrive. C'est indispensable car Linux ne met PAS deux signaux de meme type
** en file d'attente : sans ce garde-fou, des bits seraient perdus.
**
** Bonus : le serveur renvoie SIGUSR2 (ACK de message) quand l'octet '\0'
** final a ete recu, ce qui prouve au client que le message entier est bien
** arrive. Les octets sont transmis bruts : l'UTF-8 (Unicode) passe sans
** alteration.
*/

/*
** Codes d'accuse de reception places dans g_protocol_state par le client :
**   ACK_NONE             : aucun accuse recu pour l'instant (attente en cours)
**   ACK_BIT_RECEIVED     : le serveur a recu le bit envoye (controle de flux)
**   ACK_MESSAGE_RECEIVED : le serveur a recu le message entier (bonus)
*/
# define ACK_NONE 0
# define ACK_BIT_RECEIVED 1
# define ACK_MESSAGE_RECEIVED 2

/*
** Representation de l'etat de reception dans g_protocol_state (serveur) :
**   bits 0-7  : octet en cours de reconstruction (RECEIVED_BYTE_MASK)
**   bits 8-10 : nombre de bits deja recus pour cet octet (0 a 7)
*/
# define BITS_PER_BYTE 8
# define RECEIVED_BYTE_MASK 255
# define BIT_COUNT_MASK 7
# define BIT_COUNT_SHIFT 8

/*
** Valeurs ASCII et arithmetique utilisees par les outils d'affichage/parsing.
*/
# define ASCII_ZERO '0'
# define ASCII_NINE '9'
# define DECIMAL_BASE 10
# define STDOUT_FD 1

/*
** Limites du parsing strict d'un PID (pid_t est un entier signe) :
**   PID_MIN_VALUE  : plus petit PID valide. 0 est INTERDIT car kill(0, ...)
**                    enverrait le signal a tout le groupe de processus, et
**                    les PID negatifs viseraient des groupes entiers aussi.
**   PID_MAX_VALUE  : valeur maximale d'un entier signe (INT_MAX) ; au dela,
**                    le nombre ne peut pas etre un PID.
**   OVERFLOW_GUARD : PID_MAX_VALUE / 10, verifie AVANT de multiplier par 10
**                    pour eviter tout depassement de capacite.
*/
# define PID_MIN_VALUE 1
# define PID_MAX_VALUE 2147483647
# define OVERFLOW_GUARD 214748364

/*
** Codes de retour du parsing strict d'un PID (ft_atoi_pid).
*/
# define PARSING_SUCCESS 0
# define PARSING_FAILURE -1

/*
** Code de retour des fonctions d'envoi (send_byte / send_message) :
** SEND_FAILURE (-1) si le serveur est injoignable (kill echoue) ou ne
** repond plus (delai depasse). Sinon, ces fonctions renvoient le code d'ACK
** recu pour le dernier bit (ACK_BIT_RECEIVED ou ACK_MESSAGE_RECEIVED).
*/
# define SEND_FAILURE -1

/*
** Octet de fin de message envoye par le client une fois la chaine transmise.
*/
# define MESSAGE_END_MARKER '\0'

/*
** Delais d'attente d'un accuse de reception cote client (wait_for_ack) :
**   ACK_SPIN_LIMIT    : iterations de veille active (chemin rapide : un ACK
**                       revient en quelques microsecondes)
**   ACK_POLL_LIMIT    : nombre d'appels a usleep() ensuite (chemin lent, si
**                       le systeme est charge)
**   ACK_POLL_DELAY_US : delai entre deux usleep(), en microsecondes
** Au-dela (~2 secondes), le serveur est considere comme mort et le client
** s'arrete avec une erreur plutot que de rester bloque.
*/
# define ACK_SPIN_LIMIT 1000000
# define ACK_POLL_LIMIT 2000
# define ACK_POLL_DELAY_US 1000

/*
** Position des arguments de la ligne de commande du client :
** ./client <server_pid> <message>
*/
# define EXPECTED_ARGC 3
# define PID_ARG_INDEX 1
# define MESSAGE_ARG_INDEX 2

/*
** ---------------------------------------------------------------------------
** g_protocol_state : UNIQUE variable globale de chaque programme.
** ---------------------------------------------------------------------------
** Client  : contient le code d'ACK renvoye par le serveur (cf. ACK_*).
** Serveur : contient la machine a etats de reception (cf. masques ci-dessus).
**
** Elle DOIT etre globale pour deux raisons :
**   1. un gestionnaire de signaux (sigaction / SA_SIGINFO) ne peut pas
**      recevoir de parametres supplementaires ;
**   2. l'etat doit survivre entre deux appels asynchrones du handler.
**
** volatile sig_atomic_t garantit des acces atomiques et interdit la mise en
** cache par le compilateur entre le flux principal et les handlers.
*/

extern volatile sig_atomic_t	g_protocol_state;

/* --- utils.c : affichage, parsing strict et sortie d'erreur ------------- */

size_t	ft_strlen(const char *string);
void	ft_putstr_fd(const char *string, int output_fd);
void	ft_putnbr_fd(int number, int output_fd);
int		ft_atoi_pid(const char *pid_string, pid_t *parsed_pid);
void	print_error_and_exit(const char *error_message);

/* --- transfer.c : envoi bit par bit avec controle de flux -------------- */

int		send_byte(pid_t server_pid, unsigned char byte_to_send);
int		send_message(pid_t server_pid, const char *message);

#endif
