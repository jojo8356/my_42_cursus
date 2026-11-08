/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   transfer.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: student <student@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/28 10:00:00 by student           #+#    #+#             */
/*   Updated: 2026/10/28 10:00:00 by student          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minitalk.h"

/*
** wait_for_ack : attend l'acquittement du serveur apres UN bit.
** Chemin rapide : boucle de veille active (un ACK revient en quelques
** microsecondes). Chemin lent : usleep() repetes si l'ACK tarde (systeme
** charge). Abandon definitif apres ACK_POLL_LIMIT tentatives (~2 secondes)
** pour que le client ne reste jamais bloque si le serveur meurt.
** Retour : ACK_BIT_RECEIVED / ACK_MESSAGE_RECEIVED, ou ACK_NONE si le delai
** maximal est depasse.
*/

static int	wait_for_ack(void)
{
	int	spin_iterations;
	int	poll_attempts;
	int	received_ack;

	spin_iterations = 0;
	poll_attempts = 0;
	while (g_protocol_state == ACK_NONE)
	{
		if (spin_iterations < ACK_SPIN_LIMIT)
			spin_iterations++;
		else if (poll_attempts < ACK_POLL_LIMIT)
		{
			usleep(ACK_POLL_DELAY_US);
			poll_attempts++;
		}
		else
			return (ACK_NONE);
	}
	received_ack = (int)g_protocol_state;
	g_protocol_state = ACK_NONE;
	return (received_ack);
}

/*
** send_byte : envoie UN octet au serveur, bit par bit, du bit de poids fort
** au bit de poids faible. SIGUSR1 = bit a 1, SIGUSR2 = bit a 0.
** Apres chaque kill(), wait_for_ack() impose une pause jusqu'a la reponse du
** serveur : deux signaux de meme type ne sont donc jamais en vol en meme
** temps et aucun bit ne peut etre perdu.
** Retour : code d'ACK du dernier bit (ACK_MESSAGE_RECEIVED lorsque le
** serveur bonus recoit l'octet MESSAGE_END_MARKER final), ou SEND_FAILURE
** si le serveur est injoignable ou muet.
*/

int	send_byte(pid_t server_pid, unsigned char byte_to_send)
{
	int	bit_offset;
	int	signal_to_send;
	int	last_ack;

	bit_offset = BITS_PER_BYTE;
	last_ack = ACK_BIT_RECEIVED;
	while (bit_offset > 0)
	{
		bit_offset--;
		signal_to_send = SIGUSR2;
		if ((byte_to_send >> bit_offset) & 1)
			signal_to_send = SIGUSR1;
		g_protocol_state = ACK_NONE;
		if (kill(server_pid, signal_to_send) < 0)
			return (SEND_FAILURE);
		last_ack = wait_for_ack();
		if (last_ack == ACK_NONE)
			return (SEND_FAILURE);
	}
	return (last_ack);
}

/*
** send_message : transmet toute la chaine puis l'octet de fin de message
** MESSAGE_END_MARKER. Les octets sont envoyes bruts (tels quels) : l'UTF-8
** multioctets (Unicode) est donc transmis sans aucune alteration.
** Retour : code d'ACK du dernier bit, ou SEND_FAILURE en cas d'erreur.
*/

int	send_message(pid_t server_pid, const char *message)
{
	int		last_ack;
	int		character_index;

	character_index = 0;
	last_ack = ACK_BIT_RECEIVED;
	while (message[character_index])
	{
		last_ack = send_byte(server_pid,
				(unsigned char)message[character_index]);
		if (last_ack == SEND_FAILURE)
			return (SEND_FAILURE);
		character_index++;
	}
	return (send_byte(server_pid, MESSAGE_END_MARKER));
}
