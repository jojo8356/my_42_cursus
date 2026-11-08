/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: student <student@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/28 10:00:00 by student           #+#    #+#             */
/*   Updated: 2026/10/28 10:00:00 by student          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minitalk.h"

/*
** handle_server_ack : gestionnaire appele a chaque accuse de reception recu.
** SIGUSR1 = ACK de bit (le serveur a recu le bit envoye),
** SIGUSR2 = ACK de message (le serveur bonus a recu le '\0' final).
** Ecrit le code recu dans g_protocol_state pour que wait_for_ack() sorte de
** sa boucle d'attente. Volontairement minimal : seules des operations
** async-signal-safe sont realisees (ici, aucune).
*/

static void	handle_server_ack(int signal_id, siginfo_t *info, void *context)
{
	(void)info;
	(void)context;
	g_protocol_state = ACK_BIT_RECEIVED;
	if (signal_id == SIGUSR2)
		g_protocol_state = ACK_MESSAGE_RECEIVED;
}

/*
** setup_signal_handlers : branche handle_server_ack sur SIGUSR1 et SIGUSR2.
** SA_SIGINFO demande au noyau de fournir siginfo_t au gestionnaire (inutilise
** cote client, mais indispensable cote serveur pour connaitre le PID de
** l'expediteur et lui renvoyer ses ACK). sigaddset bloque ces deux signaux
** pendant l'execution du handler pour eviter tout recouvrement.
*/

static void	setup_signal_handlers(void)
{
	struct sigaction	signal_action;

	sigemptyset(&signal_action.sa_mask);
	sigaddset(&signal_action.sa_mask, SIGUSR1);
	sigaddset(&signal_action.sa_mask, SIGUSR2);
	signal_action.sa_flags = SA_SIGINFO;
	signal_action.sa_sigaction = handle_server_ack;
	if (sigaction(SIGUSR1, &signal_action, NULL) < 0)
		print_error_and_exit("sigaction failed");
	if (sigaction(SIGUSR2, &signal_action, NULL) < 0)
		print_error_and_exit("sigaction failed");
}

/*
** main : ./client <server_pid> <message>
** 1) verifie qu'il y a exactement EXPECTED_ARGC arguments ;
** 2) parse le PID du serveur de facon stricte (ft_atoi_pid) ;
** 3) transmet le message puis l'octet de fin (send_message) ;
** 4) affiche la confirmation d'envoi.
** Toute erreur passe par print_error_and_exit (exit != 0, jamais de crash).
*/

int	main(int argc, char **argv)
{
	pid_t	server_pid;
	int		server_response;

	if (argc != EXPECTED_ARGC)
		print_error_and_exit("usage: ./client <server_pid> <message>");
	if (ft_atoi_pid(argv[PID_ARG_INDEX], &server_pid) == PARSING_FAILURE)
		print_error_and_exit("invalid PID: expected a positive integer");
	setup_signal_handlers();
	server_response = send_message(server_pid, argv[MESSAGE_ARG_INDEX]);
	if (server_response == SEND_FAILURE)
		print_error_and_exit("no response from server");
	ft_putstr_fd("Message sent to server.\n", STDOUT_FD);
	return (EXIT_SUCCESS);
}
