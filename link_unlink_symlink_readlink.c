#include "functions.h"

int link(char *old_file, char *new_file){
    char parent[256], child[256];
    int oino, nino, pino;
    MINODE *omip, *nmip, *pmip;
    
    // verify old_file exists and is not a DIR
    oino = getino(old_file);
    omip = iget(dev, oino);
    if(S_ISDIR(omip->INODE.i_mode)){
        print("ERROR: CANNOT BE A DIR\n");
    }

    // new_file must not exist yet
    nino = getino(new_file);
    if(nino != 0){
        printf("ERROR: NEW FILE MUST NOT EXIST YET\n");
    }

    // create new_file with the same inode number of old_file
    strcpy(child, basename(new_file));
    strcpy(parent, dirname(new_file));

    pino = getino(parent);
    pmip = iget(dev, pino);

    enter_name(pmip, oino, child);

    // add links count and write back
    omip->INODE.i_links_count++;
    omip->dirty = 1;
    iput(omip);
    iput(pmip);
}

int unlink(char *filename){
    char parent[256], child[256];
    int ino, pino;
    MINODE *mip, *pmip;

    ino = getino(filename);
    mip = iget(dev, ino);
    if(S_ISDIR(mip->INODE.i_mode)){
        printf("ERROR: CANNOT BE A DIR\n");
    }

    strcpy(child, basename(filename));
    strcpy(parent, dirname(filename));
    pino = getino(parent);
    pmip = iget(dev, pino);

    rm_child(pmip, ino, child);
    pmip->dirty = 1;
    iput(pmip);

    mip->INODE.i_links_count--;
    
    if(mip->INODE.i_links_count > 0){
        mip->dirty = 1;
    }
    else{
        // deallocate all data blocks
        // deallocate INODE
    }
    iput(mip);
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

    strncpy(mip->INODE.i_block, old_file, 84);

    mip->INODE.i_size = strlen(old_file) + 1;

    mip->dirty = 1;
    iput(mip);

    // mark new_file parent minode dirty
    // iput parent minode

}

int readlink(char *file, char *buf){
    int ino;
    MINODE *mip;
    ino = getino(file);
    mip = iget(dev, ino);
    if(!S_ISLNK(mip->INODE.i_mode)){
        printf("ERROR: NOT A LNK\n");
    }

    strncpy(mip->INODE.i_block, file, buf);

    return mip->INODE.i_size = strlen(file) +1;
}