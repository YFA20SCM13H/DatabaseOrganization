#include <stdio.h>
#include <stdlib.h>
#include "storage_mgr.h"
#include "test_helper.h"
#include "dberror.h"

// test name
char *testName;

/* test output files */
#define TESTFILE "additional_test.bin"

/* prototypes for test functions */
static void testAdditional(void);
static void testClose(void);


/* main function running all tests */
int main (void) {
    testName = "";
    initStorageManager();
    testClose();
    testAdditional();

    return 0;
}


void testClose(void){
    SM_FileHandle fh;

testName = "test close methods";

TEST_CHECK(createPageFile (TESTFILE));

TEST_CHECK(openPageFile (TESTFILE, &fh));

TEST_CHECK(closePageFile (&fh));

}


void testAdditional(){
    SM_FileHandle fh;
    SM_PageHandle ph;
    int i;
    testName = "test Append Empty Block";
    ph = (SM_PageHandle) malloc(PAGE_SIZE);
    // create a new page file
    TEST_CHECK(createPageFile (TESTFILE));
    printf("created  file success\n");
    TEST_CHECK(openPageFile (TESTFILE, &fh));
    printf("opened  file success\n");
    printf("The total page number of file just opened is: ");
    printf( "%d\n",fh.totalNumPages);
    // change ph to be a string and write that one to disk
    for (i=0; i < PAGE_SIZE; i++)
        ph[i] ='0';
    TEST_CHECK(writeBlock (0, &fh, ph));
//    for (i=0; i < PAGE_SIZE; i++)
//    printf("%c",ph[i]);
    printf("wrote first block\n");

    testName ="test append file";
    TEST_CHECK(appendEmptyBlock (&fh));
    printf("The total page number of file just opened is: ");
    printf( "%d\n",fh.totalNumPages);
    //test total page number
    ASSERT_TRUE((fh.totalNumPages == 2), "After append a new block, the page number is expexted 2");
    //test current page position
    ASSERT_TRUE((fh.curPagePos == 1), "After append a new block, the current page position is expexted 1");

    
    // read pages into handle
    //test readCurrentBlock and writeCurrentBlock
    for (i=0; i < PAGE_SIZE; i++)
        ph[i] = 'c';
    TEST_CHECK(writeBlock (1,&fh, ph));
    printf("wrote block 2\n");

    printf("---The current page position of file just read is: ");
    printf( "%d\n",fh.curPagePos);
    TEST_CHECK(readCurrentBlock(&fh, ph));
    for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == 'c'), "expected 'c' in current page");
    printf("The total page number of file just read is: ");
    printf( "%d\n",fh.totalNumPages);
    printf("Current block was 'c'\n");
    
    printf("---The current page position of file just read is: ");
    printf( "%d\n",fh.curPagePos);
    
    //test read PreviousBlock
    TEST_CHECK(readPreviousBlock(&fh, ph));
    printf("The current page position of file readPreviousBlock is: ");
    printf( "%d\n",fh.curPagePos);
   


    //test readNextBlock
    appendEmptyBlock (&fh);
    for (i=0; i < PAGE_SIZE; i++)
        ph[i] = 'n';
    TEST_CHECK(writeCurrentBlock (&fh, ph));
    printf("wrote block 3 \n");
    fh.curPagePos=1;
    TEST_CHECK(readNextBlock(&fh, ph));
    for (i=0; i < PAGE_SIZE; i++)
      ASSERT_TRUE((ph[i] == 'n'), "expected 'n' in next page");
    printf("The total page number of file just read is: ");
    printf( "%d\n",fh.totalNumPages);
    printf("Next block was 'n'\n");


    //test readLastBlock
    appendEmptyBlock (&fh);
    for (i=0; i < PAGE_SIZE; i++)
        ph[i] = 'L';
    TEST_CHECK(writeCurrentBlock (&fh, ph));
    printf("wrote last(4) block\n");
    TEST_CHECK(readLastBlock(&fh, ph));
    for (i=0; i < PAGE_SIZE; i++)
      ASSERT_TRUE((ph[i] == 'L'), "expected 'l' in last page");
    printf("The total page number of file just read is: ");
    printf( "%d\n",fh.totalNumPages);
    printf("Last block was 'l'\n");
    
   //  test ensureCapacity
    TEST_CHECK(ensureCapacity(4, &fh));
    ASSERT_TRUE((fh.totalNumPages == 4), "the page number is expexted 4");
    TEST_CHECK(ensureCapacity(8, &fh));
    ASSERT_TRUE((fh.totalNumPages == 8), "the page number is expexted 8");
    TEST_CHECK(destroyPageFile (TESTFILE));
    TEST_DONE();
}
