INC = ./include
SRC = ./src


# INC_LOG4CPLUS = /home/fnl/Software/log4cplus/include
# LIB_LOG4CPLUS = /home/fnl/Software/log4cplus/lib
# INC_PYTHON = /usr/include/python2.7

TARGET = satSocket
BIN_TARGET = ${BIN}/${TARGET}

CC = g++
CFLAGS = -g -Wall -std=c++11 -fno-stack-protector -I${INC} 
SO = -lpthread -lssl -lcrypto
#####################version 3#############################
#################use liblog4cplus.so#######################

SOURCE = $(wildcard ${SRC}/*.cpp)

${TARGET}:${SOURCE} 
	$(CC) $(CFLAGS) -o $@ $^ ${SO}

.PHONY:clean
clean:
	rm -rf ${TARGET} 
