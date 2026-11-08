/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: student <student@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/28 10:00:00 by student           #+#    #+#             */
/*   Updated: 2026/10/28 10:00:00 by student          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minitalk.h"

/*
** Definition de l'UNIQUE variable globale (justifiee dans minitalk.h).
** Initialisation a ACK_NONE : aucun accuse de reception en attente.
*/

volatile sig_atomic_t	g_protocol_state = ACK_NONE;

/*
** ft_strlen : nombre de caracteres d'une chaine terminee par '\0'.
** (Equivalent maison, seul moyen de mesurer une chaine sans libc.)
*/

size_t	ft_strlen(const char *string)
{
	size_t	character_count;

	character_count = 0;
	while (string[character_count])
		character_count++;
	return (character_count);
}

/*
** ft_putstr_fd : ecrit une chaine sur le descripteur donne en un seul
** appel a write() : aucune allocation, aucun buffer, utilisable partout
** (y compris depuis un gestionnaire de signaux).
*/

void	ft_putstr_fd(const char *string, int output_fd)
{
	size_t	character_count;

	character_count = ft_strlen(string);
	if (character_count > 0)
		write(output_fd, string, character_count);
}

/*
** ft_putnbr_fd : ecrit un entier signe en base DECIMAL_BASE sur output_fd.
** Les chiffres de poids fort sont ecrits d'abord (appel recursif), puis le
** chiffre courant est converti en son caractere ASCII (ASCII_ZERO + valeur).
*/

void	ft_putnbr_fd(int number, int output_fd)
{
	char	digit_character;

	if (number < 0)
	{
		write(output_fd, "-", 1);
		number = -number;
	}
	if (number >= DECIMAL_BASE)
		ft_putnbr_fd(number / DECIMAL_BASE, output_fd);
	digit_character = ASCII_ZERO + (number % DECIMAL_BASE);
	write(output_fd, &digit_character, 1);
}

/*
** ft_atoi_pid : convertit une chaine en PID de facon STRICTE.
** N'accepte que les chiffres de ASCII_ZERO a ASCII_NINE : ni signe, ni
** espace, ni autre caractere. Rejette les chaines vides, le PID 0
** (PID_MIN_VALUE), les valeurs negatives et tout depassement de capacite
** (OVERFLOW_GUARD verifie avant chaque multiplication par DECIMAL_BASE).
** parsed_pid n'est ecrit qu'en cas de succes.
** Retour : PARSING_SUCCESS ou PARSING_FAILURE.
*/

int	ft_atoi_pid(const char *pid_string, pid_t *parsed_pid)
{
	long	parsed_value;
	int		character_index;
	char	current_character;

	parsed_value = 0;
	character_index = 0;
	while (pid_string[character_index])
	{
		current_character = pid_string[character_index];
		if (current_character < ASCII_ZERO || current_character > ASCII_NINE)
			return (PARSING_FAILURE);
		if (parsed_value > OVERFLOW_GUARD)
			return (PARSING_FAILURE);
		parsed_value = parsed_value * DECIMAL_BASE
			+ (current_character - ASCII_ZERO);
		character_index++;
	}
	if (parsed_value < PID_MIN_VALUE || parsed_value > PID_MAX_VALUE)
		return (PARSING_FAILURE);
	*parsed_pid = (pid_t)parsed_value;
	return (PARSING_SUCCESS);
}

/*
** print_error_and_exit : affiche un message d'erreur sur la sortie standard
** (comportement ft_printf de l'ecole) puis quitte avec EXIT_FAILURE.
** Toute erreur du projet passe par ici : le programme ne quitte donc jamais
** de facon inattendue (pas de segmentation fault, pas de blocage).
*/

void	print_error_and_exit(const char *error_message)
{
	ft_putstr_fd("Error: ", STDOUT_FD);
	ft_putstr_fd(error_message, STDOUT_FD);
	ft_putstr_fd("\n", STDOUT_FD);
	exit(EXIT_FAILURE);
}
