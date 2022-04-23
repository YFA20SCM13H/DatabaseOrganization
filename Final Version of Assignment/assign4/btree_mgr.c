#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "buffer_mgr.h"
#include "dberror.h"
#include "record_mgr.h"
#include "storage_mgr.h"
#include "tables.h"
#include "btree_mgr.h"

#define INDEX_INFO_PAGE 0
/*

typedef struct BTreeHandle {
  DataType keyType;
  char *idxId;
  void *mgmtData;
} BTreeHandle;

typedef struct BT_ScanHandle {
  BTreeHandle *tree;
  void *mgmtData;
} BT_ScanHandle;


*/
//changed Node and Btree position so that line 36, 55 error solves.
typedef struct Node {
  struct Node * parentnode;
  struct Node * nextnode; // included parentnode and nextnode
  void ** pointerkey;
  Value ** key; //included pointerkey and key for print
  bool isLeaf;
  int numKeys;
  int nextleaf;
  int * keys;
	
} Node;


typedef struct BTree {	
  DataType keyType;	
  struct Value value;
  struct RID rid;  
  Node * BTreeRoot;	//root WJC changed int to a Node
  Node * pointer;  //WJC included
  int LastPageUsed;		
  int numEntries;	//count keys
  int numNodes;		//count nodes
  int maxKeysPerNode;
  BM_BufferPool *bm;
  BM_PageHandle *pHandle;
} BTree;


typedef struct NodeData {
	RID rid;
} NodeData;

typedef struct Scan_data {
	int indexkey;
	int keysum;
	int maxchild;
	Node * nodeinfo;
} Scan_data;
//included scan data struct for easier scan

BTree **Tree;

int numKeys;
int scanNext;

// init and shutdown index manager
RC initIndexManager (void *mgmtData) {
	
	initStorageManager();
	Tree = (BTree **)malloc(10*sizeof(BTree* ));

	return RC_OK;
	
};

RC shutdownIndexManager () {
//   this free() moved to deleteKey()
//    int i;
//    for(i = 0;i<numKeys;i++){
//        free(Tree[numKeys]);
//    }
    free(Tree);
    return RC_OK;
};

// create, destroy, open, and close an btree index
RC createBtree (char *idxId, DataType keyType, int n) {
	
	BTreeHandle *bh =(BTreeHandle*)malloc(sizeof(BTreeHandle));
	bh->idxId = idxId;
	bh->keyType = keyType;
	numKeys = 0;
	scanNext = 0;
 	free(bh);
	return RC_OK;
};
RC openBtree (BTreeHandle **tree, char *idxId) {

	*tree = (BTreeHandle *)malloc(sizeof(BTreeHandle));
    	(*tree)->idxId=idxId;
	return RC_OK;
   
};
RC closeBtree (BTreeHandle *tree) {
    tree = NULL;
    free((void *)tree);
    free(tree);
    return RC_OK;
};
RC deleteBtree (char *idxId) {
    destroyPageFile(idxId);
    return RC_OK;
};

// access information about a b-tree
RC getNumNodes (BTreeHandle *tree, int *result) {
    
    int count = 0;
    int i,j;
    for (i = 1; i < numKeys; i++){
        for(j = i-1; j >= 0; j--){
            if (Tree[i]->rid.page == Tree[j]->rid.page){
                count++;
                break;
            }
        }
    }
    *result = numKeys - count;
    return RC_OK;
};
RC getNumEntries (BTreeHandle *tree, int *result) {
    
    
    *result = numKeys;

    return RC_OK;
};
RC getKeyType (BTreeHandle *tree, DataType *result) {
    int i;

    for(i=0;i<numKeys;i++)
    {
        result[i] = tree->keyType;
    }
    return RC_OK;
};

// index access
/*
 findKey return the RID for the entry with the search key in the b-tree.
 If the key does not exist this function should return RC_IM_KEY_NOT_FOUND
 */
