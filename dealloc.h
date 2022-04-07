#ifndef DEALLOC_H
#define DEALLOC_H

#include "util.h"

int tst_bit(char *buf, int bit);

int set_bit(char *buf, int bit);

int clr_bit(char *buf, int bit);

int idalloc(int dev, int ino);

int bdalloc(int dev, int bno);

int incFreeInodes(int dev);

int incFreeBlocks(int dev);

int remove_child(MINODE *pmip, char *name);

#endif