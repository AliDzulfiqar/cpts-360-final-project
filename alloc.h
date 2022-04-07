#ifndef ALLOC_H
#define ALLOC_H


#include "util.h"

int tst_bit(char *buf, int bit);

int set_bit(char *buf, int bit);

int clr_bit(char *buf, int bit);

int decFreeInodes(int dev);

int decFreeBlocks(int dev);

int ialloc(int dev);

int balloc(int dev);

int enter_name(MINODE *pip, int ino, char *name);

#endif 