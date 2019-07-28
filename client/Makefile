SRCS:=$(wildcard ./src/*.c)
OBJS := ${patsubst ./src/%.c, ./bin/%.o, $(SRCS)}
ELFS:= client
CFLAGS:= -Wall
CC:= gcc
$(ELFS):$(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ -lpthread -lcrypto -lcrypt
./bin/%.o:./src/%.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -rf $(ELFS) $(OBJS)
rebuild:clean $(ELFS)