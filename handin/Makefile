CC=gcc
FLAGS=-g -w
FILES=ext2.c program4.c 
OBJ=ext2.o program4.o 
OUT=program4

program_4: build

build: $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(OUT)

ext2.o: ext2.c ext2.h
	$(CC) $(FLAGS) -c  ext2.c

program4.o: program4.c ext2.h
	$(CC) $(FLAGS) -c  program4.c

clean:
	rm -rf $(OBJ) $(OUT)

rebuild: clean build
