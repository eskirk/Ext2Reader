/*
 * ext2.c
 *
 *  Created on: May 13, 2014
 *      Author: knavero
 */

#include "ext2.h"

FILE *fp = NULL;

//the block argument is in terms of SD card 512 byte sectors
void read_data(uint32_t block, uint16_t offset, uint8_t* data, uint16_t size) {
   if (offset > 511) {
      printf("Offset greater than 511.\n");
      exit(0);
   }

   fseek(fp, block * 512 + offset, SEEK_SET);
   fread(data, size, 1, fp);
}
