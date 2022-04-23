                                                  ___# Assignment 2 - Buffer Manager___
The buffer manager manages a fixed number of pages in memory that represent pages from a page file managed by the storage manager implemented in assignment 1.
The memory pages managed by the buffer manager are called page frames or frames for short.
We call the combination of a page file and the page frames storing pages from that file a Buffer Pool.
The Buffer manager should be able to handle more than one open buffer pool at the same time.
However, there can only be one buffer pool for each page file. 
Each buffer pool uses one page replacement strategy that is determined when the buffer pool is initialized. 
You should at least implement two replacement strategies FIFO and LRU.


Team Members:
------------------------------
YinTing Hui
YuTong Chen
GuanQuan Pei

Assets:
------------------------------
C source files: storage_mgr.c, dberror.c, test_assign2_1.c, buffer_mgr_stat.c, buffer_mgr.c
Header files: storage_mgr.h, dberror.h, test_helper.h, buffer_mgr.h, buffer_mgr_stat.h

How ro run:
------------------------------
cd to the folder
to run the default test: $ make 

Functions:
------------------------------
Below are the functions that has be implemented for the Buffer Manager:

# initBufferPool():
This function is used to create a Buffer Pool for an existing page file
Setting all PageFrame to empty
Under the frame structure of pageNum, dirtyFlag, fixCount, data and recentUseTime

# shutdownBufferPool():
This function is to destroy the buffer pool.
And free up all resources associated with buffer pool to prevent memory leak.

# forceFlushPool():
This function will write all dirty pages from the buffer pool to the disk

# markDirty():
This function is to mark a page as dirty

# unpinPage():
This function is to unpin the page.
As it is done reading to be set free, by decreasing the fix count.

# forcePage():
This function should write the current content of the page back to the page file on disk.
By looping through the pageNum and writing it back when there is dirty read.

# FIFO():
replacement strategy

# LRU():
replacement strategy

# CLOCK():
replacement strategy

# pinPage():
This function pins the page with page number pageNum.
The replacement strategies used are FIFO(), LRU() and CLOCK()

# getFrameContents():
This function is to return an array of PageNumbers according to the size of numPages.
Where the ith element is the number of the page stored in the ith page frame.
The function loops through numPages extracting the page number of the page inside the Page frame

# getDirtyFlags():
This function is to return an array of bools according to the size of numPages.
Where the ith element is TRUE if the page stored in the ith page frame is dirty.
The function loops through numPages extracting the dirty flag that shows that the page was modified

# getFixCounts():
This function is to return an array of integers according to the size of numPages.
Where the ith element is the fix count of the page stored in the ith page frame.
The function loops through numPages extracting the fix count integer that marks if it is in use by other users.

# getNumReadIO():
This function returns the number of pages that have been read from disk since the buffer pool has been initialized.

# getNumWriteIO():
This function returns the number of pages written to the page file since the buffer pool has been initialized.
