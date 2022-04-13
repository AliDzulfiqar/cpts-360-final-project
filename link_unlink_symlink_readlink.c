#include "functions.h"

int link_wrapper(char *old_file, char *new_file){
    // tokenize pathname into old_file and new_file delimited by a space
    printf("old = %s\nnew = %s\n", old_file, new_file);
    link(old_file, new_file);
}

int link(char *old_file, char *new_file){
    char parent[256], child[256];
    int oino, nino, pino;
    MINODE *omip, *nmip;
    
    // verify old_file exists and is not a DIR
    oino = getino(old_file);
    if(oino == -1){
        printf("FILE DOESN'T EXIST\n");
        return -1;
    }

    omip = iget(dev, oino);
    if(S_ISDIR(omip->INODE.i_mode)){
        print("ERROR: CANNOT BE A DIR\n");
        return -1;
    }

    // new_file must not exist yet
    nino = getino(new_file);
    if(nino != -1){
        printf("ERROR: NEW FILE MUST NOT EXIST YET\n");
        return -1;
    }

    // create new_file with the same inode number of old_file
    strcpy(child, basename(new_file));
    strcpy(parent, dirname(new_file));

    pino = getino(parent);
    if (pino == -1){
        printf("ERROR: CAN'T CREATE LINK IN PARENT DIR\n");
    }

    nmip = iget(dev, pino);

    enter_name(nmip, oino, child);

    // add links count and write back
    omip->INODE.i_links_count++;
    omip->dirty = 1;
    nmip->dirty = 1;

    iput(omip);
    iput(nmip);
}

int unlink(char *filename){
    char parent[256], child[256];
    int ino, pino;
    MINODE *mip, *pmip;

    strcpy(child, basename(filename));
    strcpy(parent, dirname(filename));
    

    // link MIP
    ino = getino(filename);
    if(ino == -1){
        printf("ERROR: GETTING LINK\n");
        return -1;
    }
    mip = iget(dev, ino);
    if(S_ISDIR(mip->INODE.i_mode)){
        printf("ERROR: CANNOT BE A DIR\n");
    }

    // check proc permission
    uint other = mip->INODE.i_mode & 0x7;
    uint group = (mip->INODE.i_mode & 0x38) >> 3;
    uint owner = (mip->INODE.i_mode & 0x1C0) >> 6;

    if(!mip->INODE.i_mode & 0x1FF){
        printf("ERROR: NO PERMISSION\n");
        return -1;
    }

    if(running->uid != mip->INODE.i_uid && running->uid != 0){ // super user
        printf("ERROR: NO PERMISSION\n");
    }

    mip->INODE.i_links_count--;
    
    if(mip->INODE.i_links_count > 0){
        mip->dirty = 1;
    }
    else{
        printf("dealloc\n");
    }

    mip->dirty = 1;
    iput(mip);

    pino = getino(parent);
    if(pino == -1){
        printf("ERROR: GETTING PARENT INO\n");
        return -1;
    }
    pmip = iget(mip->dev, pino);
    rm_child(pmip, child);
}

int symlink(char *old_file, char *new_file){
    int oino, nino;
    MINODE *mip;

    oino = getino(old_file);
    if(oino == -1){
        printf("ERROR: DOES NOT EXIST\n");
        return -1;
    }

    creat_file(new_file);

    mip = iget(dev, nino);
    mip->INODE.i_mode = 0xA1FF;
    mip->dirty = 1;

    // write the string old into the i_block for 60 chars
    // 60 + 24 = 84 for old
    strncpy(mip->INODE.i_block, old_file, 84);


    mip->INODE.i_size = strlen(old_file) + 1;

    // write back to disk
    mip->dirty = 1;
    iput(mip);
}

// int readlink(char *file, char *buf){
//     int ino;
//     MINODE *mip;
//     ino = getino(file);
//     mip = iget(dev, ino);
//     if(!S_ISLNK(mip->INODE.i_mode)){
//         printf("ERROR: NOT A LNK\n");
//     }

//     strncpy(mip->INODE.i_block, file, buf);

//     return mip->INODE.i_size = strlen(file) +1;
// }