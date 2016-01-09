all:Fatigue_Tester

# which compiler
CC=gcc

# which are include files
OBJ=Fatigue_Tester.o

# Option for development
CFLAGS= -g

# Option for release
#CFLAGS= -o 
 
Fatigue_Tester:Fatigue_Tester.o
	$(CC) $(OBJ) $(CFLAGS) -o Fatigue_Tester  -I/usr/include/mysql -L/usr/lib/mysql -lmysqlclient `pkg-config gtk+-3.0 --libs`  -l pthread  -export-dynamic
Fatigue_Tester.o:Fatigue_Tester.c socket_msg.h
	$(CC) $(CFLAGS) -c $< `pkg-config gtk+-3.0 --cflags`
clean:
	-rm *.o Fatigue_Tester
