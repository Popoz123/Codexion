NAME		= codexion

CC			= cc
CFLAGS		= -Wall -Wextra -Werror -pthread

OBJDIR		= coders/obj

SRCS		= codexion.c \
			  monitor.c \
			  struct_init.c \
			  utils.c \
			  heap_queue.c \
			  heap_utils.c

OBJS		= $(addprefix $(OBJDIR)/, $(SRCS:.c=.o))

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJDIR)/%.o: /%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