RC findKey (BTreeHandle *tree, Value *key, RID *result) {
    bool find = 0;
    int i;
    //Tree[numKeys]
    switch(key->dt){

        case DT_INT:
            for(i = 0; i < numKeys; i++){
                if (Tree[i]->value.dt == key->dt && Tree[i]->value.v.intV == key->v.intV){
                    find = 1;
                    break;
                }
            }
            break;
        case DT_STRING:
            for(i = 0; i < numKeys; i++){
                if (Tree[i]->value.dt == key->dt && (strcmp(Tree[i]->value.v.stringV, key->v.stringV) == 0)){
                    find = 1;
                    break;
                }
            }
            break;
        case DT_FLOAT:
            for(i = 0; i < numKeys; i++){
                if (Tree[i]->value.dt == key->dt && Tree[i]->value.v.floatV == key->v.floatV){
                    find = 1;
                    break;
                }
            }
            break;
        case DT_BOOL:
            for(i = 0; i < numKeys; i++){
                if (Tree[i]->value.dt == key->dt && Tree[i]->value.v.boolV == key->v.boolV){
                    find = 1;
                    break;
                }
            }
            break;
    }
    if(find == 1){
        result->page = Tree[i]->rid.page;
        result->slot = Tree[i]->rid.slot;
        return RC_OK;
        
    }else{
        return RC_IM_KEY_NOT_FOUND;
    }

};
/*
 insertKey inserts a new key and record pointer pair into the index.
 It should return error code RC_IM_KEY_ALREADY_EXISTS if this key is already stored in the b-tree.
 */
RC insertKey (BTreeHandle *tree, Value *key, RID rid) {
    Tree[numKeys] =(BTree *)malloc(sizeof(BTree));
    bool find = 0;
    int i;
    switch(key->dt){
        case DT_INT:
            for(i = 0; i < numKeys; i++){
                if (Tree[i]->value.dt == key->dt && Tree[i]->value.v.intV == key->v.intV){
                    find = 1;
                    break;
                }
            }
            break;
        case DT_STRING:
            for(i = 0; i < numKeys; i++){
                if (Tree[i]->value.dt == key->dt && (strcmp(Tree[i]->value.v.stringV, key->v.stringV) == 0)){
                    find = 1;
                    break;
                }
            }
            break;
        case DT_FLOAT:
            for(i = 0; i < numKeys; i++){
                if (Tree[i]->value.dt == key->dt && Tree[i]->value.v.floatV == key->v.floatV){
                    find = 1;
                    break;
                }
            }
            break;
        case DT_BOOL:
            for(i = 0; i < numKeys; i++){
                if (Tree[i]->value.dt == key->dt && Tree[i]->value.v.boolV == key->v.boolV){
                    find = 1;
                    break;
                }
            }
            break;
    }
    if(find == 1){
        return RC_IM_KEY_ALREADY_EXISTS;
    }else{
       
        switch(key->dt){
                
            case DT_INT:
                Tree[numKeys]->value.dt = key->dt;
                Tree[numKeys]->value.v.intV= key->v.intV;
                break;
            case DT_STRING:
                Tree[numKeys]->value.dt = key->dt;
                strcpy(Tree[numKeys]->value.v.stringV, key->v.stringV);
                break;
            case DT_FLOAT:
                Tree[numKeys]->value.dt = key->dt;
                Tree[numKeys]->value.v.floatV= key->v.floatV;
                break;
            case DT_BOOL:
                Tree[numKeys]->value.dt = key->dt;
                Tree[numKeys]->value.v.boolV= key->v.boolV;
                break;
        }
        Tree[numKeys]->rid.page = rid.page;
        Tree[numKeys]->rid.slot = rid.slot;
        numKeys++;
        return RC_OK;
    }
};
/*
 deleteKey removes a key (and corresponding record pointer) from the index.
 It should return RC_IM_KEY_NOT_FOUND if the key is not in the index.
 */

