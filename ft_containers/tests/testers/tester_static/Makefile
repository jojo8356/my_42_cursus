INCLUDE_PATH := ../../../includes

SRCS_DIR = srcs

FILES = $(wildcard ${SRCS_DIR}/*.cpp)

CONTAINERS = $(FILES:${SRCS_DIR}/%.cpp=%)

all: ${CONTAINERS}

${CONTAINERS}:
	g++ -c ${CXXFLAGS} -o /dev/null -I ${INCLUDE_PATH} -D NAMESPACE=ft ${SRCS_DIR}/$@.cpp

clean:
	@true

fclean: clean

re: fclean all

.PHONY: all clean fclean re
