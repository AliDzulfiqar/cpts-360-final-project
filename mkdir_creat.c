#include "functions.h"
#include "alloc.h"

extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;

extern char gpath[128];
extern char *name[64];
extern int n;

extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, iblk;

int mk_dir(char *pathname)
{   
    MINODE *start;
    char pname[128], bname[128];

    strcpy(pname, pathname);
    strcpy(bname, pathname);

    if(pathname[0] == '/') //if the pathname is relative
    {
        start = root; //set the MINODE to root
        dev = root->dev;
    }
    else // the pathname is absolute
    {
        start = running->cwd;
        dev = running->cwd->dev;
    }

    char *parent = dirname(pname); 
    char *child = basename(bname);

    int pino = getino(parent);

    //check if pino is empty

    if(pino == -1)
    {
        printf("Error: %s does not !\n", parent);
        return -1;
    }

    MINODE *pmip = iget(dev,pino);

    if(!S_ISDIR(pmip->INODE.i_mode))
    {
        printf("Error: %s is not a directory!\n", parent);
        return -1;
    }

    if(search(pmip, child) != 0)
    {
        printf("Error: %s is a basename that should not exist in parent directory.", child);
        return -1;
    }

    kmkdir(pmip, child);
    pmip->INODE.i_links_count++;
    pmip->INODE.i_atime = time(0L);
    pmip->dirty = 1; // mark the pmip dirty
    iput(pmip);
    return 0;

}

int kmkdir(MINODE *pip, char *name)
{
    MINODE *mip;
    char *buf[BLKSIZE], *cp;
    DIR *dp;

    int ino = ialloc(dev);
    int blk = balloc(dev);

    mip = iget(dev,ino);

    INODE *ip = &mip->INODE;
    ip->i_mode = 0x41ED; //040755: DIR type and permissions
    ip->i_uid = running->uid; //owner uid
    ip->i_gid = running->gid; //group id
    ip->i_size = BLKSIZE; //size in bytes
    ip->i_links_count = 2; //2 because of . and ..
    ip->i_atime = time(0L);
    ip->i_ctime = time(0L);
    ip->i_mtime = time(0L);
    ip->i_blocks = 2; //LINUX: Blocks count in 512-byte chunks
    ip->i_block[0] = blk;

    mip->dirty = 1; // minode is dirty
    iput(mip); //write INODE back to disk

    get_block(dev,blk,buf);
    bzero(buf,BLKSIZE); //clear buf[] to 0
    dp = (DIR *)buf;
    cp = buf;

    dp->inode = ino;
    dp->rec_len = 12;
    dp->name_len = 1;
    dp->name[0] = '.';


    cp += dp->rec_len;
    dp = (DIR *)cp;

    dp->inode = pip->ino;
    dp->rec_len = BLKSIZE - 12; //rec_len spans block
    dp->name_len = 2;
    dp->name[0] = '.';
    dp->name[1] = '.';

    put_block(dev,blk,buf); //write to blk on disks
    enter_name(pip, ino, name); // add name to block

    return 0;
}

int my_creat(char *pathname)
{
    MINODE *start;
    char pname[128], bname[128];

    strcpy(pname, pathname);
    strcpy(bname, pathname);

    if(pathname[0] == '/') //if the pathname is relative
    {
        start = root; //set the MINODE to root
        dev = root->dev;
    }
    else // the pathname is absolute
    {
        start = running->cwd;
        dev = running->cwd->dev;
    }

    char *parent = dirname(pname); 
    char *child = basename(bname);

    int pino = getino(parent);

    //check if pino is empty

    if(pino == -1)
    {
        printf("Error: %s does not !\n", parent);
        return -1;
    }

    MINODE *pmip = iget(dev,pino);

    if(!S_ISDIR(pmip->INODE.i_mode))
    {
        printf("Error: %s is not a directory!\n", parent);
        return -1;
    }

    if(search(pmip, child) != 0)
    {
        printf("Error: %s is a basename that should not exist in parent directory.", child);
        return -1;
    }

    kcreat(pmip, child);
    pmip->INODE.i_atime = time(0L);
    pmip->dirty = 1; // mark the pmip dirty
    iput(pmip);
    return 0;
}

int kcreat(MINODE *pip, char *name)
{
    MINODE *mip;
    char *buf[BLKSIZE], *cp;
    DIR *dp;

    int ino = ialloc(dev); 
    int blk = balloc(dev);

    mip = iget(dev,ino);

    INODE *ip = &mip->INODE;
    ip->i_mode = 0x81A4; //0644: FILE type and permissions
    ip->i_uid = running->uid; //owner uid
    ip->i_gid = running->gid; //group id
    ip->i_size = BLKSIZE; // size in bytes
    ip->i_links_count = 1; // 1 because its a file instead of dir 
    ip->i_atime = time(0L);
    ip->i_ctime = time(0L);
    ip->i_mtime = time(0L);
    ip->i_blocks = 2; //LINUX: Blocks count in 512-byte chunks
    ip->i_block[0] = 0; //all new file have 0 number of data blocks in them
    
    for(int i = 1; i < 15; i++)
    {
        ip->i_block[i] = 0; // set all data blocks to be 0 to clear the memory
    }

    mip->dirty = 1; // minode is dirty
    iput(mip); //write INODE back to disk

    enter_name(pip, ino, name); // add name to block

    return 0;

}