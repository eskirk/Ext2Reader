/*
 ============================================================================
 Name        : ext2Reader.c
 Author      : Kevin Navero
 Version     :
 Copyright   : 
 Description : ext2 reader, Program 4
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "ext2.h"

#define DEFAULT_SIZE 64
#define ARG_COUNT_L 4
#define ARG_COUNT_MIN 2
#define ARG_COUNT_MAX 3
#define BLOCK_SIZE 1024

void print_error_msg_and_exit(int exit_value);
void list_entries(char *image, char *dir);
void print_file_to_screen(char *image, char *dir, char *file_dump);

int main(int argc, char **argv) {
   int c;
   char file_dump[DEFAULT_SIZE];
   char image[DEFAULT_SIZE];
   char dir[DEFAULT_SIZE];

   strcpy(dir, "/");

   if ((c = getopt(argc, argv, "l:")) != -1) {
      switch (c) {
      case 'l':
         if (argc != ARG_COUNT_L)
            print_error_msg_and_exit(1);

         strcpy(image, optarg);
         strcpy(file_dump, argv[argc - 1]);

         print_file_to_screen(image, dir, file_dump);

         break;
      default:
         print_error_msg_and_exit(1);
      }
   }
   else {
      if (argc < ARG_COUNT_MIN || argc > ARG_COUNT_MAX)
         print_error_msg_and_exit(1);

      strcpy(image, *++argv);

      if (*++argv)
         strcpy(dir, *argv);

      list_entries(image, dir);
   }

   return 0;
}

void print_error_msg_and_exit(int exit_value) {
   printf("\nUsage: \n"
         "     ext2reader <image.ext2> [path]\n"
         "     ext2reader -l <image.ext2> <file_to_dump.txt>\n"
         "\n     If [path] is not specified, '/' will be used"
         "\nOptions:\n"
         " -l    print to the screen the contents of <file_to_dump.txt>\n");
   exit(1);
}

void list_entries(char *image, char *dir) {
   printf("image: %s\n", image);
   printf("dir: %s\n", dir);

   fp = fopen(image, "r");
   if (!fp) {
      printf("Could not find file %s\n", image);
      exit(1);
   }

   ext2_super_block *sb = malloc(BLOCK_SIZE);
   read_data(2, 0, sb, BLOCK_SIZE);

   printf("inodes count: %d\n", sb->s_inodes_count);
   printf("blocks count: %d\n", sb->s_blocks_count);
   printf("reserved blocks count: %d\n", sb->s_r_blocks_count);
   printf("free inodes count: %d\n", sb->s_free_inodes_count);
   printf("free blocks count: %d\n", sb->s_free_blocks_count);

   // teardown
   free(sb);
   fclose(fp);
}

// TODO finish this function
void print_file_to_screen(char *image, char *dir, char *file_dump) {
   printf("Inside print_file_to_screen()\n");
   printf("image: %s\n", image);
   printf("dir: %s\n", dir);
   printf("file_dump: %s\n", file_dump);
}
