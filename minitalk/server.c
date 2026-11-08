/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: student <student@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/28 10:00:00 by student           #+#    #+#             */
/*   Updated: 2026/10/28 10:00:00 by student          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minitalk.h"

/*
** print_received_byte : affiche l'octet reconstruit sur la sortie standard.
** L'octet MESSAGE_END_MARKER ('\0') ne s'affiche pas : il marque la fin du
** message, on saute simplement a la ligne. Les octets sont ecrits bruts via
** write() : l'UTF-8 (Unicode) s'affiche donc sans alteration.
*/

static void	print_received_byte(int byte_value)
{
	char	received_char;

	received_char = (char)byte_value;
	if (received_char == MESSAGE_END_MARKER)
		write(STDOUT_FD, "\n", 1);
	else
		write(STDOUT_FD, &received_char, 1);
}

/*
** handle_received_bit : gestionnaire appele pour CHAQUE signal recu, donc
** pour chaque bit du message.
** 1) le bit recu est ajoute a l'octet en cours (SIGUSR1 = 1, SIGUSR2 = 0),
**    en commencant par le bit de poids fort ;
** 2) apres BITS_PER_BYTE bits, l'octet est complet : on l'affiche puis on
**    remet la machine a etats a zero ; un octet nul termine le message ;
** 3) SIGUSR1 est renvoye a l'expediteur : c'est l'ACK de flux qui autorise
**    l'envoi du bit suivant (voir minitalk.h).
** L'etat vit dans g_protocol_state (variable globale unique) car un handler
** ne peut pas recevoir de parametres. write() et kill() sont les seules
** fonctions appelees : elles sont async-signal-safe.
*/

static void	handle_received_bit(int signal_id, siginfo_t *info, void *context)
{
	int	byte_in_progress;
	int	bits_received;

	(void)context;
	byte_in_progress = (g_protocol_state & RECEIVED_BYTE_MASK) << 1;
	byte_in_progress = byte_in_progress | (signal_id == SIGUSR1);
	byte_in_progress = byte_in_progress & RECEIVED_BYTE_MASK;
	bits_received = (g_protocol_state >> BIT_COUNT_SHIFT) & BIT_COUNT_MASK;
	bits_received++;
	if (bits_received == BITS_PER_BYTE)
	{
		print_received_byte(byte_in_progress);
		byte_in_progress = 0;
		bits_received = 0;
	}
	g_protocol_state = byte_in_progress | (bits_received << BIT_COUNT_SHIFT);
	if (info->si_pid > 0)
		kill(info->si_pid, SIGUSR1);
}

/*
** setup_signal_handlers : branche handle_received_bit sur SIGUSR1 et SIGUSR2.
** SA_SIGINFO fournit siginfo_t au handler, d'ou l'on recupere si_pid : le
** PID du client expediteur, indispensable pour lui renvoyer ses ACK (un
** serveur peut dialoguer avec plusieurs clients a la suite).
*/

static void	setup_signal_handlers(void)
{
	struct sigaction	signal_action;

	sigemptyset(&signal_action.sa_mask);
	sigaddset(&signal_action.sa_mask, SIGUSR1);
	sigaddset(&signal_action.sa_mask, SIGUSR2);
	signal_action.sa_flags = SA_SIGINFO;
	signal_action.sa_sigaction = handle_received_bit;
	if (sigaction(SIGUSR1, &signal_action, NULL) < 0)
		print_error_and_exit("sigaction failed");
	if (sigaction(SIGUSR2, &signal_action, NULL) < 0)
		print_error_and_exit("sigaction failed");
}

/*
** main : ./server
** 1) affiche le PID du serveur (c'est l'information que le client doit
**    recevoir en parametre) ;
** 2) installe les gestionnaires de signaux ;
** 3) attend les signaux en boucle jusqu'a l'arret du processus.
** Aucune allocation memoire : aucun leak possible. Plusieurs clients peuvent
** transmettre des messages a la suite sans relancer le serveur, chaque '\0'
** remettant la machine a etats a zero.
*/

int	main(void)
{
	pid_t	server_pid;

	server_pid = getpid();
	ft_putstr_fd("Server PID: ", STDOUT_FD);
	ft_putnbr_fd((int)server_pid, STDOUT_FD);
	ft_putstr_fd("\n", STDOUT_FD);
	setup_signal_handlers();
	while (1)
		pause();
	return (EXIT_SUCCESS);
}
