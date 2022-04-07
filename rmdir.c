#include "command.h"
#include "dealloc.h"

extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;

extern char gpath[128];
extern char *name[64];
extern int n;

extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, iblk;

int rm_dir(char *pathname)
{
    //get inode from pathname
    int ino = getino(pathname);

    if (ino == -1)
    {
        printf("ERROR: ino does not exist.\n");
        return -1;
    }

    MINODE *mip = iget(dev,ino);

    //verify INODE is a DIR 
    if (!S_ISDIR(mip->INODE.i_mode))
    {
        printf("ERROR: This node is not a directory.\n");
        return -1;
    }

    //verify DIR is empty
    if (!test_emptydir(mip))
    {
        printf("ERROR: DIR is not empty!\n");
        return -1;
    }

    //verify minode is not BUSY
    if(mip->refCount > 1)
    {
        printf("ERROR: This node is currently busy, since its refcount %d is > 1.\n", mip->refCount);
        iput(mip);
        return -1;
    }

    

    //get parent's ino and inode
    char *parent, *child, name1[256], name2[256];
    
    strcpy(name1, pathname);
    strcpy(name2, pathname);

    parent = dirname(name1);
    child = basename(name2);

    int pino = getino(parent);
    //int pino = findino(mip->dev, mip->INODE.i_block[0]);
    if (pino == -1)
    {
        printf("ERROR: Parent INODE not found.\n");
        return -1;
    }

    MINODE *pip = iget(mip->dev,pino);
    
    remove_child(pip,child);

    pip->INODE.i_links_count--;
    // pip->INODE.i_atime = time(0L);
    // pip->INODE.i_ctime = time(0L);
    pip->dirty = 1;
    iput(pip);



    bdalloc(mip->dev, mip->INODE.i_block[0]);
    idalloc(mip->dev, mip->ino);

    mip->dirty = 1;
    iput(mip);



    // for (int i = 0; i < 12; i++)
    // {
    //     if(mip->INODE.i_block[i] == 0)
    //     {
    //         continue;
    //     }

    //     bdalloc(mip->dev, mip->INODE.i_block[i]);
    // }
    
    // idalloc(mip->dev, mip->ino);

    // mip->dirty = 1;
    // iput(mip);

    return 0;
}

int test_emptydir(MINODE *mip)
{
    DIR *dp;
    INODE *ip = &mip->INODE;
    char *cp, buf[BLKSIZE], temp[256];

    //if links_count > 2, not empty
    if (ip->i_links_count > 2)
    {
        return 0;
    }

    //if links count = 2, may contain reg files
    else if (ip->i_links_count == 2)
    {
        //traverse DIR's data block 
        for (int i = 0; i < 12; i++)
        {
            if (ip->i_block[i] == 0)
                break;
            get_block(mip->dev, mip->INODE.i_block[i], buf);
            dp = (DIR *)buf;
            cp = buf;

            while (cp < buf + BLKSIZE)
            {
                strncpy(temp, dp->name, dp->name_len);
                temp[dp->name_len] = 0;
                //printf("%4d%8d%8u %s\n", dp->inode, dp->rec_len, dp->name_len, temp);
                //since we got the name of file as temp, we can compare temp with . and ..
                if(strcmp(temp, ".") && strcmp(temp, ".."))
                {
                    return 0;
                }

                cp += dp->rec_len;
                dp = (DIR *)cp;
            }
        }
    }
    // the directory is empty
    return 1;
}
