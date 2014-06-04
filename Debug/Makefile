CC=gcc
FLAGS=-g -w
FILES=../src/ext2.c ../src/ext2reader.c ../src/main.c
OBJ=ext2.o ext2reader.o main.o
OUT=ext2reader

all: build

build: $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(OUT)

ext2.o: ../src/ext2.c ../src/ext2.h
	$(CC) $(FLAGS) -c  ../src/ext2.c

ext2reader.o: ../src/ext2reader.c ../src/ext2.h
	$(CC) $(FLAGS) -c  ../src/ext2reader.c

main.o: ../src/main.c
	$(CC) $(FLAGS) -c ../src/main.c

clean:
	rm -rf $(OBJ) $(OUT)

rebuild: clean build
