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
#define TO_BGDT 2
#define ISDIR_SHIFT 14
#define ISFILE_SHIFT 15

typedef enum bool {
   false, true
} bool;

/*
 * Prints a message displaying Usage instructions and exits returning a
 * value of 1
 */
void print_error_msg_and_exit(int exit_value);

/*
 * Finds the directory specified by |dir| inside ext2 filesystem |image|
 * and returns the ext2_dir_entry at the beginning of the list. Client is
 * responsible for freeing the dynamically allocated ext2_dir_entry.
 */
ext2_dir_entry *find_dir(FILE *image, char *dir);

/*
 * List entries inside |dir|
 */
void list_entries(ext2_dir_entry *dir);

/*
 * Dumps contents of |file_dump| given that |file_dump| is a valid file
 * inside directory |dir|
 */
void dump_file(ext2_dir_entry *dir, char *file_dump);

/*
 * This is called by parse_blocks() for recursion, and is not to be
 * directly called by the client application. |blocks| points to the beginning
 * of an inode's i_block field. The inode must of type regular file (0x8000)
 * inside i_mode. |opt| can be either "d" for direct, "i" for indirect, or
 * "I" for doubly indirect.
 */
static void parse_blocks_recurs(uint32_t *blocks, char *opt) {
   int i, j, k;
   char data[BLOCK_SIZE];

   if (!strcmp(opt, "d")) {
      for (i = 0; blocks[i] && i < 12; i++) {
         read_data(blocks[i] * 2, 0, data, BLOCK_SIZE);
         for (j = 0; data[j] && j < BLOCK_SIZE; j++)
            printf("%c", data[j]);
      }

      if (blocks[i - 1])
         parse_blocks_recurs(blocks, "i");
   }
   else if (!strcmp(opt, "i")) {
      uint32_t direct_blocks[BLOCK_SIZE];

      read_data(blocks[12] * 2, 0, direct_blocks, BLOCK_SIZE);
      for (i = 0; direct_blocks[i] && i < 256; i++) {
         read_data(direct_blocks[i] * 2, 0, data, BLOCK_SIZE);
         for (j = 0; data[j] && j < BLOCK_SIZE; j++)
            printf("%c", data[j]);
      }

      if (direct_blocks[i - 1])
         parse_blocks_recurs(blocks, "I");
   }
   else if (!strcmp(opt, "I")) {
      uint32_t indirect_blocks[BLOCK_SIZE];
      uint32_t direct_blocks[BLOCK_SIZE];

      read_data(blocks[13] * 2, 0, indirect_blocks, BLOCK_SIZE);
      for (i = 0; i < 256; i++) {
         read_data(indirect_blocks[i] * 2, 0, direct_blocks, BLOCK_SIZE);
         for (j = 0; j < 256; j++) {
            read_data(direct_blocks[j] * 2, 0, data, BLOCK_SIZE);
            for (k = 0; data[k] && k < BLOCK_SIZE; k++)
               printf("%c", data[k]);
         }
      }
   }
   else
      fprintf(stderr, "\nError: invalid opt\n");
}

/*
 * Parse block pointers and dump the data to screen. The intended use of this
 * function is such that |blocks| points to the beginning of an inode's
 * i_block field. The inode must be of type regular file (0x8000) inside mode.
 * This function is called within dump_file() and shouldn't be called directly
 * inside the client application
 */
static void parse_blocks(uint32_t *blocks) {
   parse_blocks_recurs(blocks, "d");
}

int main(int argc, char **argv) {
   int c, i;
   bool list_entries_flag = true;
   char file_dump[DEFAULT_SIZE];
   char buffer[DEFAULT_SIZE];
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
   fprintf(stderr,
         "\nUsage: \n"
               "     ext2reader <image.ext2> [path]\n"
               "     ext2reader -l <image.ext2> <file_to_dump.txt>\n"
               "\n     If [path] is not specified, '/' will be used\n"
               "\nOptions:\n"
               "     -l    print to the screen the contents of <file_to_dump.txt>\n"
               "\nNotes:\n"
               "     All paths not prefixed with '/' are relative to the root directory\n");
   exit(1);
}

