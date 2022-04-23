                                                     ___# Assignment 3 - Record Manager___
 The record manager handles tables with a fixed schema.
 Clients can insert records, delete records, update records, and scan through the records in a table.
 A scan is associated with a search condition and only returns records that match the search condition. 
 Each table should be stored in a separate page file and your record manager should access the pages of the file through the buffer manager implemented in the last assignment.



Team Members:
------------------------------
YinTing Hui
YuTong Chen
Jason M. Fetzer
Woojin Choi

Assets:
------------------------------
C source files: storage_mgr.c, dberror.c, test_assign3_1.c, buffer_mgr_stat.c, buffer_mgr.c, test_expr.c, record_mgr.c, expr.c, rm_serializer.c
Header files: storage_mgr.h, dberror.h, test_helper.h, buffer_mgr.h, buffer_mgr_stat.h, record_mgr.h, dt.h, expr.h, tables.h

How ro run:
------------------------------
cd to the folder
to run the default test: $ make all

Functions:
------------------------------
Below are the functions that has be implemented for the Record Manager:

# initRecordManager():
This function is to initialize the record manager

# shutdownRecordManager():
This function is to shutdown the record manager

# createTable():
This function creates a table
Creates the underlying page file and store information in the Table Information pages.

# openTable():
This function opens the table
This function is required inorder to use all other functions

# closeTable():
This function closes the table
Closing a table will cause all outstanding changes to the table to be written to the page file. 

# deleteTable():
This function deletes the table

# getNumTuples():
This function returns the number of tuples in the table.

# insertRecord():
This function is to insert a new record

# deleteRecord():
This function is to delete a record with a certain RID

# updateRecord():
This function is to update an existing record with new values.

# getRecord():
This function is to retrieve a record with a certain RID

# startScan():
This function initiates a scan to retrieve all tuples from a table that fulfill a certain condition.
If condition is null all then all tuples should be returned.

# next():
This method should return the next tuple that fulfills the scan condition.

# closeScan():
This function indicates to the record manager that all associated resources can be cleaned up.

# getRecordSize():
This function is used to return the size in bytes of records for a given schema.

# createSchema():
This function creates a schema.

# freeSchema():
This function will free the schema.

# createRecord():
This function creates a new function.
It will allocate memory to the data field to hold the binary representations for all attributes of this record as determined by the schema.

# freeRecord():
This function will free the record.

# getAttr():
Getting attributes from the schema, the data types that will be differentiated are String, int, float and bool

# setAttr():
Setting attributes from the schema, the data types that will be differentiated are String, int, float and bool
