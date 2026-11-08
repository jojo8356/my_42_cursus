# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: arena <arena@student.42.fr>                +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/11/04 08:00:00 by arena          #+#    #+#              #
#    Updated: 2026/11/04 08:00:00 by arena         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	= libftprintf.a

CC		= cc
CFLAGS	= -Wall -Wextra -Werror
AR		= ar rcs
RM		= rm -f

COMMON_SRC	= ft_output.c ft_num.c
MAND_SRC	= ft_printf.c ft_print_cs.c ft_print_num.c ft_print_ptr.c
BONUS_SRC	= ft_printf_bonus.c ft_parse_bonus.c ft_render_bonus.c \
			  ft_render_num_bonus.c ft_write_number_bonus.c

COMMON_OBJ	= $(COMMON_SRC:.c=.o)
MAND_OBJ	= $(MAND_SRC:.c=.o)
BONUS_OBJ	= $(BONUS_SRC:.c=.o)

MAND_STAMP	= .mandatory
BONUS_STAMP	= .bonus

all: $(NAME)

$(NAME): $(MAND_STAMP)

$(MAND_STAMP): $(COMMON_OBJ) $(MAND_OBJ)
	@$(RM) $(BONUS_STAMP) $(NAME)
	$(AR) $(NAME) $(COMMON_OBJ) $(MAND_OBJ)
	@touch $(MAND_STAMP)

bonus: $(BONUS_STAMP)

$(BONUS_STAMP): $(COMMON_OBJ) $(BONUS_OBJ)
	@$(RM) $(MAND_STAMP) $(NAME)
	$(AR) $(NAME) $(COMMON_OBJ) $(BONUS_OBJ)
	@touch $(BONUS_STAMP)

%.o: %.c ft_printf.h ft_printf_bonus.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(COMMON_OBJ) $(MAND_OBJ) $(BONUS_OBJ)

fclean: clean
	$(RM) $(NAME) $(MAND_STAMP) $(BONUS_STAMP)

re: fclean all

.PHONY: all bonus clean fclean re
