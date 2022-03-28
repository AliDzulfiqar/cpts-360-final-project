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
  char *t1 = "xwrxwrxwr-------";
  char *t2 = "----------------";

  char ftime[256];
  u16 mode = mip->INODE.i_mode;

  if (S_ISREG(mode)) 
      printf("%c", '-');
  if (S_ISDIR(mode))
      printf("%c", 'd');
  if (S_ISLNK(mode))
      printf("%c", 'l');
  for (int i = 8; i >= 0; i--)
  {
      if (mode & (1 << i))
          printf("%c", t1[i]); // print r|w|x printf("%c", t1[i]);
      else
          printf("%c", t2[i]); // or print -
  }
  printf("%4d ", mip->INODE.i_links_count); // link count
  printf("%4d ", mip->INODE.i_gid);         // gid
  printf("%4d ", mip->INODE.i_uid);         // uid
  printf("%8d ", mip->INODE.i_size);       // file size

  strcpy(ftime, ctime((time_t *)&(mip->INODE.i_mtime))); // print time in calendar form ftime[strlen(ftime)-1] = 0; // kill \n at end
  ftime[strlen(ftime) - 1] = 0;                // removes the \n
  printf("%s ", ftime);                        // prints the time

  printf("%s", name);
  if (S_ISLNK(mode))
  {
    printf(" -> %s", (char *)mip->INODE.i_block); // print linked name 
  }

  printf("\n");
  return 0;
}

int ls_dir(MINODE *mip)
{
  char *t1 = "xwrxwrxwr-------";
  char *t2 = "----------------";

  char ftime[256];
  MINODE *temp_mip;

  u16 mode = mip->INODE.i_mode;

  char buf[BLKSIZE], temp[256];
  DIR *dp;
  char *cp;

  get_block(dev, mip->INODE.i_block[0], buf);
  dp = (DIR *)buf;
  cp = buf;
  
  while (cp < buf + BLKSIZE){
     strncpy(temp, dp->name, dp->name_len);
     temp[dp->name_len] = 0;
	
     temp_mip = iget(dev, dp->inode);
     ls_file(temp_mip, temp);
     temp_mip->dirty = 1;
     iput(temp_mip);

     cp += dp->rec_len;
     dp = (DIR *)cp;
  }
  printf("\n");

  return 0;
}

int ls(char *pathname)
{
  int mode;
  if(strcmp(pathname, ""))
  {
    ls_dir(running->cwd);
  }
  else{
    int ino = getino(pathname);
    if(ino == -1)
    {
      printf("INODE DO NOT EXIST\n");
      return -1;
    }
    else{
      dev = root->dev;
      MINODE *mip = iget(dev, ino);
      mode = mip->INODE.i_mode;
      if (S_ISDIR(mode))
      {
        ls_dir(mip);
      }
      else
      {
        ls_file(mip, basename(pathname));
      }
    }
  }
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




