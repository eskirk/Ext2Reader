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

#define DEBUG 0

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
 * and returns a uint32_t pointer pointing to the beginning of the 12
 * direct block pointers associated to the |dir| inode. Client is responsible
 * for freeing the dynamically allocated ext2_dir_entry.
 */
uint32_t *find_dir(FILE *image, char *dir);

/*
 * List entries inside some directory whose directory entries are pointed
 * to by |blocks|. |blocks| associates to a directory inode's 12 direct
 * pointers. Use find_dir() to get the direct block pointers associated to a
 * directory inode then pass them into list_entries() or dump_file()
 */
void list_entries(uint32_t *blocks);

/*
 * Dumps contents of file |file_dump| given that |file_dump| is a valid file
 * inside the directory associated to |blocks|. |blocks| is obtained using
 * find_dir() and specifies the 12 direct block pointers belonging to the
 * inode found using find_dir()
 */
void dump_file(uint32_t *blocks, char *file_dump);

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

uint32_t *find_dir(FILE *image, char *dir) {
   int i;
   bool found;

   // get superblock
   ext2_super_block *sb = malloc(BLOCK_SIZE);
   read_data(2, 0, sb, BLOCK_SIZE);

   // get bgdt, root dir entry list, and an array of the first
   // 12 blocks in inode 2
   ext2_group_desc *bgdt = malloc(BLOCK_SIZE);
   ext2_inode *ino = malloc(INODE_SIZE);
   ext2_dir_entry *dentry = malloc(BLOCK_SIZE);
   uint32_t *blocks = malloc(12 * sizeof(uint32_t));

   read_data(2 + TO_BGDT, 0, bgdt, BLOCK_SIZE);
   read_data(bgdt[0].bg_inode_table * 2, INODE_SIZE, ino, INODE_SIZE);

   // copy all the direct block pointers to a blocks array for root
   // inode (inode 2)
   for (i = 0; ino->i_block[i] && i < 12; i++)
      blocks[i] = ino->i_block[i];
   blocks[i] = 0;
   read_data(blocks[i = 0] * 2, 0, dentry, BLOCK_SIZE);
   ext2_dir_entry *dentry_next = dentry;

   if (strcmp(dir, "/")) { // if not equal to root
      char dir_cpy[DEFAULT_SIZE];
      strcpy(dir_cpy, dir);
      char *dir_search = strtok(dir_cpy, "/");

      // search for each tokenized directory until dir_search is found
      while (dentry_next->inode && dir_search) {
         found = false;
         char dentry_name[DEFAULT_SIZE];

         // obtain name of dentry as a c-string
         strncpy(dentry_name, dentry_next->name, dentry_next->name_len);
         dentry_name[dentry_next->name_len] = NULL;

#if DEBUG
         printf("dentry_name: %s\n", dentry_name);
         printf("dir_search: %s\n", dir_search);
#endif

         // locate entry name match
         if (!strcmp(dir_search, dentry_name)) {
            int block_group = (dentry_next->inode - 1) / sb->s_inodes_per_group;
            int local_inode_index = (dentry_next->inode - 1)
                  % sb->s_inodes_per_group;

#if DEBUG
            printf("\n\n");
            printf("block_group: %d\n", block_group);
            printf("inode: %d\n", dentry_next->inode);
            printf("local_inode_index: %d\n", local_inode_index);
#endif

            int sectors = local_inode_index * INODE_SIZE / 512;
            int offset = local_inode_index * INODE_SIZE % 512;
            read_data(bgdt[block_group].bg_inode_table * 2 + sectors, offset,
                  ino, INODE_SIZE);

#if DEBUG
            printf("block_group: %d\n", block_group);
            printf("inode: %d\n", dentry_next->inode);
            printf("inodes per group: %d\n", sb->s_inodes_per_group);
            printf("mode: 0x%04X\n", ino->i_mode);
            printf("\n\n");
#endif

            // if a matched entry name is a directory
            if (ino->i_mode >> ISDIR_SHIFT & 1) {
               for (i = 0; ino->i_block[i] && i < 12; i++)
                  blocks[i] = ino->i_block[i];
               blocks[i] = 0;
               i = 0;

               read_data(blocks[0] * 2, 0, dentry, BLOCK_SIZE);
               dentry_next = dentry;

               dir_search = strtok(NULL, "/");
               found = true;

               continue;
            }
         }

         // go to the next directory entry
         dentry_next = ((char *) dentry_next) + dentry_next->rec_len;
         if ((char *) dentry_next - (char *) dentry >= BLOCK_SIZE) {
            read_data(blocks[++i] * 2, 0, dentry, BLOCK_SIZE);
            dentry_next = dentry;
         }
      }

      if (!found) {
         fprintf(stderr, "\nError: %s is not a directory. Exiting...\n", dir);
         exit(1);
      }
   }

// teardown
   free(dentry);
   free(ino);
   free(bgdt);
   free(sb);
   return blocks;
}

