
#include "storage_mgr.h"
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
int starting_value;
//to make sure it will run, only for reminder that it has started
void initStorageManager()
{
    printf("Initializing Storage Manager...\n");
}

//makes a page in a file
RC createPageFile(char *fileName)
{
    FILE *fp;
    int i;
    fp = fopen(fileName, "w+"); // opening file in read mode
    char *content;
    content = (char *) calloc (PAGE_SIZE, sizeof(char));
    if(fp == NULL) //check if file exist
        {
        RC_message = "file couldn't be created";
        return RC_FILE_NOT_FOUND;
    }
    else{ //fill the page with "\0"
        fseek(fp, 0, SEEK_SET);
        for (i = 0; i < PAGE_SIZE; i++){
            fwrite("\0", 1, 1, fp);
            fseek(fp, 0, SEEK_END);
        }
        return RC_OK;
          }
    }
        
//open page file
RC openPageFile (char *fileName, SM_FileHandle *fHandle){
    FILE*fp;
    fp = fopen(fileName, "r+");

    if(fp != NULL){
        long len;
        int pageNum;
        fseek(fp, 0, SEEK_END);
        len = ftell(fp);
//      printf("total size = %ld \n", len);
//use total file char lenth/PAGE_SIZE to get pageNum, since we add/append page by use number of chars *PAGE_SIZE. So we assume there is no mod of pageNum
        pageNum = (int)(len)/PAGE_SIZE;
        fHandle->fileName = fileName;
        fHandle->totalNumPages = pageNum;
        fHandle->curPagePos = 0;
        fHandle->mgmtInfo = fp;
        return RC_OK;
    }
    else{
        RC_message = "Sorry, fail to open this file.";
        return RC_FILE_NOT_FOUND;
    }
}


//closePageFile
 RC closePageFile (SM_FileHandle *fHandle) {
    // Checking if file pointer or the storage manager is intialised. If initialised, then close.
     if(fclose(fHandle->mgmtInfo)==0){
         return RC_OK;
     }else{
         return RC_FILE_NOT_FOUND;
     }
 }

// destroyPageFile
RC destroyPageFile (char *fileName) {
    FILE *fp;
    fp = fopen(fileName, "r");
    if(fp == NULL)
        return RC_FILE_NOT_FOUND;
    remove(fileName);//remove the filename to destroy page file.
    return RC_OK;
}

//read block function
RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    //check whether file is open
    if (fHandle->fileName == NULL)
    {
        return RC_FILE_NOT_FOUND;
    }

    //check if the page that is to be read exist
    if (pageNum >= 0 && pageNum < fHandle->totalNumPages)
    {
        //caculate the offset of the block to be read
        long int fileOffset = pageNum * PAGE_SIZE + sizeof(int);
        fseek(fHandle->mgmtInfo, fileOffset, SEEK_SET);  //set the file pointer to the begining of the page
        fread(memPage, sizeof(char), PAGE_SIZE, fHandle->mgmtInfo); //read a page from file

        //update the file handle
        fHandle->curPagePos = pageNum;
        fseek(fHandle->mgmtInfo, fileOffset, SEEK_SET);

        return RC_OK;
    }
    else {
        return RC_READ_NON_EXISTING_PAGE;
    }

}

//gets the current position of the block
int getBlockPos(SM_FileHandle *fHandle)
{
    if (fHandle != NULL)
    {
        return fHandle->curPagePos;
    }
    else
    {
        return RC_FILE_NOT_FOUND;
    }
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
    findBlock = readBlock(fHandle->totalNumPages-1, fHandle, memPage);
    return findBlock;
}

//function to read the page before the current on file
RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    //to find the previous block you subtract current one by 1
    int findBlock;
    findBlock = readBlock(fHandle->curPagePos - 1, fHandle, memPage);
    return findBlock;
}


//read Current Block by curPagePos
RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){

    int findBlock;
    findBlock =readBlock(fHandle->curPagePos, fHandle, memPage);
    return findBlock;

}
//read Current Block by curPagePos+1
RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    int findBlock;
    findBlock =readBlock(fHandle->curPagePos+1, fHandle, memPage);
    return findBlock;
}



/* writing blocks to a page file */

RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
    //if file name is null
    if (access(fHandle->fileName, R_OK) != 0){
        return RC_FILE_NOT_FOUND;
    //if seek failed, fseek return -1;
    }else if(fseek(fHandle->mgmtInfo, pageNum*PAGE_SIZE,SEEK_SET) != 0){
        return RC_READ_NON_EXISTING_PAGE;
        // if write success, fwrite return 1
    }else if(fwrite(memPage, PAGE_SIZE, sizeof(char), fHandle->mgmtInfo) == 1){
        fHandle->curPagePos = pageNum;
        fclose(fHandle->mgmtInfo);
        return RC_OK;
    }else{
        return RC_WRITE_FAILED;

    }
}


//fclose of not? Need to discuss
// use writeBlock and curPagePos to achieve writeCurrentBlock
RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    if(fHandle->curPagePos < 0){
        return RC_WRITE_FAILED;
    }
    writeBlock(fHandle->curPagePos, fHandle, memPage);
    return RC_OK;
}

// append Empty Block
RC appendEmptyBlock (SM_FileHandle *fHandle)
{
    fHandle->mgmtInfo = fopen(fHandle->fileName,"a");
    if(fHandle == NULL){
    return RC_FILE_NOT_FOUND;
}
    //Add one page to total page number
    fHandle->totalNumPages += 1;
    //Update current position number
    fHandle->curPagePos = fHandle->totalNumPages - 1;
    // Creating an empty page of size PAGE_SIZE bytes
    char *emptyBlock = (char *) calloc (PAGE_SIZE,sizeof(char));
    long appendBlock = fwrite(emptyBlock, PAGE_SIZE, sizeof(char), fHandle->mgmtInfo);
        if(appendBlock == 1){
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
            return RC_OK;
        }else{
            return RC_WRITE_FAILED;
        }
}

// ensureCapacity
 RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle) {
    // Open the file in 'a' mode which will open the file to append the data at the end of file.
    FILE *fp;
    fp = fopen(fHandle->fileName, "a");

    if(fp == NULL)
        return RC_FILE_NOT_FOUND;
    //if capacity is enough, no need to append; if not, appendEmptyBlock
    while(numberOfPages > fHandle->totalNumPages)
        appendEmptyBlock(fHandle);
   
	 fclose(fp);//Close the page file
    return RC_OK;
}

