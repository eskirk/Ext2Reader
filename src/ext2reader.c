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
   fprintf(stderr, "\nUsage: \n"
         "     ext2reader <image.ext2> [path]\n"
         "     ext2reader -l <image.ext2> <file_to_dump.txt>\n"
         "\n     If [path] is not specified, '/' will be used"
         "\nOptions:\n"
         " -l    print to the screen the contents of <file_to_dump.txt>\n");
   exit(1);
}

void list_entries(char *image, char *dir) {
   int i, j;

   printf("image: %s\n", image);
   printf("dir: %s\n", dir);

   fp = fopen(image, "r");
   if (!fp) {
      fprintf(stderr, "Could not find file %s\n", image);
      exit(1);
   }

   // superblock
   ext2_super_block *sb = malloc(BLOCK_SIZE);
   read_data(2, 0, sb, BLOCK_SIZE);
   printf("blocks per group: %d\n", sb->s_blocks_per_group);
   printf("block size: %d\n", 1024 << sb->s_log_block_size);
   printf("inodes per group: %d\n", sb->s_inodes_per_group);
   printf("total number of inodes: %d\n", sb->s_inodes_count);
   printf("total number of inodes in use: %d\n",
         sb->s_inodes_count - sb->s_free_inodes_count);
   printf("number of blocks in a group: %d\n", sb->s_blocks_per_group);
   printf("\n\n");

   ext2_group_desc *bgdt = malloc(BLOCK_SIZE);
   uint8_t *inode_bitmap = malloc(BLOCK_SIZE);
   ext2_inode *inode_table = malloc(sb->s_inodes_per_group * INODE_SIZE);

   // iterate through inodes in use by iterating through block groups
   int block_group = -1, local_inode_index = -1, shift;

   for (i = 1, shift = 0; i <= sb->s_inodes_count; i++, shift++) {
      int old_block_group = block_group;
      block_group = (i - 1) / sb->s_inodes_per_group;
      local_inode_index = (i - 1) % sb->s_inodes_per_group;

      if (block_group != old_block_group) {
         int curr_bg_idx = 2 * (1 + sb->s_blocks_per_group * block_group);

         read_data(curr_bg_idx + 2, 0, bgdt, BLOCK_SIZE);
         read_data(curr_bg_idx + 6, 0, inode_bitmap, BLOCK_SIZE);
         read_data(curr_bg_idx + 8, 0, inode_table,
               sb->s_inodes_per_group * INODE_SIZE);
         shift = 0;

         printf("\n\nblock group %d\n", block_group);
      }

      shift %= 8;
      if (inode_bitmap[local_inode_index / 8] >> shift & 1) { // if inode in use
         printf("index: %d, mode: 0x%04X", local_inode_index,
               inode_table[local_inode_index].i_mode);

         if (inode_table[local_inode_index].i_mode >> 14 & 1)
            printf(" is a directory\n");
         else
            printf("\n");
      }
   }

   // teardown
   free(inode_table);
   free(inode_bitmap);
   free(bgdt);
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
