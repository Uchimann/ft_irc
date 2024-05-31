NAME = ircserv
BOTNAME = bot
CC = c++

FLAG = -std=c++98 -Wall -Wextra -Werror

SRC = srcs/main.cpp srcs/Server.cpp srcs/Channel.cpp \
      srcs/Utils.cpp srcs/UtilsServer.cpp srcs/commands/BOT.cpp srcs/commands/JOIN.cpp \
      srcs/commands/CAP.cpp srcs/commands/HELP.cpp srcs/commands/INFO.cpp srcs/commands/INVITE.cpp \
      srcs/commands/KICK.cpp srcs/commands/LIST.cpp srcs/commands/MODE.cpp srcs/commands/NOTICE.cpp \
      srcs/commands/NICK.cpp srcs/commands/OPER.cpp srcs/commands/PART.cpp \
      srcs/commands/PASS.cpp srcs/commands/PRIVMSG.cpp srcs/commands/QUIT.cpp srcs/commands/USER.cpp \
      srcs/commands/TOPIC.cpp srcs/commands/WHO.cpp srcs/commands/WHOIS.cpp

BOTSRC = srcs/bot/Bot.cpp srcs/bot/main.cpp srcs/Utils.cpp

OBJDIR = obj
BOTOBJDIR = bot_obj

OBJS = $(SRC:%.cpp=$(OBJDIR)/%.o)
BOTOBJS = $(BOTSRC:%.cpp=$(BOTOBJDIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(FLAG) $(OBJS) -o $(NAME)

$(BOTNAME): $(BOTOBJS)
	$(CC) $(FLAG) $(BOTOBJS) -o $(BOTNAME)

$(OBJDIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CC) $(FLAG) -c $< -o $@

$(BOTOBJDIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CC) $(FLAG) -c $< -o $@

bonus: $(BOTNAME)

clean:
	rm -rf $(OBJDIR) $(BOTOBJDIR)

fclean: clean
	rm -f $(NAME) $(BOTNAME)

re: fclean all

.PHONY: all clean fclean re bonus
