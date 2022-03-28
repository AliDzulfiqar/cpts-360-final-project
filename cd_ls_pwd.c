#include "functions.h"



/************* cd_ls_pwd.c file **************/
int cd()
{
  int ino = getino(pathname);
  if(ino == 0){
    printf("ERROR: INO NOT FOUND\n");
    return -1;
  }

  MINODE *mip = iget(dev, ino);

  if(!S_ISDIR(mip->INODE.i_mode)){
    printf("ERROR: NOT A DIR\n");
    return -1;
  }

  iput(running->cwd);
  running->cwd = mip;
  return 0;
}

int ls_file(MINODE *mip, char *name)
{
  printf("ls_file: to be done: READ textbook!!!!\n");
  // READ Chapter 11.7.3 HOW TO ls
  return 0;
}

int ls_dir(MINODE *mip)
{
  printf("ls_dir: list CWD's file names; YOU FINISH IT as ls -l\n");

  char buf[BLKSIZE], temp[256];
  DIR *dp;
  char *cp;

  get_block(dev, mip->INODE.i_block[0], buf);
  dp = (DIR *)buf;
  cp = buf;
  
  while (cp < buf + BLKSIZE){
     strncpy(temp, dp->name, dp->name_len);
     temp[dp->name_len] = 0;
	
     printf("%s  ", temp);

     cp += dp->rec_len;
     dp = (DIR *)cp;
  }
  printf("\n");

  return 0;
}

int ls()
{
  printf("ls: list CWD only! YOU FINISH IT for ls pathname\n");
  ls_dir(running->cwd);
  return 0;
}

int rpwd(MINODE *wd)
{
  if(wd == root){
    return 0;
  }

  // get ino and parent ino
  char buf[BLKSIZE], lname[256];
  int ino;
  get_block(dev, wd->INODE.i_block[0], buf);
  int parent_ino = findino(wd, &ino);
  MINODE *pip = iget(dev, parent_ino);
  
  findmyname(pip, ino, lname);
  rpwd(pip);
  pip->dirty = 1;
  iput(pip);
  printf("/%s", lname);
  return 0;
}

void pwd(MINODE *wd)
{
  if (wd == root){
    printf("/\n");
    return;
  }
  rpwd(wd);
  printf(" ");
}




