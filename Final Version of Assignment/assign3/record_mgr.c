#include "dberror.h"
#include "record_mgr.h"
#include "expr.h"
#include "tables.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct RM
{
    //BufferManager's pagehandle and bufferpool needed.
    BM_PageHandle pageHandle;
    BM_BufferPool *bufferPool;
    SM_FileHandle fHandler;
    int tupleCount; //number of tuples
    int freeloc; //free page location
} RM;
// scan table
typedef struct STable
{
    int currentScan;
    Expr *condition; //condition of scan records in table
    int currentPage;
} STable;


const int MaxPage = 100;
RM *recordM;
RM_TableData *TData;


// table and manager
extern RC initRecordManager(void *mgmtData) {
    printf("initializing Record Manager...\n");
    initStorageManager();
    return RC_OK;
}

extern RC shutdownRecordManager() {
    recordM = NULL;
    free(recordM);
    printf("shut down Record Manager process...\n");
    return RC_OK;
    
}

extern RC createTable(char *name, Schema *schema) {
    SM_FileHandle fHandler;
    recordM = (RM*)malloc(sizeof(RM));
    BM_BufferPool *bufferPool = (BM_BufferPool *) malloc (sizeof(BM_BufferPool));
    int size = getRecordSize(schema);
    int pageRemain = (PAGE_SIZE - sizeof(int))/(size + sizeof(int));

    createPageFile(name);
    openPageFile(name, &fHandler);
    initBufferPool(bufferPool, name, MaxPage, RS_FIFO, NULL);
    pinPage(bufferPool,&recordM->pageHandle,0);
    
    int *tableDataInfo = (int*)&recordM->pageHandle.data;
    tableDataInfo[0] = 0;
    tableDataInfo[1] = 1;
    
    TData = (RM_TableData*)malloc(sizeof(RM_TableData));
    TData->name = name;
    TData->schema = schema;
    TData->mgmtData = malloc(sizeof(RM));
    
    recordM =(RM*) TData->mgmtData;
    recordM->tupleCount=0;
    recordM->freeloc=1;
    recordM->bufferPool = bufferPool;
    recordM->fHandler = fHandler;
 // recordM->pageHandle = pageHandle;
    appendEmptyBlock(&fHandler);
    
    BM_PageHandle *newPageHandle = (BM_PageHandle *) malloc (sizeof(BM_PageHandle));
    pinPage(bufferPool, newPageHandle, 1);
    int *pageData = (int *)newPageHandle->data;
    pageData[0] = 0;
    int *slotinfo = (int *)(newPageHandle->data + sizeof(int));
    int index;
    index = 0;
    while(index < pageRemain){
        slotinfo[index] = 0;
        index++;
    }

    unpinPage(bufferPool, newPageHandle);
    forcePage(bufferPool, newPageHandle);
    closePageFile(&fHandler);
    free(newPageHandle);
    
    return RC_OK;
}

extern RC openTable(RM_TableData *rel, char *name) {
    if(name == NULL ||TData == NULL) return RC_FILE_NOT_FOUND;
    rel->mgmtData = TData->mgmtData;
    rel->schema = TData->schema;
    rel->name = TData->name;
    return RC_OK;
}


extern RC closeTable(RM_TableData *rel) {
    rel->name = NULL;
    rel->schema = NULL;
    //no need of shutdownBufferPool() the error is "Shut down failed! The frame is under use"
    return RC_OK;
    
}

extern RC deleteTable(char *name) {
    destroyPageFile(name);
    free(TData);
    return RC_OK;
    
}

extern int getNumTuples(RM_TableData *rel) {
    return(((RM*)rel->mgmtData)->tupleCount);
    
}

