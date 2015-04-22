CC = gcc
LFLAGS = -g -Wextra -Wall -Wmissing-declarations -Wmissing-prototypes -Wredundant-decls -Wshadow -Wbad-function-cast -Wcast-qual -o
CFLAGS = -c -g -Wextra -Wall -Wmissing-declarations -Wmissing-prototypes -Wredundant-decls -Wshadow -Wbad-function-cast -Wcast-qual
EXEC_RECEIVER = receiver-udp
EXEC_SENDER = sender-udp
SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

all : $(EXEC_RECEIVER) $(EXEC_SENDER) 

$(EXEC_RECEIVER) : server.o lib.o md5.o
	@ $(CC) $(LFLAGS) $@ $^ $(LDFLAGS)

$(EXEC_SENDER) : client.o lib.o md5.o
	@ $(CC) $(LFLAGS) $@ $^ $(LDFLAGS)

%.o : %.c
	@ $(CC) $(CFLAGS) $< -o $*.o

clean:
	@ rm -f $(OBJ)
mrproper: clean
	@ rm -f $(EXEC)
