SRCS:=$(wildcard ./src/*.c)
OBJS := ${patsubst ./src/%.c, ./bin/%.o, $(SRCS)}
ELFS:= sendServer
CFLAGS:= -Wall
$(ELFS):$(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ -lpthread -lcrypto -lmysqlclient
./bin/%.o:./src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean rebuild
clean:
	rm -rf $(ELFS) $(OBJS)
rebuild:clean $(ELFS)