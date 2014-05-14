/*
 * ext2.c
 *
 *  Created on: May 13, 2014
 *      Author: knavero
 */

#include "ext2.h"

FILE *fp = NULL;

//the block argument is in terms of SD card 512 byte sectors
/*
 * Sets cursor to |sector|*512 + |offset|, reads |size| bytes from
 * that position in the FILE *|fp| global, and stores the data in a block
 * pointed to by |data|
 */
void read_data(uint32_t sector, uint16_t offset, uint8_t* data, uint16_t size) {
   if (offset > 511) {
      printf("Offset greater than 511.\n");
      exit(0);
   }

   fseek(fp, sector * 512 + offset, SEEK_SET);
   fread(data, size, 1, fp);
}
