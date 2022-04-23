#include <stdio.h>
#include "stdlib.h"
#include "string.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"

int use = 0;
int numRead = 0;
int numWrite = 0;
int clockRef = 0;

typedef struct PageFrame
{
    int pageNum;// Page number of the Page frame
    int dirtyFlag;//if page is modified
    int fixCount;//count used by other users
    char *data;
    int recentUseTime;// the most recent time the page has been used
}PageFrame;

// Buffer Manager Interface Pool Handling
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,
          const int numPages, ReplacementStrategy strategy,
                  void *stratData){
  //  printf("initial Buffer Pool process......");
    int i;
    bm->numPages = numPages;
    bm->pageFile = (char *) pageFileName;
    bm->strategy = strategy;
    PageFrame* pf = malloc(sizeof(PageFrame)*numPages);
    for(i = 0; i < bm->numPages; i++){
        pf[i].data = NULL;
        pf[i].pageNum = -1;
        pf[i].dirtyFlag = 0;
        pf[i].fixCount = 0;
        pf[i].recentUseTime = 0;
    }
    bm->mgmtData = pf;
    numWrite = 0;
    return RC_OK;
}
RC shutdownBufferPool(BM_BufferPool *const bm){
 //   printf("shutdown Buffer Pool process......");
    if (bm == NULL || bm->numPages<0)
        printf("Invalid buffer pool!");
    PageFrame* pf = bm -> mgmtData;
    forceFlushPool(bm);
    int i;
    for(i = 0; i < bm->numPages; i++){
        if(pf[i].fixCount != 0){
            printf("Shut down failed! The frame is under use: %d\n",pf[i].pageNum);
        }
    }
    free(pf);
    bm->mgmtData = NULL;
    //bm->numPages = 0;
    return RC_OK;
}
RC forceFlushPool(BM_BufferPool *const bm){
 //   printf("force flish Buffer Pool process......");
    if (bm == NULL || bm->numPages<0)
        printf("Invalid buffer pool!");
    PageFrame* pf = bm -> mgmtData;
    int i;
    for(i = 0; i < bm->numPages; i++){
        if(pf[i].fixCount == 0 && pf[i].dirtyFlag == 1){
            SM_FileHandle fHandle;
            openPageFile(bm->pageFile, &fHandle);
            writeBlock(pf[i].pageNum, &fHandle, pf[i].data);
            pf[i].dirtyFlag = 0;
            numWrite++;
        }
  }
    return RC_OK;
}



//replacement strategies
void FIFO(BM_BufferPool *const bm, PageFrame *page){

//    printf("FIFO process......");
    PageFrame* pf = bm->mgmtData;
    int i;
    int first = numRead % bm->numPages;
    for(i = 0; i < bm->numPages; i++){
        //check if page frame is under use
        if(pf[first].fixCount == 0){
            //check if dirty
            if(pf[first].dirtyFlag == 1){
                SM_FileHandle fHandle;
                openPageFile(bm->pageFile, &fHandle);
                writeBlock(pf[first].pageNum, &fHandle, pf[first].data);
                numWrite++;
            }
            pf[first].dirtyFlag = page->dirtyFlag;
            pf[first].fixCount = page->fixCount;
            pf[first].data = page->data;
            pf[first].pageNum = page->pageNum;
            break;
        }
        //if page frame is under use
        else{
            first++;
            if(first % bm->numPages == 0)
                first = 0;
            
        }
    }
}



void LRU(BM_BufferPool *const bm, PageFrame *page)
{
//    printf("LRU process......");
    PageFrame* pf = bm->mgmtData;
    //SM_FileHandle fHandle;
    int i;
    int leastIndex = 0;
    int leastUse = 0;
    //replace the least
    for(i = 0; i < bm->numPages; i++)
    {
        if (pf[i].fixCount == 0) //check if page in use
        {
            leastIndex = i;
            leastUse = pf[i].recentUseTime;
            break;
        }
    }
    i = leastIndex + 1;
    for(i = 0; i < bm->numPages; i++){
        if(pf[i].recentUseTime < leastUse){
            leastIndex = i;
            leastUse = pf[i].recentUseTime;
        }
    }
    if(pf[leastIndex].dirtyFlag ==1){
        SM_FileHandle fHandle;
        openPageFile(bm->pageFile, &fHandle);
        writeBlock(pf[leastIndex].pageNum, &fHandle, pf[leastIndex].data);
        numWrite++;
    }
    pf[leastIndex].recentUseTime = page->recentUseTime;
    pf[leastIndex].dirtyFlag = page->dirtyFlag;
    pf[leastIndex].fixCount = page->fixCount;
    pf[leastIndex].data = page->data;
    pf[leastIndex].pageNum = page->pageNum;
}
//CLOCK page replacement Strategy
void CLOCK(BM_BufferPool *const bm, PageFrame *page)
{
    //    printf("CLOCK process......");
    PageFrame* pf = bm->mgmtData;
    SM_FileHandle fh;

    while (clockRef < bm->numPages)
    {
        clockRef = (clockRef % bm->numPages == 0) ? 0 : clockRef;

        if (pf[clockRef].recentUseTime == 0)
        {
            if (pf[clockRef].dirtyFlag == 1) //check if page is modified
            {
                openPageFile(bm->pageFile, &fh);
                writeBlock(pf[clockRef].pageNum, &fh, pf[clockRef].data);
                numWrite++; // Increase the number of writes
            }

            // Setting Content
            pf[clockRef].recentUseTime = 1 ;
            pf[clockRef].recentUseTime = page->recentUseTime;
            pf[clockRef].dirtyFlag = page->dirtyFlag;
            pf[clockRef].pageNum = page->pageNum;
            pf[clockRef].fixCount = page->fixCount;
            pf[clockRef].data = page->data;
            clockRef++;
            break;
        }
        else
        {
            pf[clockRef].recentUseTime = 0;
            clockRef++;
        }
    }
}

