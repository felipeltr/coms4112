OUTPUT = build

CC = gcc

OBJS = main.o tree.o

$(OUTPUT): $(OBJS)
	$(CC) $(OBJS) -o $(OUTPUT)

%.o: %.c
	$(CC) -c -Wall $<

.PHONY: clean
clean:
	rm -f *.o $(OUTPUT)

.PHONY: all
all: clean $(OUTPUT)

main.o: main.c
tree.o: tree.c