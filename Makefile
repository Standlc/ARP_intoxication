NAME		=	ft_malcolm

SRCS		=	main.c								 \

OBJS		=	$(SRCS:.c=.o)

SRCS_SNIFF	=	sniff.c

OBJS_SNIFF	=	$(SRCS_SNIFF:.c=.o)

CC			=	cc

CFLAGS		=	-Wall -Wextra -Werror

RM			=	rm -f

all: $(NAME)

$(NAME): $(OBJS)
		$(CC) $(OBJS) -o $(NAME)

sniff: $(OBJS_SNIFF)
		$(CC) $(OBJS_SNIFF) -o sniff

%.o: %.c ft_malcolm.h Makefile
		$(CC) $(CFLAGS) -c $< -o $@

clean:
		@$(RM) $(OBJS) $(OBJS_SNIFF)

fclean:	clean
		@$(RM) $(NAME) sniff

re:		fclean all

.PHONY: all clean fclean re