ext2_dir_entry *find_dir(FILE *image, char *dir) {
   // get superblock
   ext2_super_block *sb = malloc(BLOCK_SIZE);
   read_data(2, 0, sb, BLOCK_SIZE);

   // get bgdt, inode table, and root dir entry list
   ext2_group_desc *bgdt = malloc(BLOCK_SIZE);
   ext2_inode *inode_table = malloc(sb->s_inodes_per_group * INODE_SIZE);
   ext2_dir_entry *dentry = malloc(BLOCK_SIZE);

   read_data(2 + TO_BGDT, 0, bgdt, BLOCK_SIZE);
   read_data(bgdt[0].bg_inode_table * 2, 0, inode_table,
         sb->s_inodes_per_group * INODE_SIZE);
   read_data(inode_table[1].i_block[0] * 2, 0, dentry, BLOCK_SIZE);

   if (strcmp(dir, "/")) {
      char dir_cpy[DEFAULT_SIZE];
      strcpy(dir_cpy, dir);
      char *dir_search = strtok(dir_cpy, "/");
      bool found;

      while (dir_search) {
         ext2_dir_entry *dentry_next = dentry;
         found = false;
         while (dentry_next->inode && !found) {
            char dentry_name[DEFAULT_SIZE];

            strncpy(dentry_name, dentry_next->name, dentry_next->name_len);
            dentry_name[dentry_next->name_len] = NULL;

            // locate entry name and verify if a directory
            if (!strcmp(dir_search, dentry_name)) {
               int block_group = (dentry_next->inode - 1)
                     / sb->s_inodes_per_group;
               int local_inode_index = (dentry_next->inode - 1)
                     % sb->s_inodes_per_group;

               read_data(bgdt[block_group].bg_inode_table * 2, 0, inode_table,
                     sb->s_inodes_per_group * INODE_SIZE);

               if (inode_table[local_inode_index].i_mode >> ISDIR_SHIFT & 1) {
                  read_data(inode_table[local_inode_index].i_block[0] * 2, 0,
                        dentry, BLOCK_SIZE);
                  found = true;
               }
            }

            dentry_next = ((char *) dentry_next) + dentry_next->rec_len;
         }

         if (!found) {
            fprintf(stderr, "\nError: %s is not a directory. Exiting...\n",
                  dir);
            exit(1);
         }

         dir_search = strtok(NULL, "/");
      }
   }

   // teardown
   free(inode_table);
   free(bgdt);
   free(sb);
   return dentry;
}

void list_entries(ext2_dir_entry *dir) {
   while (dir->inode) {
      int c_idx;

      for (c_idx = 0; c_idx < dir->name_len; c_idx++)
         printf("%c", dir->name[c_idx]);
      printf("\n");

      dir = ((char *) dir) + dir->rec_len;
   }
}

void dump_file(ext2_dir_entry *dir, char *file_dump) {
   int i, j;
   ext2_super_block *sb = malloc(BLOCK_SIZE);
   ext2_group_desc *bgdt = malloc(BLOCK_SIZE);
   ext2_inode *inode_table = malloc(sb->s_inodes_per_group * INODE_SIZE);
   char *data = malloc(BLOCK_SIZE);
   bool data_dumped = false;

   read_data(2, 0, sb, BLOCK_SIZE);
   read_data(2 + TO_BGDT, 0, bgdt, BLOCK_SIZE);

   while (dir->inode && !data_dumped) {
      char dentry_name[DEFAULT_SIZE];

      strncpy(dentry_name, dir->name, dir->name_len);
      dentry_name[dir->name_len] = NULL;

      // locate entry name and verify if a file
      if (!strcmp(file_dump, dentry_name)) {
         int block_group = (dir->inode - 1) / sb->s_inodes_per_group;
         int local_inode_index = (dir->inode - 1) % sb->s_inodes_per_group;

         read_data(bgdt[block_group].bg_inode_table * 2, 0, inode_table,
               sb->s_inodes_per_group * INODE_SIZE);

         // if a file, traverse through all in-use block pointers to dump
         // data
         if (inode_table[local_inode_index].i_mode >> ISFILE_SHIFT & 1) {
            uint32_t *block = inode_table[local_inode_index].i_block;
            parse_blocks(block);
            data_dumped = true;
         }
      }

      dir = ((char *) dir) + dir->rec_len;
   }

   if (!data_dumped) {
      fprintf(stderr, "\nError: file %s could not be found. Exiting...\n",
            file_dump);
      exit(1);
   }

   free(sb);
   free(bgdt);
   free(inode_table);
   free(data);
}
