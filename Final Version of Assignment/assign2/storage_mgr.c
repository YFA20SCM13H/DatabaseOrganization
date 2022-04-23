#include<stdio.h>
#include "dberror.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include<math.h>
#include "storage_mgr.h"
//to make sure it will run, only for reminder that it has started
void initStorageManager(void)
{
    printf("Sets the storage manager");
}

//makes a page in a file
RC createPageFile(char *fileName)
{
    FILE *fp;
    int i;
    fp = fopen(fileName, "w+"); // opening file in read mode
    SM_PageHandle emptyPage = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
    if (fp == NULL) //check if file exist
    {
        RC_message = "file couldn't be created";
        return RC_FILE_NOT_FOUND;
    }
    else { //fill the page with "\0"
        fwrite(emptyPage, sizeof(char), PAGE_SIZE, fp);
    }
    fclose(fp);
    free(emptyPage);
    return RC_OK;
}

//open page file
RC openPageFile(char *fileName, SM_FileHandle *fHandle) {
    FILE*fp;
    fp = fopen(fileName, "r+");
    if (fp != NULL) {
        fHandle->mgmtInfo = fp;
        fHandle->fileName = fileName;
        fseek(fHandle->mgmtInfo, 0, SEEK_SET);
        int startPos = ftell(fHandle->mgmtInfo);
        fHandle->curPagePos = startPos;
        fwrite(fileName,PAGE_SIZE,0,fp);
        fHandle->totalNumPages = 1;
        fclose(fp);
        return RC_OK;
    }
    else {
        RC_message = "Sorry, fail to open this file.";
        return RC_FILE_NOT_FOUND;
    }
}

RC closePageFile(SM_FileHandle *fHandle) {
    // Checking if file pointer or the storage manager is intialised. If initialised, then close.
    if (fclose(fHandle->mgmtInfo) == 0) {
        return RC_OK;
    }
    else {
        return RC_FILE_NOT_FOUND;
    }
}
//destroys the page
RC destroyPageFile(char *fileName) {
    FILE *fp;
    fp = fopen(fileName, "r");
    if (fp == NULL)
        return RC_FILE_NOT_FOUND;
    remove(fileName);//remove the filename to destroy page file.
    return RC_OK;
}
//read block function
RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    FILE *fp;
    fp = fopen(fHandle->fileName, "r");
    //check whether file is open
    if(fp == NULL){
        return RC_FILE_NOT_FOUND;
    }
    fseek(fp, (pageNum * PAGE_SIZE), SEEK_SET);
    if(fread(memPage, 1, PAGE_SIZE, fp) > PAGE_SIZE){
        return RC_READ_NON_EXISTING_PAGE;
    }
    //update the file handle
    fHandle->curPagePos = ftell(fp);
    fclose(fp);
    return RC_OK;
}

//gives the current position of the block
int getBlockPos (SM_FileHandle *fHandle)
{
    if(fHandle!= NULL)
        return fHandle->curPagePos;
    else
        return RC_FILE_NOT_FOUND;
}

//function to read the first block
RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    //through readBlock we have already checked and opened the file so we can just redirect what is needed from that file
    int findBlock;
    findBlock = readBlock(0, fHandle, memPage);
    return findBlock;
}
//function to read the last page of the file
RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    //to find the last block you take the total number of pages and minus 1
    int findBlock;
    findBlock = readBlock(fHandle->totalNumPages - 1, fHandle, memPage);
    return findBlock;
}

//reads previous block in relation to current position of the block
RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    //to find the previous block you subtract current one by 1
    int findBlock;
    findBlock = readBlock(fHandle->curPagePos - 1, fHandle, memPage);
    return findBlock;
}


//read Current Block by curPagePos
RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {

    int findBlock;
    findBlock = readBlock(fHandle->curPagePos, fHandle, memPage);
    return findBlock;

}
//read Current Block by curPagePos+1
RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    int findBlock;
    findBlock = readBlock(fHandle->curPagePos + 1, fHandle, memPage);
    return findBlock;
}


/* writing blocks to a page file */

RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
    FILE* fp;
    //if file is null
    if (fHandle->mgmtInfo == NULL)
        return RC_FILE_NOT_FOUND;
    int cur_pos = (pageNum)*PAGE_SIZE;
    fp = fopen(fHandle->fileName, "r+");
    if(pageNum!=0){
        fHandle->curPagePos = cur_pos;
        fclose(fp);
        writeCurrentBlock(fHandle,memPage);
        
    }else{
        fseek(fp,cur_pos,SEEK_SET);
        int i=0;
        while(i<PAGE_SIZE){
            fputc(memPage[i],fp);
            i++;
        }
        fHandle->curPagePos = ftell(fp);
        fclose(fp);
        
    }
        return RC_OK;
}

//Write Current Block
RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    FILE * fp;
    fp = fopen(fHandle->fileName, "r+");
    if(fHandle->mgmtInfo == NULL)
        return RC_FILE_NOT_FOUND;
    if (fHandle->curPagePos < 0) {
        return RC_WRITE_FAILED;
    }
    int curPos;
    curPos = fHandle->curPagePos;
    fseek(fp,curPos,SEEK_SET);
    fwrite(memPage,1,strlen(memPage),fp);
    fHandle->curPagePos = ftell(fp);
    fclose(fp);
    return RC_OK;
}

//Append Empty Block
RC appendEmptyBlock(SM_FileHandle *fHandle)
{
    fHandle->mgmtInfo = fopen(fHandle->fileName, "a");
    if (fHandle == NULL) {
        return RC_FILE_NOT_FOUND;
    }
    //Add one page to total page number
    fHandle->totalNumPages += 1;
    //Update current position number
    fHandle->curPagePos = fHandle->totalNumPages - 1;
    // Creating an empty page of size PAGE_SIZE bytes
    char *emptyBlock = (char *)calloc(PAGE_SIZE, sizeof(char));
    long appendBlock = fwrite(emptyBlock, PAGE_SIZE, sizeof(char), fHandle->mgmtInfo);
    if (appendBlock == 1) {
        int i;
        FILE *fp;
        fp = fopen(fHandle->fileName, "r+");
        //seek to the end of the file
        fseek(fp, 0, SEEK_END);
        //writes to the empty page
        for (i = 0; i < PAGE_SIZE; i++)
        {
            fwrite("\0", 1, 1, fp);
            fseek(fp, 0, SEEK_END);
        }
        free(emptyBlock);
        return RC_OK;
    }
    else {
        return RC_WRITE_FAILED;
    }
    
}
// ensureCapacity
RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle) {
    // Open the file in 'a' mode which will open the file to append the data at the end of file.
    FILE *fp;
    fp = fopen(fHandle->fileName, "a");

    if (fp == NULL)
        return RC_FILE_NOT_FOUND;
    //if capacity is enough, no need to append; if not, appendEmptyBlock
    while (numberOfPages > fHandle->totalNumPages)
        appendEmptyBlock(fHandle);

    fclose(fp);//Close the page file
    return RC_OK;
}

