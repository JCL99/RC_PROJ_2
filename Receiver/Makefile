TARGETS = file-receiver file-sender

CC = gcc
CFLAGS = -Wall -Werror -O3

default : $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean :
	rm -f $(TARGETS)
