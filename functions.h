#ifndef FUNCTIONS
#define FUNCTIONS

// C Libs:
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>


// USR
#include "type.h"
#include "globals.h"


/* util.c */
int get_block(int dev, int blk, char *buf);
int put_block(int dev, int blk, char *buf);
int tokenize(char *pathname);
MINODE *iget(int dev, int ino);
int iput(MINODE *mip);
int getino(char *pathname);
int findmyname(MINODE *parent, u32 myino, char myname[]);
int findino(MINODE *mip, u32 *myino);

/* cd_ls_pwd.c */
int cd();
int ls_file(MINODE *mip, char *name);
int ls_dir(MINODE *mip);
int ls();
void pwd(MINODE *wd);

/* link_unlink_symlink_readlink */
int link(char *old_file, char *new_file);
int unlink(char *filename);
int symlink(char *old_file, char *new_file);
// int readlink(char *file, char *buf);

/* mkdir_creat */
int mk_dir(char *pathname);
int kmkdir(MINODE *pip, char *name);
int my_creat(char *pathname);
int kcreat(MINODE *pip, char *name);

/* rmdir */
int rm_dir(char *pathname);
int test_emptydir(MINODE *mip);

#endif