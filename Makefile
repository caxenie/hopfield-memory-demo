# Computational Intelligence Lab @ NST TUM WSS2013-14
# compile all / independent homework assignments

# setup the compiler to use
CC=gcc -Wall -DNDEBUG -static -std=c99 -O3 -pipe 
# binaries
BINARIES=hopfield_phonebook

# targets with dependencies and rules to compile

all: $(BINARIES)
	
%: %.c
	$(CC) -g -o $@ $< -lm

clean:
	rm -rf $(BINARIES)

