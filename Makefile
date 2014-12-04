#Makefile 

CC = gcc

banksim: banksim.c 
	$(CC) -pthread -o banksim banksim.c queue.c

clean:
	rm -f ./banksim *.o *~