void list_entries(uint32_t *blocks) {
   int i = 0, size;
   int sectors, offset, block_group, local_idx;
   char name[DEFAULT_SIZE];
   char type;

   // get superblock
   ext2_super_block *sb = malloc(BLOCK_SIZE);
   read_data(2, 0, sb, BLOCK_SIZE);

   // get bgdt and allocate space for an inode
   ext2_group_desc *bgdt = malloc(BLOCK_SIZE);
   read_data(4, 0, bgdt, BLOCK_SIZE);
   ext2_inode *ino = malloc(INODE_SIZE);

   // set dir and dir_next directory entries
   ext2_dir_entry *dir = malloc(BLOCK_SIZE);
   read_data(blocks[i] * 2, 0, dir, BLOCK_SIZE);
   ext2_dir_entry *dir_next = dir;

   printf("%20s %20s %20s\n\n", "filename", "type", "size");
   while (dir_next->inode) {
      // turn directory entry name into c-string
      strncpy(name, dir_next->name, dir_next->name_len);
      name[dir_next->name_len] = NULL;

      // get the associating inode
      block_group = (dir_next->inode - 1) / sb->s_inodes_per_group;
      local_idx = (dir_next->inode - 1) % sb->s_inodes_per_group;
      sectors = local_idx * INODE_SIZE / 512;
      offset = local_idx * INODE_SIZE % 512;

      read_data(bgdt[block_group].bg_inode_table * 2 + sectors, offset, ino,
      INODE_SIZE);

      if (ino->i_mode >> ISDIR_SHIFT & 1)
         type = 'd';
      else if (ino->i_mode >> ISFILE_SHIFT & 1)
         type = 'f';
      else
         type = 'u';
      size = ino->i_size;

      printf("%20s %20c %20d\n", name, type, size);

      dir_next = ((char *) dir_next) + dir_next->rec_len;
      if ((char *) dir_next - (char *) dir >= BLOCK_SIZE) {
         read_data(blocks[++i] * 2, 0, dir, BLOCK_SIZE);
         dir_next = dir;
      }
   }

   free(dir);
   free(ino);
   free(bgdt);
   free(sb);
}

void dump_file(uint32_t *blocks, char *file_dump) {
   int i = 0;
   ext2_super_block *sb = malloc(BLOCK_SIZE);
   ext2_group_desc *bgdt = malloc(BLOCK_SIZE);
   ext2_inode *ino = malloc(INODE_SIZE);
   ext2_dir_entry *dentry = malloc(BLOCK_SIZE);
   bool data_dumped = false;

   read_data(2, 0, sb, BLOCK_SIZE);
   read_data(2 + TO_BGDT, 0, bgdt, BLOCK_SIZE);
   read_data(blocks[i] * 2, 0, dentry, BLOCK_SIZE);

   ext2_dir_entry *dentry_next = dentry;

   while (dentry_next->inode && !data_dumped) {
      char dentry_name[DEFAULT_SIZE];

      strncpy(dentry_name, dentry_next->name, dentry_next->name_len);
      dentry_name[dentry_next->name_len] = NULL;

      // locate entry name and verify if a file
      if (!strcmp(file_dump, dentry_name)) {
         int block_group = (dentry_next->inode - 1) / sb->s_inodes_per_group;
         int local_inode_index = (dentry_next->inode - 1)
               % sb->s_inodes_per_group;

         int sectors = local_inode_index * INODE_SIZE / 512;
         int offset = local_inode_index * INODE_SIZE % 512;
         read_data(bgdt[block_group].bg_inode_table * 2 + sectors, offset,
                           ino, INODE_SIZE);

         // if a file, traverse through all in-use block pointers to dump
         // data
         if (ino->i_mode >> ISFILE_SHIFT & 1) {
            uint32_t *block = ino->i_block;
            parse_blocks(block);
            data_dumped = true;
            continue;
         }
      }

      dentry_next = ((char *) dentry_next) + dentry_next->rec_len;
      if ((char *) dentry_next - (char *) dentry >= BLOCK_SIZE) {
         read_data(blocks[++i] * 2, 0, dentry, BLOCK_SIZE);
         dentry_next = dentry;
      }
   }

   if (!data_dumped) {
      fprintf(stderr, "\nError: file %s could not be found. Exiting...\n",
            file_dump);
      exit(1);
   }

   free(sb);
   free(bgdt);
   free(ino);
   free(dentry);
}
