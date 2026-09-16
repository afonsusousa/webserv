CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98 -I. -I./LocalResource
PROG := webserv

SRCS := main.cpp Server.cpp Connection.cpp HttpRequest.cpp LocalResource/FileResource.cpp LocalResource/CgiResource.cpp Parser/ConfigParser.cpp
OBJS := ${SRCS:.cpp=.o}

all: ${PROG}

${PROG}: ${OBJS}
	${CXX} ${CXXFLAGS} ${OBJS} -o ${PROG}

%.o: %.cpp
	${CXX} ${CXXFLAGS} -c $< -o $@

clean:
	rm -f ${OBJS}

fclean: clean
	rm -f ${PROG}

re: fclean all

debug: CXXFLAGS += -g -O0
debug: PROG := fixed
debug: re

run: all
	./${PROG}

.PHONY: all clean fclean re debug run
