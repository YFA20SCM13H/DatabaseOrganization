                                                    ___# Assignment 4 - B+-Tree___
 The index should be backed up by a page file and pages of the index should be accessed through your buffer manager.
 Each node should occupy one page.
 For debugging purposes you should support trees with a smaller fan-out and still let each node occupy a full page.
 A B+-tree stores pointer to records (the RID introduced in the last assignment) index by a keys of a given datatype.



Team Members:
------------------------------
YinTing Hui
YuTong Chen
Jason M. Fetzer
Woojin Choi

Assets:
------------------------------
C source files: storage_mgr.c, dberror.c, test_assign4_1.c, buffer_mgr_stat.c, buffer_mgr.c, test_expr.c, record_mgr.c, expr.c, rm_serializer.c, bree_mgr.c
Header files: storage_mgr.h, dberror.h, test_helper.h, buffer_mgr.h, buffer_mgr_stat.h, record_mgr.h, dt.h, expr.h, tables.h, btree_mgr.h

How to run:
------------------------------
cd to the folder
to run the default test: $ make all

Functions:
------------------------------
Below are the functions that has be implemented for the Record Manager:
------------------------------

# initIndexManager():
This function is to initialize index manager.

# shutdownIndexManager():
This function is to shutdown the index manager.

# createBtree():
Create a BTree.

# openBtree():
Before a client can access a b-tree index, it has to be opened.

# closeBtree():
Closing a B-Tree.  The index manager ensures all new or modified pages are flushed back to disk using the Buffer Manager.

# deleteBtree():
This function deletes the Btree and removes the corresponding page file.

# getNumNodes():
Return the number of Nodes.

# getNumEntries():
Return the number of Entries.

# getKeyType():
Return the data type.

# findKey():
returns the RID for the entry with the search key in the b-tree. If the key does not exist this function returns RC_IM_KEY_NOT_FOUND.

# insertKey():
insertKey inserts a new key and record pointer pair into the index. It returns error code RC_IM_KEY_ALREADY_EXISTS if this key is already stored in the b-tree.

# deleteKey():
deleteKey removes a key (and corresponding record pointer) from the index. It returns RC_IM_KEY_NOT_FOUND if the key is not in the index.
For deletion it is up to the client whether this is handled as an error.

# openTreeScan():
This method initializes the scan structure

# nextEntry():
This function will use ScanHandle and output the RID in ascending order of values.
The nextEntry method returns RC_IM_NO_MORE_ENTRIES if there are no more entries to be returned. 

# closeTreeScan():
This function is to close the Tree scan so it takes the ScanHandle and free its management data

# printTree():
This method is used to print the Bplus Tree.
