OUTPUT = build

CC = gcc

OBJS = main.o tree.o random.o

DEF = 

$(OUTPUT): $(OBJS)
	$(CC) -msse4.2 -O3 -flto $(OBJS) -o $(OUTPUT)

.PHONY: clic
clic: DEF = -D _CLIC_ENV
clic: all

%.o: %.c
	$(CC) $(DEF) -O3 -flto -msse4.2 -c -Wall $<

.PHONY: clean
clean:
	rm -f *.o $(OUTPUT)

.PHONY: all
all: clean $(OUTPUT)

main.o: main.c
tree.o: tree.c
random.o: random.c