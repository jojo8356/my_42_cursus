# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/11/04 08:00:00 by jojo8356          #+#    #+#              #
#    Updated: 2026/11/08 18:30:00 by jojo8356         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		= libftprintf.a

CC			= cc
CFLAGS		= -Wall -Wextra -Werror
CPPFLAGS	= -I$(INC_DIR)
AR			= ar rcs
RM			= rm -f

SRC_DIR		= src
INC_DIR		= include
OBJ_DIR		= obj

HEADERS		= $(INC_DIR)/ft_printf.h $(INC_DIR)/ft_printf_bonus.h

COMMON_SRCS	= ft_output.c ft_num.c
MAND_SRCS	= ft_printf.c ft_print_cs.c ft_print_num.c ft_print_ptr.c
BONUS_SRCS	= ft_printf_bonus.c ft_parse_bonus.c ft_render_bonus.c \
			  ft_render_num_bonus.c ft_write_number_bonus.c

COMMON_OBJS	= $(addprefix $(OBJ_DIR)/, $(COMMON_SRCS:.c=.o))
MAND_OBJS	= $(addprefix $(OBJ_DIR)/, $(MAND_SRCS:.c=.o))
BONUS_OBJS	= $(addprefix $(OBJ_DIR)/, $(BONUS_SRCS:.c=.o))

MAND_STAMP	= $(OBJ_DIR)/.mandatory
BONUS_STAMP	= $(OBJ_DIR)/.bonus

all: $(NAME)

$(NAME): $(MAND_STAMP)

$(MAND_STAMP): $(COMMON_OBJS) $(MAND_OBJS)
	@mkdir -p $(OBJ_DIR)
	@$(RM) $(BONUS_STAMP) $(NAME)
	$(AR) $(NAME) $(COMMON_OBJS) $(MAND_OBJS)
	@touch $(MAND_STAMP)

bonus: $(BONUS_STAMP)

$(BONUS_STAMP): $(COMMON_OBJS) $(BONUS_OBJS)
	@mkdir -p $(OBJ_DIR)
	@$(RM) $(MAND_STAMP) $(NAME)
	$(AR) $(NAME) $(COMMON_OBJS) $(BONUS_OBJS)
	@touch $(BONUS_STAMP)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	$(RM) -r $(OBJ_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all bonus clean fclean re
