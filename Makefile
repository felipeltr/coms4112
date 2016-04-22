OUTPUT = build

CC = gcc

OBJS = main.o tree.o random.o

$(OUTPUT): $(OBJS)
	$(CC) -msse4.2 $(OBJS) -o $(OUTPUT)

%.o: %.c
	$(CC) -msse4.2 -c -Wall $<

.PHONY: clean
clean:
	rm -f *.o $(OUTPUT)

.PHONY: all
all: clean $(OUTPUT)

main.o: main.c
tree.o: tree.c
random.o: random.c