RC deleteKey (BTreeHandle *tree, Value *key) {
    int i, k;
    int Next;
    Next = 0;
    i = 0;
    bool Check = 0;
    //See if key exist
    switch (key->dt){
    case DT_INT:
        while ( i < numKeys){
            if (Tree[i]->value.dt == key->dt && Tree[i]->value.v.intV == key->v.intV){
                Check = 1;
                break;
            }
            i++;
        }
        break;

    case DT_STRING:
        while (i < numKeys){
            if (Tree[i]->value.dt == key->dt && (strcmp(Tree[i]->value.v.stringV, key->v.stringV) == 0)){
                Check = 1;
                break;
            }
            i++;
        }
        break;
    case DT_FLOAT:
        while ( i < numKeys){
            if (Tree[i]->value.dt == key->dt && Tree[i]->value.v.floatV == key->v.floatV){
                Check = 1;
                break;
            }
            i++;
        }
        break;
    case DT_BOOL:
        while ( i < numKeys){
            if (Tree[i]->value.dt == key->dt && Tree[i]->value.v.boolV == key->v.boolV){
                Check = 1;
                break;
            }
            i++;
        }
        break;
    }
    if (Check == 1){
        Next = i + 1;
        for (k = i; k < numKeys&&Next < numKeys; k++){
            switch (Tree[Next]->value.dt){
                case DT_INT:
                    Tree[k]->value.dt = Tree[Next]->value.dt;
                    Tree[k]->value.v.intV = Tree[Next]->value.v.intV;
                    break;
                case DT_STRING:
                    Tree[k]->value.dt = Tree[Next]->value.dt;
                    strcpy(Tree[k]->value.v.stringV, Tree[Next]->value.v.stringV);
                    break;
                case DT_FLOAT:
                    Tree[k]->value.dt = Tree[Next]->value.dt;
                    Tree[k]->value.v.floatV = Tree[Next]->value.v.floatV;
                    break;
                case DT_BOOL:
                    Tree[k]->value.dt = Tree[Next]->value.dt;
                    Tree[k]->value.v.boolV = Tree[Next]->value.v.boolV;
                    break;
                }
            Tree[k]->rid.page = Tree[Next]->rid.page;
            Tree[k]->rid.slot = Tree[Next]->rid.slot;
            Next++;
        }
        numKeys--;
        free(Tree[k]);
        return RC_OK;
    }else{
        return RC_IM_KEY_NOT_FOUND;
    }
};

RC openTreeScan (BTreeHandle *tree, BT_ScanHandle **handle) {
//    BTree *Btree = (BTree *) tree->mgmtData;
    
//     Node *Nodeinfo=calloc(1,sizeof(Node));

//     Scan_data *Scan=calloc(1,sizeof(Scan_data));
//     if(Btree->BTreeRoot!=NULL)
//     {
//       while(Nodeinfo->isLeaf)
//         Nodeinfo=Nodeinfo->pointerkey[0];
//         Scan->indexkey=0;
//         Scan->keysum=Nodeinfo->numKeys;
//         Scan->nodeinfo=Nodeinfo;
//         Scan->maxchild=Btree->maxKeysPerNode;
//         (*handle)->mgmtData=Scan;
//     }
//     else{return RC_IM_KEY_NOT_FOUND;};
  
//     return RC_OK; 
	int i;
        for (i = 0; i < numKeys - 1; i++) {
            int index;
            index = i;
            int j;
            j = i + 1;
            while ( j < numKeys ) {
                if (Tree[j]->value.v.intV < Tree[index]->value.v.intV) {
                    index = j;
                }
                j++;
            }
            BTree *tempTree = (BTree *)malloc(sizeof(BTree));
            tempTree = Tree[i];
            Tree[i] = Tree[index];
            Tree[index] = tempTree;
        }
        return RC_OK;
};

