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
#include "ext2reader.h"

#define DEBUG 1

#define DEFAULT_SIZE 64
#define ARG_COUNT_L 4
#define ARG_COUNT_MIN 2
#define ARG_COUNT_MAX 3

int main(int argc, char **argv) {
   int c, i;
   bool list_entries_flag = true;
   char file_dump[DEFAULT_SIZE];
   char buffer[DEFAULT_SIZE];
   char image[DEFAULT_SIZE];
   char dir[DEFAULT_SIZE];
   uint32_t *dir_blocks;

   strcpy(dir, "/");

   if ((c = getopt(argc, argv, "l:")) != -1) {
      switch (c) {
      case 'l':
         if (argc != ARG_COUNT_L)
            print_error_msg_and_exit(1);

         strcpy(image, optarg);
         strcpy(file_dump, argv[argc - 1]);
         strcpy(dir, file_dump);

         // manipulate dir to the last directory right before the filename
         for (i = strlen(dir) - 1; dir[i] != '/' && i > 0; i--)
            ;
         dir[i] = NULL;

         if (!i)
            strcpy(dir, "/");
         else if (dir[0] != '/') {
            strcpy(buffer, "/");
            strcat(buffer, dir);
            strcpy(dir, buffer);
         }

         // manipulate file_dump to just the filename
         strcpy(buffer, file_dump);
         char *pch = strtok(buffer, "/");
         while (pch) {
            strcpy(file_dump, pch);
            pch = strtok(NULL, "/");
         }

         list_entries_flag = false;
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
   }

   fp = fopen(image, "r");
   if (!fp) {
      fprintf(stderr, "\nError: Could not find file %s\n", image);
      exit(1);
   }

   dir_blocks = find_dir(fp, dir);
   if (list_entries_flag)
      list_entries(dir_blocks);
   else
      dump_file(dir_blocks, file_dump);

   free(dir_blocks);
   fclose(fp);

   return 0;
}
