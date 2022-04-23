					___# Assignment 1 - Storage Manager___
The goal of this assignment is to implement a simple storage manager 
A module that is capable of reading blocks from a file on disk into memory and writing blocks from memory to a file on disk. 
The storage manager deals with pages (blocks) of fixed size (PAGE_SIZE). 
In addition to reading and writing pages from a file, it provides methods for creating, opening, and closing files. 
The storage manager has to maintain several types of information for an open file: The number of total pages in the file, the current page position (for reading and writing), the file name, and a POSIX file descriptor or FILE pointer.


Team Members:
------------------------------
YinTing Hui
GuanQuan Pei
YuTong Chen

Assets:
------------------------------
C source files: storage_mgr.c, dberror.c, test_assign1_1.c,test_additional.c, test_assign_Selftest.c
Header files: storage_mgr.h, dberror.h, test_helper.h

How ro run:
------------------------------
cd to the folder
to run the default test: $ make run
to run the additional test: $ Make Selfrun

Functions:
------------------------------
Below are the functions that has be implemented for the storage manager:

# createPageFile():
This function creates a page and fills it with '\0'.
This function first check for whether or not the file exists.
Then Fill the page size full of with '\0' characters

# openPageFile():
This function will open existing page file
it will check if the file exists

# closePageFile():
This function is to close the page file
It will check if the file is open and if so close it

# destroyPageFile():
This function is to delete the file
It will check if the file exists or not if it does destroy it

# readBlock():
This function reads the block at position pageNum from a file and stores its content in the memory pointed to by the memPage page handle.
It will first check for the existance of the file and then check if the file has less than pageNum pages.
Then calculate the offset of the block to be read then set the file pointer to the offset
Read the page from the file and then Update the file.

# getBlockPos():
This function will return the position of the page from the file.
The function will check if the file is there to get the page position.
If there is then return the current position.
Or else it will return a file not found error.

# readFirstBlock():
This function will read the first block of the file.
For this I decided it will be more efficient if we just run readBlock() but then alter the position.
To do this we simply get the position by giving it the position of "0" to go to the first position.

# readLastBlock():
This function will read the last block of the file.
To do this similar to readFirstBlock() read we use a similar method.
But instead of a position of 0 it will use the fHandle and change position to total number of pages - 1 to read the last block.

# readPreviousBlock():
This function will read the previous block from the current position.
To do this the same method is applied.
But instead will use fHandle to apply current position -1 to get previous block.

# readCurrentBlock():
This This function will read the current block from the current position.
We will use fHandle to apply current position to get current block.


# readNextBlock():
This This function will read the Next block from the current position.
We will use fHandle to apply current position + 1 to get Next block.

# writeBlock():
Write a page to disk using an absolute position.
If the file can be found and we read an exist page, use fwrite to write the content at the specfic(pageNum) position.


# writeCurrentBlock():
Write a page to disk using the current position.
If the file can be found and we read an exist page, use fwrite to write the content at the current position(curPagePos).


# appendEmptyBlock():
Increase the number of pages in the file by one. The new last page should be filled with zero bytes.
Add one page to total page number


# ensureCapacity():
This function will check if the page is filled
if not it will then go back to appendemptyblock