// Buffer Manager Interface Access Pages
RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page){
//    printf("marking dirty process......");
    if (bm == NULL || bm->numPages<0)
        printf("Invalid buffer pool!");
    int i = 0;
    PageFrame* pf = bm->mgmtData;
    for(i = 0; i< bm->numPages; i++){
        if (pf[i].pageNum== page->pageNum){
            pf[i].dirtyFlag = 1;
            return RC_OK;
            break;
        }
    }
    return RC_OK;
}


RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page){
//    printf("unpin process......");
    if (bm == NULL || bm->numPages<0)
        printf("Invalid buffer pool!");
    int i;
    PageFrame* pf = bm->mgmtData;
    for(i = 0; i< bm->numPages; i++){
        if(pf[i].pageNum == page->pageNum){
            //unpinning need to reduces its fix count
            pf[i].fixCount--;
            return RC_OK;
            break;
        }
    }
        
    return RC_OK;
}
RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page){
    // write the current content of the page back to the page file on disk
//    printf("forcePage process......");

    int i = 0;
    PageFrame* pf = bm->mgmtData;
    if (bm == NULL || bm->numPages<0)
        printf("Invalid buffer pool!");

    for(i = 0; i< bm->numPages; i++){
        if(pf[i].pageNum == page->pageNum){
            SM_FileHandle fHandle;
            openPageFile(bm->pageFile, &fHandle);
            writeBlock(pf[i].pageNum, &fHandle, pf[i].data);
            pf[i].dirtyFlag = 0;
            numWrite++;
            //closePageFile(&fHandle);
        }
    }
    return RC_OK;
}

/*
 pinPage pins the page with page number pageNum.
 The buffer manager is responsible to set the pageNum field of the page handle passed to the method.
 Similarly, the data field should point to the page frame the page is stored in
 (the area in memory storing the content of the page).
 Here is the algrithm:
initialize the bool fullCheck to check if the buffer pool is full
for all buffer:
    if(current page frame not empty)
        if the page requested by the client is already cached in this page frame
        the buffer simply returns a pointer to this page frame to the client
        fullCheck = 0
    else(there is unused frame in buffer pool and the page not in it)
        if there is, read data from disk into that frame
        fullCheck = 0
if fullCheck = 1(buffer pool is full)
    ....
 */

RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page,const PageNumber pageNum){
//    printf("pinPage process......");

//    if (bm == NULL || bm->numPages<0)
//        printf("Invalid buffer pool!");
    PageFrame* pf = bm->mgmtData;
    //current page frame is empty
    if (pf[0].pageNum == -1){
        //allocating memory
        pf[0].data = (SM_PageHandle) malloc(PAGE_SIZE);
        SM_FileHandle fHandle;
        openPageFile(bm->pageFile, &fHandle);
        ensureCapacity(pageNum,&fHandle);
        readBlock(pageNum, &fHandle, pf[0].data);
        pf[0].pageNum = pageNum;
        pf[0].fixCount++;
        numRead = 0;
        use= 0;
        pf->recentUseTime = use;
        page->pageNum = pageNum;
        page->data =pf[0].data;
        return RC_OK;
    }else{
        int i;
        bool fullCheck = 1;
        for(i = 0; i<bm->numPages ; i++){
            if(pf[i].pageNum != -1){
                // check if the page in buffer
                if(pf[i].pageNum == pageNum){
                    pf[i].fixCount++;
                    //use to record recentUseTime in LRU
                    use++;
                    fullCheck = 0;
                    if(bm->strategy == RS_CLOCK)
                        pf[i].recentUseTime = 0;
                    if(bm->strategy == RS_LRU)
                        pf[i].recentUseTime = use;
                    page->pageNum = pageNum;
                    page->data = pf[i].data;
                    clockRef++;
                    break;
                }
            }else {
                SM_FileHandle fHandle;
                openPageFile(bm->pageFile, &fHandle);
                //pf[0].data = (SM_PageHandle) malloc(PAGE_SIZE);
                pf[i].data = (SM_PageHandle) malloc(PAGE_SIZE);
                readBlock(pageNum, &fHandle, pf[i].data);
                pf[i].pageNum = pageNum;
                pf[i].fixCount = 1;
                numRead++;
                use++;
                if(bm->strategy == RS_CLOCK)
                    pf[i].recentUseTime = 0;
                if(bm->strategy == RS_LRU)
                    pf[i].recentUseTime = use;                
                page->pageNum = pageNum;
                page->data = pf[i].data;
                fullCheck = 0;
        
                break;
            }
        }if(fullCheck == 1){
            PageFrame *newpf = malloc(sizeof(PageFrame));
            SM_FileHandle fHandle;
            openPageFile(bm->pageFile, &fHandle);
            newpf->data = (SM_PageHandle) malloc(PAGE_SIZE);
            readBlock(pageNum, &fHandle, newpf->data);
            newpf->pageNum = pageNum;
            newpf->dirtyFlag = 0;
            newpf->fixCount = 1;
            use++;
            numRead++;
            if(bm->strategy == RS_CLOCK)
                newpf->recentUseTime = 0;
            if(bm->strategy == RS_LRU)
                newpf->recentUseTime = use;
                    
            page->data = newpf->data;
            page->pageNum = pageNum;
            switch(bm->strategy)
            {
                case RS_FIFO: // Using FIFO algorithm
                    FIFO(bm, newpf);
                    break;
                            
                case RS_LRU: // Using LRU algorithm
                    LRU(bm, newpf);
                    break;
                            
                case RS_CLOCK: // Using CLOCK algorithm
                    CLOCK(bm, newpf);
                    break;
                            
                case RS_LFU: // Using LFU algorithm
                    printf("\n LRU-k algorithm not implemented");
                    break;
                            
                case RS_LRU_K:
                    printf("\n LRU-k algorithm not implemented");
                    break;
                            
                default:
                    printf("\nAlgorithm Not Implemented\n");
                    break;
            }
        }
        return RC_OK;
    }
}




// Statistics Interface
//These functions return statistics about a buffer pool and its contents
//The getFrameContents function returns an array of PageNumbers of the size numPages, where the ith element is the number of the page stored in the ith page fram, An empty page frame is presented using the constant NO_PAGE
PageNumber *getFrameContents(BM_BufferPool *const bm)
{
    int i;
    int MaxPage;
    MaxPage = bm->numPages;
    PageFrame* pf;
    pf = bm->mgmtData;
    PageNumber *frameContent = malloc(sizeof(PageNumber)*MaxPage);
    for (i = 0; i< MaxPage; i++){
        if(pf[i].pageNum != -1){
            frameContent[i] =pf[i].pageNum;
        }
        else{
            frameContent[i] = NO_PAGE;
        }
    }
    return frameContent;
}


//The getDirtyFlags function returns an array of bools (of size numPages) where the ith element is TRUE if the page stored in the ith page frame is dirty. Empty page frames are considered as clean.
bool *getDirtyFlags(BM_BufferPool *const bm)
{
    int i;
    int MaxPage;
    MaxPage = bm->numPages;
    PageFrame* pf;
    pf = bm->mgmtData;
    bool *dirtyCheck = malloc(sizeof(bool)*MaxPage);
    for (i = 0; i < MaxPage; i++) {
        if (pf[i].pageNum == -1 ) {
            dirtyCheck[i] = 0;
        } else {
          if (pf[i].dirtyFlag == 1)
              dirtyCheck[i] = 1;
          if (pf[i].dirtyFlag == 0)
              dirtyCheck[i] = 0;
        }
      }
      return dirtyCheck;
}


//The getFixCounts function returns an array of ints (of size numPages) where the ith element is the fix count of the page stored in the ith page frame. Return 0 for empty page frames.
int *getFixCounts(BM_BufferPool *const bm)
{
    int i;
    int MaxPage;
    MaxPage = bm->numPages;
    PageFrame* pf;
    pf = bm->mgmtData;
    int* getFixCount = malloc(sizeof(int)*MaxPage);
    for (i = 0; i < MaxPage; i++) {
        getFixCount[i] = pf[i].fixCount;
    }
    
 //   free(bp_mgmt->fixCount); //free memeory pointed to prevent memory leak

    return  getFixCount;
}


//This function gets the total number of ReadBlock operations performed
int getNumReadIO(BM_BufferPool *const bm)
{
    return numRead+1;
}

//This function gets the total number of writeBlock operations performed
int getNumWriteIO(BM_BufferPool *const bm)
{
    return numWrite;
}