// handling records in a table
extern RC insertRecord(RM_TableData *rel, Record *record) {
    // full check parameter
    int fullCheck= 0;
    int freeIndex = -1;
    int index = 0;
    //assign data
    RM *recordManager= (RM *)rel->mgmtData;
    //get record size
    int size = getRecordSize(rel->schema);
    int pageNum = (PAGE_SIZE - sizeof(int))/(size + sizeof(int));
    //find avalible page
    int page =((RM *)rel->mgmtData)->freeloc;
    pinPage(recordManager->bufferPool, &recordManager->pageHandle, page);
    //offset
    char *offSet = recordManager->pageHandle.data + sizeof(int);
    int i = 0;
    while (i < pageNum) {
        fullCheck = *(int *)offSet;
        if (fullCheck == 0) {
            break;
        }
        index += 1;
        offSet+= sizeof(int);
        i++;
    }
    //full check True
    if(index > pageNum - 1){
        index = -1;
    }
    freeIndex = index;
    //full check False
    if(freeIndex >= 0){
        int offSet = PAGE_SIZE  - (freeIndex + 1) * size * sizeof(char)-1;
        record->id.slot = freeIndex;
        record->id.page = page;
        memcpy(recordManager->pageHandle.data + offSet, record, size);
        //slot update
        int *slotInsert = (int *)(recordManager->pageHandle.data + sizeof(int));
        slotInsert[freeIndex] = 1;
        //pin the 0 page
        pinPage(recordManager->bufferPool, &recordManager->pageHandle, 0);
        int *count = (int *)recordManager->pageHandle.data;
        //increase the record number
        count[0] += 1;
        //increase the tuples count
        ((RM *)rel->mgmtData)->tupleCount += 1;
        
    }else if(freeIndex < 0){
        BM_BufferPool *bufferPool =((RM *)rel->mgmtData)->bufferPool;
        BM_PageHandle *pageHandle = (BM_PageHandle *) malloc (sizeof(BM_PageHandle));
        int newPage = ((RM *)rel->mgmtData)->freeloc + 1;
        int slot = 0;
        //add a page
        appendEmptyBlock(&((RM *)rel->mgmtData)->fHandler);
        pinPage(bufferPool, pageHandle, newPage);
        
        int offset = PAGE_SIZE  - size * sizeof(char)-1;
        record->id.slot = 0;
        record->id.page = newPage;
        memcpy(pageHandle->data + offset, record, size);
        int * findSlot = (int *)(pageHandle->data + sizeof(int));
        findSlot[slot] = 1;
        
        markDirty(recordManager->bufferPool, &recordManager->pageHandle);
        unpinPage(recordManager->bufferPool, &recordManager->pageHandle);
        recordManager->tupleCount += 1;
        recordManager->freeloc = newPage;
        free(pageHandle);
    }
    return RC_OK;
}



extern RC deleteRecord(RM_TableData *rel, RID id) {
    RM *recordManager = rel->mgmtData;
    pinPage(recordManager->bufferPool, &recordManager->pageHandle, id.page);
    recordManager->tupleCount--;
    recordManager->freeloc = id.page;
    markDirty(recordManager->bufferPool, &recordManager->pageHandle);
    unpinPage(recordManager->bufferPool, &recordManager->pageHandle);
    return RC_OK;
}

extern RC updateRecord(RM_TableData *rel, Record *record) {
    RM *recordManager = (RM *)rel->mgmtData;
    pinPage(recordManager->bufferPool,&recordManager->pageHandle, record->id.page);
    char *data = recordManager->pageHandle.data+ PAGE_SIZE  - (record->id.slot + 1) *getRecordSize(rel->schema) * sizeof(char)-1;
    int recordSize = getRecordSize(rel->schema);
    memcpy(data, record, recordSize);
    ((int *)(recordManager->pageHandle.data))[record->id.slot+1] = 1;
    if(markDirty(recordManager->bufferPool, &recordManager->pageHandle) != RC_OK) {
         return RC_RM_UNKOWN_DATATYPE;
     }
     
    if(unpinPage(recordManager->bufferPool, &recordManager->pageHandle) != RC_OK) {
         return RC_RM_UNKOWN_DATATYPE;
     }
     return RC_OK;
}

extern RC getRecord(RM_TableData *rel, RID id, Record *record) {
    RM *recordManager=rel->mgmtData; //assign recordmanager tabledata
    BM_PageHandle *pageHandle = (BM_PageHandle *) malloc (sizeof(BM_PageHandle));
    pinPage(recordManager->bufferPool, pageHandle, id.page); //pinPage;
    int recordSize = getRecordSize(rel->schema); // size of the input
    char *dataFind = pageHandle->data+ PAGE_SIZE  - (id.slot + 1) *recordSize * sizeof(char) - 1;
    memcpy( record,dataFind ,getRecordSize(rel->schema));
    unpinPage(recordManager->bufferPool, pageHandle);
    free(pageHandle);
    return RC_OK;
};

// scans
extern RC startScan(RM_TableData *rel, RM_ScanHandle *scan, Expr *cond) {
    STable *sTable = malloc(sizeof(STable));
    sTable->condition=cond;
    sTable->currentScan=0;
    sTable->currentPage=1;
    scan->mgmtData=sTable;
    scan->rel=rel;
    return RC_OK;
    //openTable();
    //scan using malloc
}

extern RC next(RM_ScanHandle *scan, Record *record) {
    //initialize type of data>> table, schema, scan data
    //keep track of total scancount and scanNumber
    //move pointer and pinPage, memcpy, unpin
    int count,tuple,index, slot;
    RID rid;
    Value *value=malloc(sizeof(Value));
    count=((STable *)scan->mgmtData)->currentScan;
    rid.page=((STable *)scan->mgmtData)->currentPage;
    rid.slot=count;
    tuple=((RM *)scan->rel->mgmtData)->tupleCount;
    slot=(PAGE_SIZE-sizeof(int))/(getRecordSize(scan->rel->schema)+sizeof(int));
    for (index = count; index < tuple; index++){
        if (rid.slot>=slot){
            rid.page++;
            rid.slot= -1;
        }
        getRecord(scan->rel, rid, record);
        evalExpr(record, scan->rel->schema, ((STable *)scan->mgmtData)->condition, &value);
        if (value->v.boolV){
            break;
        }
        rid.slot++;
    }
    if (index>=tuple){
        free(value);
        return RC_RM_NO_MORE_TUPLES;
    }
    index++;
    ((STable *)scan->mgmtData)->currentScan = index;
    free(value);
    return RC_OK;
}


