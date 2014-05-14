/*
 ============================================================================
 Name        : Ext2Reader.c
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

void print_error_msg_and_exit(int exit_value);

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

         // TODO operations to print to the screen the contents of
         // |file_dump| here

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

      // TODO operation list all entries in |dir| here
      fp = fopen(image, "r");
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
