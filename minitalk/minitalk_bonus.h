/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minitalk_bonus.h                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: student <student@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/28 10:00:00 by student           #+#    #+#             */
/*   Updated: 2026/10/28 10:00:00 by student          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINITALK_BONUS_H
# define MINITALK_BONUS_H

# include "minitalk.h"

/*
** Messages affiches par client_bonus selon la reponse du serveur :
**   MSG_CONFIRM : le serveur (bonus) a renvoye l'ACK de message (SIGUSR2),
**                 la reception du message entier est donc confirmee.
**   MSG_SENT    : les bits ont ete acquittes (ACK de flux) mais le serveur
**                 n'a pas envoye d'ACK de message (serveur obligatoire).
*/
# define MSG_CONFIRM "Message received by the server!"
# define MSG_SENT "Message sent to server."

#endif
