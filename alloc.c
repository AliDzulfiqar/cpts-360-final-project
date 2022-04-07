#include "alloc.h"

extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;

extern char gpath[128];
extern char *name[64];
extern int n;

extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, iblk;

extern char line[128], cmd[32], pathname[128];

int tst_bit(char *buf, int bit)
{
    return buf[bit / 8] & ( 1 << (bit % 8));
}

int set_bit(char *buf, int bit)
{
    if(buf[bit / 8] |= (1 << bit % 8))
    {
        return 1;
    }

    return 0;
}

int clr_bit(char *buf, int bit)
{
    if(buf[bit / 8] &= ~(1 << bit % 8))
    {
        return 1;
    }

    return 0;
}

int decFreeInodes(int dev)
{
  char buf[BLKSIZE];

  // dec free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count--;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count--;
  put_block(dev, 2, buf);
}

int decFreeBlocks(int dev)
{
  char buf[BLKSIZE];

  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count--;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count--;
  put_block(dev, 2, buf);
}

int ialloc(int dev)  // allocate an inode number from inode_bitmap
{
  int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, imap, buf);

  for (i=0; i < ninodes; i++){ // use ninodes from SUPER block
    if (tst_bit(buf, i)==0)
    {
      set_bit(buf, i);
	    put_block(dev, imap, buf);

	    decFreeInodes(dev);

	    printf("allocated ino = %d\n", i+1); // bits count from 0; ino from 1
        return i+1;
    }
  }
  return 0;
}

int balloc(int dev)
{
  int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, bmap, buf);

  for (i=0; i < nblocks; i++){ // use ninodes from SUPER block
    if (tst_bit(buf, i)==0)
    {
      set_bit(buf, i);
      decFreeBlocks(dev);
	    put_block(dev, bmap, buf);

	    printf("allocated ino = %d\n", i+1); // bits count from 0; ino from 1
        return i+1;
    }
  }
  return 0;
}

int enter_name(MINODE *pip, int ino, char *name)
{
  INODE *ip;
  DIR *dp;
  char buf[BLKSIZE], *cp;
  int bno;

  

  ip = &pip->INODE; // to obtain the inode 

  //assuming only 12 direct blocks
  for (int i = 0; i < 12; i++)
  {
    if(ip->i_block[i] == 0)
    {
      break;
    }

    // step to last entry in data block
    bno = ip->i_block[i];
    get_block(pip->dev, ip->i_block[i], buf); 
    dp = (DIR *)buf;
    cp = buf;

    while(cp + dp->rec_len < buf + BLKSIZE)
    {
      printf("%s\n", dp->name);
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }

    // now we are at the last entry in block
    int ideal_length = 4*((8 + dp->name_len + 3)/4); //the ideal length of each dir_entry
    int need_length = 4*((8 + strlen(name) +3)/4); // ideal length of last dir_entry name
    
    int remain = dp->rec_len - ideal_length;

    if(remain >= need_length)
    {
      dp->rec_len = ideal_length; // trim the previous entry rec_len to its ideal_length
      cp += dp->rec_len;  //create new entry
      dp =(DIR *)cp; //point the new entry as LAST entry

      //writing the data block to disk
      dp->inode = ino; // add the inode 
      strcpy(dp->name, name); // add the name of entry
      dp->name_len = strlen(name); // add the len of name of entry
      dp->rec_len = remain; // add the size of the record as remainder
      
      //after finishing all the info needed, we save the block to disk
      put_block(dev,bno,buf); 

      return 0;
    }

    else
    {
      //no space in existing data blocks

      //allocate new data block
      ip->i_size = BLKSIZE;  
      bno = balloc(dev);  //allocate new block
      ip->i_block[i]= bno;  // add the new block to the list 
      
      pip->dirty = 1; // we mark the ino dirty since it has changed with new block

      get_block(dev,bno,buf); // obtain the block from the memory
      dp = (DIR *)buf;
      cp = buf;

      //write the new data block to disk
      dp->inode = ino; // add the inode 
      dp->name_len = strlen(name); // add the len of name of entry
      strcpy(dp->name, name); // add the name of entry
      dp->rec_len = BLKSIZE; // parent size by BLKSIZE

      //same as above, we save the block to disk after creating the new data block
      put_block(dev,bno,buf); 
      return 1;

    }
  }
}