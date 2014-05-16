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
#define INODE_SIZE sizeof(ext2_inode)
#define IS_DIR 14
#define TO_BGDT 2
#define TO_INODE_BITMAP 6
#define TO_INODE_TABLE 8

typedef enum bool {
   false, true
} bool;

void print_error_msg_and_exit(int exit_value);
ext2_dir_entry *find_dir(FILE *image, char *dir);
void list_entries(ext2_dir_entry *dir);
void dump_file(ext2_dir_entry *dir, char *file_dump);

int main(int argc, char **argv) {
   int c;
   bool list_entries_flag = true;
   char file_dump[DEFAULT_SIZE];
   char image[DEFAULT_SIZE];
   char dir[DEFAULT_SIZE];
   ext2_dir_entry *ext2_dir;

   strcpy(dir, "/");

   if ((c = getopt(argc, argv, "l:")) != -1) {
      switch (c) {
      case 'l':
         if (argc != ARG_COUNT_L)
            print_error_msg_and_exit(1);

         strcpy(image, optarg);
         strcpy(file_dump, argv[argc - 1]);
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
      fprintf(stderr, "Could not find file %s\n", image);
      exit(1);
   }

   ext2_dir = find_dir(fp, dir);
   if (list_entries_flag)
      list_entries(ext2_dir);
   else
      dump_file(ext2_dir, file_dump);

   free(ext2_dir);
   fclose(fp);

   return 0;
}

void print_error_msg_and_exit(int exit_value) {
   fprintf(stderr, "\nUsage: \n"
         "     ext2reader <image.ext2> [path]\n"
         "     ext2reader -l <image.ext2> <file_to_dump.txt>\n"
         "\n     If [path] is not specified, '/' will be used"
         "\nOptions:\n"
         " -l    print to the screen the contents of <file_to_dump.txt>\n");
   exit(1);
}

ext2_dir_entry *find_dir(FILE *image, char *dir) {
   int i, j;

   // get superblock
   ext2_super_block *sb = malloc(BLOCK_SIZE);
   read_data(2, 0, sb, BLOCK_SIZE);

   // get bgdt, inode table, and inode for root dir
   ext2_group_desc *bgdt = malloc(BLOCK_SIZE);
   ext2_inode *inode_table = malloc(sb->s_inodes_per_group * INODE_SIZE);
   ext2_dir_entry *dentry = malloc(BLOCK_SIZE);

   read_data(2 + TO_BGDT, 0, bgdt, BLOCK_SIZE);
   read_data(2 + TO_INODE_TABLE, 0, inode_table,
         sb->s_inodes_per_group * INODE_SIZE);
   read_data(inode_table[1].i_block[0] * 2, 0, dentry, BLOCK_SIZE);

   // free anything unnecessary at this point
   free(inode_table);
   free(bgdt);
   free(sb);

   if (!strcmp(dir, "/"))
      return dentry;
   else {
      // TODO look for dir
      printf("look for dir\n");
      return NULL;
   }
}

void list_entries(ext2_dir_entry *ext2_dir) {
   while (ext2_dir->inode) {
      int c_idx;

      for (c_idx = 0; c_idx < ext2_dir->name_len; c_idx++)
         printf("%c", ext2_dir->name[c_idx]);
      printf("\n");

      ext2_dir = ((char *) ext2_dir) + ext2_dir->rec_len;
   }
}

// TODO finish this function
void dump_file(ext2_dir_entry *dir, char *file_dump) {
   printf("Inside dump_file()\n");
   printf("file_dump: %s\n", file_dump);
}