RC nextEntry(BT_ScanHandle *handle, RID *result) {
    int x;
    int i;
    i = 0;
    Value Aval;
    RID Arid;

    while (i < numKeys - 1) {
        int y;
        y = i;
        for (x = i + 1; x < numKeys; x++) {
            if (Tree[x]->value.v.intV < Tree[y]->value.v.intV) {
                y = x;
            }
        }
        Aval.dt = Tree[i]->value.dt;
        Aval.v.intV = Tree[i]->value.v.intV;
        Arid.page = Tree[i]->rid.page;
        Arid.slot = Tree[i]->rid.slot;

        Tree[i]->value.dt = Tree[y]->value.dt;
        Tree[i]->value.v.intV = Tree[y]->value.v.intV;
        Tree[i]->rid.page = Tree[y]->rid.page;
        Tree[i]->rid.slot = Tree[y]->rid.slot;

        Tree[y]->value.dt = Aval.dt;
        Tree[y]->value.v.intV = Aval.v.intV;
        Tree[y]->rid.page = Arid.page;
        Tree[y]->rid.slot = Arid.slot;
        i++;
    }
    if (scanNext < numKeys) {
        result->page = Tree[scanNext]->rid.page;
        result->slot = Tree[scanNext]->rid.slot;
        scanNext++;
        return RC_OK;
    }
    else {
        return RC_IM_NO_MORE_ENTRIES;
    }

};

RC closeTreeScan (BT_ScanHandle *handle) {
//    free(handle->mgmtData);
    free(handle);
    return RC_OK;
};



// These functions help in printing the B+ Tree
void que(BTree * Btree, Node * newnode) {
	Node * exnode;
	if (Btree->pointer == NULL) {
		Btree->pointer = newnode;
		Btree->pointer->nextnode = NULL;
	} else {
		exnode = Btree->pointer;
		while (exnode->nextnode != NULL) {
			exnode = exnode->nextnode;
		}
		exnode->nextnode = newnode;
		newnode->nextnode = NULL;
	}
}

Node * deque(BTree * Btree) {
	Node * exnode = Btree->pointer;
	Btree->pointer = Btree->pointer->nextnode;
	exnode->nextnode = NULL;
	return exnode;
}


// Gives the length of the path from any node to the root.
int lengthroot(Node * Root, Node * child) {
	int length = 0;
	Node * exnode = child;
	while (exnode != Root) {
		exnode = exnode->parentnode;
		length++;
	}
	return length;
}


// debug and test functions
char *printTree (BTreeHandle *tree) {
    
  BTree *Btree = (BTree *) tree->mgmtData;
	Node * new = NULL;
  // Check
  int i=0;
  int rank=0;
  int newrank=0;

	if (Btree->BTreeRoot != NULL) {
    Btree->pointer=NULL;
    que(Btree,Btree->BTreeRoot);
    while(Btree->pointer!=NULL){
      new=deque(Btree);
      if(new->parentnode!=NULL && new==new->parentnode->pointerkey[0]){
        newrank=lengthroot(Btree->BTreeRoot,new);
  
        if(newrank!=rank){
          rank=newrank;
          printf("\n" );
        }
      }
      for (i=0; i<new->numKeys; i++){
        switch(Btree->keyType){
          case DT_INT:
            printf("%d",(*new->key[i]).v.intV);
            i++;
            break;
          case DT_STRING:
    				printf("%s ", (*new->key[i]).v.stringV);
    				i++;
    				break;
    			case DT_BOOL:
    				printf("%d ", (*new->key[i]).v.boolV);
    				i++;
    				break;
    			case DT_FLOAT:
    				printf("%.10f ", (*new->key[i]).v.floatV);
    				i++;
    				break;
          }
        printf("(%d - %d) ", ((NodeData *) new->pointerkey[i])->rid.page, ((NodeData *) new->pointerkey[i])->rid.slot);
      }
      if(!new->isLeaf){
        for(i=0; i<=new->numKeys;i++){
          que(Btree,new->pointerkey[i]);
        }
      }
      printf("| " );
  
    }
    printf("\n");
  }
  else{
    printf("Empty.\n" );
    return '\0';
  }
  
  return '\0';
};
