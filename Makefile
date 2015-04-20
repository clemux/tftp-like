CC = gcc
LFLAGS = -g -W -Wall -Wmissing-declarations -Wmissing-prototypes -Wredundant-decls -Wshadow -Wbad-function-cast -Wcast-qual -g -o
CFLAGS = -c -g -W -Wall -Wmissing-declarations -Wmissing-prototypes -Wredundant-decls -Wshadow -Wbad-function-cast -Wcast-qual -g
EXEC_RECEIVER = receiver-udp
EXEC_SENDER = sender-udp
SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

all : $(EXEC_RECEIVER) $(EXEC_SENDER) 

$(EXEC_RECEIVER) : server.o lib.o
	@ $(CC) $(LFLAGS) $@ $^ $(LDFLAGS)

$(EXEC_SENDER) : client.o lib.o
	@ $(CC) $(LFLAGS) $@ $^ $(LDFLAGS)

%.o : %.c
	@ $(CC) $(CFLAGS) $< -o $*.o

clean:
	@ rm -f $(OBJ)
mrproper: clean
	@ rm -f $(EXEC)