extern RC closeScan(RM_ScanHandle *scan) {
    free(scan->mgmtData);
    scan->mgmtData = NULL;
    return RC_OK;
}

// dealing with schemas
extern int getRecordSize(Schema *schema) {
    int size = 0;
    int i;
    for(i=0; i<schema->numAttr; i++){
        switch (schema->dataTypes[i]){
            case DT_STRING:
                size = size + (schema->typeLength[i])* sizeof(char);
                break;
            case DT_INT:
                size = size + sizeof(int);
                break;
            case DT_FLOAT:
                size = size + sizeof(float);
                break;
            case DT_BOOL:
                size = size + sizeof(bool);
                break;
            default:
                printf("Data type not supported. \n");
                return RC_RM_UNKOWN_DATATYPE;
                break;
        }
    }
    return size;
}


extern Schema *createSchema(int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys) {
    Schema *s = (Schema *) malloc(sizeof(Schema));     // Allocate memory space to schema
    s->numAttr = numAttr;
    s->attrNames = attrNames;
    s->dataTypes = dataTypes;
    s->typeLength = typeLength;
    s->keySize = keySize;
    s->keyAttrs = keys;
    return s;
}

extern RC freeSchema(Schema *schema) {
    int i;
    free(schema->keyAttrs);
    free(schema->typeLength);
    free(schema->dataTypes);
    for (i = 0;i < schema->numAttr;i++) {
        free(schema->attrNames[i]);
    }
    free(schema->attrNames);
    free(schema);
    return RC_OK;
}

// dealing with records and attribute values
extern RC createRecord(Record **record, Schema *schema) {
    *record = (Record *)malloc(sizeof(Record));
    (*record)->data = (char *)malloc(getRecordSize(schema));
    return RC_OK;
}

extern RC freeRecord(Record *record) {
    free(record->data);
    free(record);
    return RC_OK;
}


extern RC getAttr(Record *record, Schema *schema, int attrNum, Value **value) {
 
    Value *v = (Value *)malloc(sizeof(Value));
    int offSet = 0;
    for(int i=0; i<attrNum; i++){
        switch (schema->dataTypes[i]){
            case DT_INT:
                offSet = offSet + sizeof(int);
                break;
              
            case DT_STRING:
                offSet = offSet + (schema->typeLength[i]) * sizeof(char);
                break;
                
            case DT_FLOAT:
                offSet = offSet + sizeof(float);
                break;
            
            case DT_BOOL:
                offSet = offSet + sizeof(bool);
                break;
            default:
                printf("Data type not supported. \n");
                return RC_RM_UNKOWN_DATATYPE;
                break;
        }
    }
    switch (schema->dataTypes[attrNum]){
        case DT_INT:
            memcpy(&(v->v.intV),record->data+offSet,sizeof(int));
            break;
        case DT_STRING:
            v->v.stringV=(char *)calloc(schema->typeLength[attrNum],sizeof(char));
            memcpy(v->v.stringV,((char *)record->data)+offSet,schema->typeLength[attrNum]*sizeof(char));
            break;
        case DT_FLOAT:
            memcpy(&(v->v.floatV),record->data+offSet,sizeof(float));
            break;
        case DT_BOOL:
            memcpy(&(v->v.boolV),record->data+offSet,sizeof(bool));
            break;
        default:
            printf("Data type not supported. \n");
            return RC_RM_UNKOWN_DATATYPE;
            break;
    }
    v->dt=schema->dataTypes[attrNum];
    *value=v;
    return RC_OK;
}

extern RC setAttr(Record *record, Schema *schema, int attrNum, Value *value) {
    int offSet = 0;
    for(int i=0; i<attrNum; i++){
        switch (schema->dataTypes[i]){
            case DT_INT:
                offSet = offSet + sizeof(int);
                break;
              
            case DT_STRING:
                offSet = offSet + (schema->typeLength[i]) * sizeof(char);
                break;
                
            case DT_FLOAT:
                offSet = offSet + sizeof(float);
                break;
            
            case DT_BOOL:
                offSet = offSet + sizeof(bool);
                break;
            default:
                printf("Data type not supported. \n");
                return RC_RM_UNKOWN_DATATYPE;
                break;
                
        }
    }
    switch(value->dt) {
        case DT_INT:
            memcpy(record->data+offSet,&(value->v.intV),sizeof(int));
            break;
        
        case DT_STRING:
            strcpy(record->data+offSet,value->v.stringV);
            break;
        
        case DT_FLOAT:
            memcpy(record->data+offSet,&(value->v.floatV),sizeof(float));
            break;
        
        case DT_BOOL:
            memcpy(record->data+offSet,&(value->v.boolV),sizeof(bool));
            break;
        default:
            printf("Data type not supported. \n");
            return RC_RM_UNKOWN_DATATYPE;
            break;
        
    }

    return RC_OK;
}

