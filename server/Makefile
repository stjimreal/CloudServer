SRCS:=$(wildcard ./src/*.c)
OBJS := ${patsubst ./src/%.c, ./bin/%.o, $(SRCS)}
ELFS:= sendServer
CFLAGS:= -Wall
$(ELFS):$(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ -lpthread -lcrypto -lmysqlclient -lcrypt
./bin/%.o:./src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(ELFS) $(OBJS)
.PHONY: rebuild
rebuild:clean $(ELFS)