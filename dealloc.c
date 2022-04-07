#include "dealloc.h"

extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;

extern char gpath[128];
extern char *name[64];
extern int n;

extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, iblk;

extern char line[128], cmd[32], pathname[128];


// int tst_bit(char *buf, int bit)
// {
//     return buf[bit / 8] & ( 1 << (bit % 8));
// }

// int set_bit(char *buf, int bit)
// {
//     if(buf[bit / 8] |= (1 << bit % 8))
//     {
//         return 1;
//     }

//     return 0;
// }

// int clr_bit(char *buf, int bit)
// {
//     if(buf[bit / 8] &= ~(1 << bit % 8))
//     {
//         return 1;
//     }

//     return 0;
// }

int idalloc(int dev,int ino)
{
  int i;  
  char buf[BLKSIZE];

  if (ino > ninodes){
    printf("inumber %d out of range\n", ino);
    return -1;
  }

  // get inode bitmap block
  get_block(dev, imap, buf);
  clr_bit(buf, ino-1);

  // write buf back
  put_block(dev, imap, buf);

  // update free inode count in SUPER and GD
  incFreeInodes(dev);
  
}

int bdalloc(int dev, int bno)
{
    char buf[BLKSIZE];
   
    get_block(dev, bmap, buf);
    clr_bit(buf, bno-1);

    
    put_block(dev, bmap, buf);

    // update free inode count in SUPER and GD
    incFreeBlocks(dev);
    
}

int incFreeInodes(int dev)
{
  char buf[BLKSIZE];

  // inc free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count++;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count++;
  put_block(dev, 2, buf);
}

int incFreeBlocks(int dev)
{
  char buf[BLKSIZE];

  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count++;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count++;
  put_block(dev, 2, buf);
}

int remove_child(MINODE *pmip, char *name)
{
    char *cp, *lastcp, buf[BLKSIZE], *start, *end, temp[256];
    DIR *dp, *prevdp, *lastdp;
    INODE *ip = &pmip->INODE;

    for (int i = 0; i < 12; i++) //go through all memory blocks
    {
        if(ip->i_block[i] = 0)
        {
            get_block(pmip->dev, ip->i_block[i], buf); // get block
            dp = (DIR *)buf;
            cp = buf;

            while(cp < buf + BLKSIZE)
            {
                //we need to get the name and assign a delimiter to try search the name
                //bzero(temp,256);
                strncpy(temp,dp->name,dp->name_len); // get name
                printf("%s\n", dp->name);
                temp[dp->name_len] = 0; //the delimiter

                //now we can compare the name using temp and name
                if(!strcmp(temp,name))
                {
                    //first and only entry in a data block
                    if(cp == buf && cp + dp->rec_len == buf + BLKSIZE)
                    {
                        //deallocate data block
                        bdalloc(pmip->dev, ip->i_block[i]);
                        
                        //reduce parent's file size by BLKSIZE
                        ip->i_size -= BLKSIZE;

                        //compact parent's i_block array
                        while(ip->i_block[i + 1] != 0 && i + 1 < 12)
                        {
                            i++;
                            get_block(pmip->dev, ip->i_block[i], buf);
                            put_block(pmip->dev, ip->i_block[i-1], buf);
                        }
                    }

                    //last entry in block
                    else if(cp + dp->rec_len >= buf + BLKSIZE)
                    {
                        //absorb rec_len to predecessor entry
                        prevdp->rec_len += dp->rec_len;
                        //add the block back with rec_len on previous dp
                        put_block(pmip->dev, ip->i_block[i], buf);
                    }

                    //entry is first but not the only entry or in middle of block
                    else
                    {
                        lastdp = (DIR *)buf;
                        lastcp = buf;

                        while(lastcp + lastdp->rec_len < buf + BLKSIZE)
                        {
                            lastcp += lastdp->rec_len;
                            lastdp = (DIR *)lastcp;
                        }

                        lastdp->rec_len += dp->rec_len; //add deleted rec_len to last entry

                        //since we shifted, we also need to change the details of both start and end of block
                        start = dp->rec_len + cp;
                        end = buf + BLKSIZE;

                        //now we shift to left
                        memcpy(dp,cp,ip->i_size);
                        

                        //after shifting, we save the block 
                        put_block(pmip->dev, ip->i_block[i],buf);
                    }

                    pmip->dirty = 1;
                    iput(pmip);
                    return 0;
                    
                }

                prevdp = dp;
                cp += dp->rec_len;
                dp = (DIR *)cp;
            }
        }
    }

    return -1;
}
