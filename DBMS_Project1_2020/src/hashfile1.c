#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/hash_file.h"
#include "../lib/BF_lib/BF.h"


HT_ErrorCode HT_Init() {
  //insert code here

  return HT_OK;
}

HT_ErrorCode HT_CreateIndex(char *filename, char attrType, char* attrName, int attrLength, int buckets ) {
  //printf("%d records in buckets\n",RECORDS_IN_BUCKETS);

  int blkCnt;
  int bucketArr[buckets];
  int bucketArr1[buckets];
  int fd, block_num;
  for(int i = 0; i < buckets; i++) bucketArr[i] = 0;

  if(BF_CreateFile(filename) != BFE_OK) return HT_ERROR;
  else{
    fd=BF_OpenFile(filename);
    if(fd<0){
      BF_PrintError("Error creating file");
			exit(EXIT_FAILURE);
    }
  //allocate first block
  if(BF_AllocateBlock(fd)<0){
       BF_PrintError("Error Allocating block");
			exit(EXIT_FAILURE);
  }

  blkCnt = BF_GetBlockCounter(fd);
  

  // printf("number of blocks %d ",blkCnt );
  void* block;

  int j=5;
  HT_info inf; //,ff;
  inf.fileDesc=fd;
  inf.numBuckets=buckets;
  inf.attrLength=attrLength;
  // inf.attrName=attrName;
  strcpy(inf.attrName, attrName);
  inf.attrType=attrType;


  if(BF_ReadBlock(fd, 0, &block)<0){
    BF_PrintError("Error reading the block");
  } 
  // memcpy(block, (char*)&j, sizeof(int));
 
  memcpy(block, (char*)&inf, sizeof(HT_info));
  if (BF_WriteBlock(fd, 0) < 0){
				BF_PrintError("Error writing block back");
		
			}

  //----------------------------------------------------------------------

  //allocate new block (the second block)
  if(BF_AllocateBlock(fd)<0){
       BF_PrintError("Error Allocating the second block \n");
			exit(EXIT_FAILURE);
  }

  //now we have 2 blocks
  blkCnt = BF_GetBlockCounter(fd);
  // printf("Number of blocks alloacated   %d   \n " ,blkCnt);
  //now lets copy the bucket array that we will use later to second block
  if(BF_ReadBlock(fd, blkCnt-1, &block)<0){
    BF_PrintError("Error reading the block");
    exit(EXIT_FAILURE);
  } 

  memcpy(block,&bucketArr,sizeof(bucketArr));

  if (BF_WriteBlock(fd, blkCnt-1) < 0){
		BF_PrintError("Error writing block back");
    exit(EXIT_FAILURE);
		
	}
  // if(BF_ReadBlock(fd, blkCnt-1, &block)<0){
  //   BF_PrintError("Error reading the block");
  //   exit(EXIT_FAILURE);
  // } 
  // memcpy(&bucketArr1,block,sizeof(bucketArr));

  //printf("jirfhiufhiujref  %d \n", bucketArr1[3]);
  return HT_OK;
}
}


HT_info* HT_OpenIndex(const char *fileName){
    int fd; 
    fd=BF_OpenFile(fileName);
    if(fd<0){
      BF_PrintError("Error opening  file");
			exit(EXIT_FAILURE);
    }

    void* block;
    if(BF_ReadBlock(fd, 0, &block)<0){
    BF_PrintError("Error reading the block");
    	exit(EXIT_FAILURE);
    }
  
    HT_info *temp = (HT_info *) block;
    // printf("temp = %d\n",temp->fileDesc);

   return  temp;

  //---------------------------------------------------------------------------------------
  
}

int HT_CloseFile(HT_info* header_info) {
  if(BF_CloseFile(header_info->fileDesc) < 0) {
    perror("BF_CloseFile");
    return HT_ERROR;
  } else 
    return HT_OK;
}



// int Hashing(int id,int buckets){ return id % buckets; }

unsigned short Hashing(char *str, int buckets)
{
    unsigned short hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash % buckets;
}


int  HT_InsertEntry(HT_info inf, Record record) {

  int lastBlock, blockRecords,
      nextBlock, currentBlock,
      currentBucket;


  void *block;
  int blockCounter;

  // i made a change here
  char id[8];
  sprintf(id, "%d", record.id);
  unsigned short hashValue = Hashing(id,inf.numBuckets);
  Record r;

  if(BF_ReadBlock(inf.fileDesc, 0, &block)<0){
    BF_PrintError("Error reading the block");
    exit(EXIT_FAILURE);
  } 
  
  int totalBuckets=inf.numBuckets;
  int fd=inf.fileDesc;


  if(BF_ReadBlock(inf.fileDesc, 1, &block)<0){
    BF_PrintError("Error reading the block");
    exit(EXIT_FAILURE);
  } 

  int* staticHash = (int*) block;
  
  // printf("hash = %d\n", hashValue);

  if(staticHash[hashValue] == 0) {

    // if(hashValue == 3)
    //   printf("Bazw gia to 3\n");

    //printf("edw  \n ");
   //if the bucket has no records in, then create the bucket
     //allocate new block (for bucket x)
     if(BF_AllocateBlock(fd)<0){
          BF_PrintError("Error Allocating  block \n");
			    exit(EXIT_FAILURE);
     }
    blockCounter = BF_GetBlockCounter(fd);
    if(BF_ReadBlock(fd, blockCounter-1, &block)<0){
        BF_PrintError("Error reading the block");
         exit(EXIT_FAILURE);
    } 
  
    //update data of the block 
    lastBlock = -1; blockRecords = 0;
    memcpy(block,&lastBlock,sizeof(int));
    block += 4;
    
    blockRecords++;
    memcpy(block,&blockRecords,sizeof(int));
    block += 4;
    // memcpy(block+ 4,&record,sizeof(record));
    memcpy(block,&record,sizeof(record));

    // memcpy(&r,block + 4,sizeof(record));

    staticHash[hashValue] = blockCounter - 1;  
    if (BF_WriteBlock(fd, blockCounter-1) < 0){
		   BF_PrintError("Error writing block back");
       exit(EXIT_FAILURE);
	  }

  return blockCounter-1;

  }else{

    // if(hashValue == 3)
    //   printf("bazw gia to 3\n");
    // staticHash[ hashValue ] has data in it
    // Get bucket number-staticHash[hashValue]
      currentBucket = staticHash[hashValue];
      currentBlock = currentBucket; 
      if(BF_ReadBlock(fd, currentBlock, &block)<0){
        BF_PrintError("Error reading the block");
         exit(EXIT_FAILURE);
    } 
      memcpy(&nextBlock,block,sizeof(int));
      // Get records of current block and move to register Data
      memcpy(&blockRecords,block + sizeof(int),sizeof(int));


      while(1){ 
        // Traverse all blocks in staticHash[hashValue]-bucket
        
        if(nextBlock == -1){
          //currentBlock = currentBucket; 
          //cause we are already in the last block of the bucket 
          break;
        } else {
          currentBlock = nextBlock;
          if(BF_ReadBlock(fd, currentBlock, &block)<0){
              BF_PrintError("Error reading the block");
              exit(EXIT_FAILURE);
       } 
          memcpy(&nextBlock,block,sizeof(int));
          // Get records of current block and move to register Data
          memcpy(&blockRecords,block+ sizeof(int),sizeof(int));

        }
      }
      //dgefjuyhegujegfuyifgeefguyiegfwie

    if( RECORDS_IN_BUCKETS > blockRecords) {
          // There is space for some records
          if(BF_ReadBlock(fd, currentBlock, &block)<0){
              BF_PrintError("Error reading the block");
              exit(EXIT_FAILURE);
       } 
       // update recordNum
          // blockRecords++;
          // block += 4;
          // memcpy(block, &blockRecords, sizeof(int));
          // block += 4;
        // set record 
          // memcpy(block + (blockRecords-1)* sizeof(Record),&record, sizeof(Record));
          // block += (blockRecords-1) * sizeof(Record);


          memcpy(block + (2 * sizeof(int)) + (blockRecords*sizeof(record)),&record,sizeof(record));
          blockRecords++;
          memcpy(block + sizeof(int),&blockRecords,sizeof(int));
          // data = data + 8;
          // memcpy(data + 2 * sizeof(int) + (blockRecords*sizeof(record)),&record,sizeof(record));

          block= block + 2 * sizeof(int) + ((blockRecords-1)*sizeof(record));

          memcpy(&r,block,sizeof(Record));
          //printf("We are still in block-%d with %d records\n",currentBlock,blockRecords);
          // printf("Record = %d %s %s %s\n",r.id,r.name,r.surname,r.city);
          //printf("next block = %d\n",nextBlock);
          if (BF_WriteBlock(fd, currentBlock) < 0){
		          BF_PrintError("Error writing block back");
                 exit(EXIT_FAILURE);
          }

          return currentBlock;
         
        } else {
          // if there is no space at the lst block of the bucket alloacate new block for the bucket
          // Save previous block number

        //  printf("Create new block-%d after block-%d\n",blockNum,currentBlock);
         // nextBlock = blockNum;//blockNum - 1;
           if(BF_ReadBlock(fd, currentBlock, &block)<0){
              BF_PrintError("Error reading the block");
              exit(EXIT_FAILURE);
            } 

           nextBlock= BF_GetBlockCounter(fd);

           memcpy(block,&nextBlock,sizeof(int));
           

          if (BF_WriteBlock(fd, currentBlock) < 0){
		          BF_PrintError("Error writing block back");
                 exit(EXIT_FAILURE);	}


        if(BF_AllocateBlock(fd)<0){
          BF_PrintError("Error Allocating  block \n");
			    exit(EXIT_FAILURE);
         }
          
          if( nextBlock==BF_GetBlockCounter(fd)-1){
            ;//printf("udfhuifheuifhuihfuifhewuihfewui okokokokokokoko \n " );
          }
        
          blockCounter=BF_GetBlockCounter(fd);

          if(BF_ReadBlock(fd, blockCounter-1, &block)<0){
              BF_PrintError("Error reading the block");
              exit(EXIT_FAILURE);
          } 

          lastBlock = -1; blockRecords = 0;
          memcpy(block,&lastBlock,sizeof(int));
          block = block + 4;
          memcpy(block + 4,&record,sizeof(record));
          memcpy(&r,block + 4,sizeof(record));

          blockRecords++;
          memcpy(block,&blockRecords,sizeof(int));

        if (BF_WriteBlock(fd, blockCounter-1) < 0){
		          BF_PrintError("Error writing block back");
                 exit(EXIT_FAILURE);
        }
          // printf("block counter = %d\n", blockCounter-1);
          // printf("New block is  %d\n",nextBlock);
          // printf("Record = %d %s %s %s\n",r.id,r.name,r.surname,r.city);
          // printf("Last overflow block = %d in bucket %d\n\n",nextBlock,staticHash[hashValue]);
          // printf("%d %s\n", inf.fileDesc, inf.attrName);
        
          return (blockCounter-1);
        }


  }

return HT_OK;}

int HT_GetAllEntries(HT_info info, void *id) {
  printf("\nHT_GetAllEntries: ");
  int lastBlock, blockRecords;
  int nextBlock;
  int currentBucket;
  int recsinBlock = 0;
  int total = 0;


  void *block;
  int blockCounter = 0;
  int fd = info.fileDesc;

  if(BF_ReadBlock(fd, 0, &block)<0){
    BF_PrintError("Error reading the block");
    exit(EXIT_FAILURE);
  } 
  
  int totalBuckets = info.numBuckets; // number of buckets
  int totalBlocks = BF_GetBlockCounter(fd); // number of total blocks allocated

  // reading from 1st block the array of buckets
  if(BF_ReadBlock(fd, 1, &block)<0){
    BF_PrintError("Error reading the block");
    exit(EXIT_FAILURE);
  } 

  int *staticHash = (int*) block;

  int *index = malloc(sizeof(int) * BUCKETS_NUM);
  for(int i = 0; i < BUCKETS_NUM; i++)
    index[i] = staticHash[i];
   
  Record r;

  int currentBlock = 0;
  int totalRecs = 0;

  if(id == NULL){
    // for every bucket
    for(int i = 0; i < BUCKETS_NUM; i++) {
    // for(int i = 2; i < totalBlocks; i++) {
      printf("\n  Bucket-%d: \n",i);

      if(BF_ReadBlock(fd, index[i], &block) < 0 ) {
        perror("BF_ReadBlock");
        return -1;
      }

      int nextBlock = 0;

      while (nextBlock != -1)
      {
        memcpy(&nextBlock, block, sizeof(int));
        block += 4;

        // get records
        memcpy(&recsinBlock, block, sizeof(int));
        block += 4;

        for(int j = 0; j < recsinBlock; j++) {
        // total++;
          memcpy(&r, block + j*sizeof(Record), sizeof(Record));
          printf("\n    %d,\"%s\",\"%s\",\"%s\"\n",
            r.id, r.name, r.surname, r.city);  
        } printf("\n");

        totalRecs += recsinBlock;

        if(nextBlock == -1) ;
        else {
          // currentBlock = nextBlock;
          if(BF_ReadBlock(fd, nextBlock, &block) < 0 ) {
            perror("BF_ReadBlock");
            return -1;
          }
          // printf("next block = %d\n", nextBlock);
        }
    }
      // printf("total records are %d\n", totalRecs);
    }
    free(index);

    return totalBlocks;
  }else{ 
    // hashing the id value to totalBuckets

    // i made a change here too
    char key[8];
    sprintf(key, "%d", *(int*) id);
    unsigned short int hash = Hashing(key, totalBuckets);
    
    // get 1st Block in bucket hashed
    int currentBlock = staticHash[hash];

    // reach the block 
    if(BF_ReadBlock(fd, currentBlock, &block) < 0) {
      perror("BF_ReadBlock");
      return -1;
    }
    // this is the nextBlock after currentBlock
    int nextBlock = 0;
    memcpy(&nextBlock, block, sizeof(int));
    block += sizeof(int);

    // get the number of records in current block
    memcpy(&recsinBlock, block, sizeof(int));
    block += sizeof(int);

    int notFound = 1;
    int countBlocks = 1; // counter of Blocks needed

    while(notFound) {
      // check for every record in currentBlock
    
      for(int i = 0; i < recsinBlock; i++) {
          memcpy(&r, block, sizeof(Record));
          
          if(*(int*)id == r.id) {
            printf("\n  %d,\"%s\",\"%s\",\"%s\"",
                r.id,r.name,r.surname,r.city);
            // end loop, record found
            return countBlocks;
          }
        // moveto next record
          block += sizeof(Record);
          countBlocks++;
      }
      if(nextBlock == -1 ) 
        break;
      else {
            // traverse the 'list' of blocks
            currentBlock = nextBlock;

             // reach the block 
            if(BF_ReadBlock(fd, currentBlock, &block) < 0) {
              perror("BF_ReadBlock");
              return -1;
            } 
            memcpy(&nextBlock,block ,sizeof(int));
            memcpy(&recsinBlock,block + sizeof(int),sizeof(int));
            block += 8;

            countBlocks++;
          }      
    }      
      // if not found
      if(notFound) {
        printf("Entry \"%d\" doesn't exist.\n", *(int*)id);
      } 
  } 
   
  return HT_ERROR;
}


int HT_DeleteEntry(HT_info info, void *id) {
  printf("\nHT_DeleteEntry: ");
 //First lets get the information from the file
  void *block;
  int fd = info.fileDesc;

  if(BF_ReadBlock(fd, 0, &block)<0){
    BF_PrintError("\nError reading the block");
    exit(EXIT_FAILURE);
  } 
  int totalBuckets = info.numBuckets; // number of buckets
  int totalBlocks = BF_GetBlockCounter(fd); // number of total blocks allocated
  // reading from 1st block the array of buckets(Hash table)
  if(BF_ReadBlock(fd, 1, &block)<0){
    BF_PrintError("\nError reading the block");
    exit(EXIT_FAILURE);
  } 
  int *staticHash = (int*) block;
  int *index = malloc(sizeof(int) * BUCKETS_NUM);
  for(int i = 0; i <BUCKETS_NUM; i++){
    index[i] = staticHash[i];
    // printf("index %d \n" ,index[i]);
  }
//-------------------------------------------------------------
  //if id == NULL.There is nothing to find.
  if(id==NULL){return -1;}
  char key[8];
  sprintf(key, "%d", *(int*) id);
  //hash the key to get the bucket
  unsigned short int hash = Hashing(key, totalBuckets);
    //now lets get the 1st block of the bucket.
    int currentBlock = index[hash];
    // printf("\n  The bucket that the record to be deleted is %d", hash);
    // printf("\nThe fisrt block of the bucket is %d" ,currentBlock);
    if(BF_ReadBlock(fd, currentBlock, &block) < 0) {
      perror("BF_ReadBlock");
      return -1;
    }
  //now we are in the first block of the bucket.
  // this is the nextBlock after currentBlock.
  int nextBlock = 0;
  memcpy(&nextBlock, block, sizeof(int));
  block += sizeof(int);
  int counter=0;
  // get the number of records in the currentBlock.
    memcpy(&counter, block, sizeof(int));
    block += sizeof(int);

   
while(1){

  if(nextBlock == -1){
    //there is no next block so we are in the block we need
    break;
  }else{//else pass the blocks until you find the one with no next block,it will be the last allocated in this bucket
       //  printf("we are going to the next block of this bucket the block with block_num: %d\n", (*nextBlock));
    //we are not int the last block nextBlock=!-1,so keep traversing.
    currentBlock = nextBlock;
    if(BF_ReadBlock(fd, currentBlock, &block) < 0) {
      perror("\nBF_ReadBlock");
      return -1;
    }
    //get info of the currentBlock.
      memcpy(&nextBlock, block, sizeof(int));
        block += 4;
      memcpy(&counter, block, sizeof(int));
        block += 4;
    //if we are not at the last block but the next block is empty then the last record is at the CurrentBlock,so break.
    if(nextBlock!=-1) {
      void* tempBlock;
      if(BF_ReadBlock(fd, nextBlock, &tempBlock) < 0) {
      perror("\nBF_ReadBlock");
      return -1;
    }
    int tempCount = 0;
    memcpy(&tempCount,tempBlock+4,sizeof(int));
      if(tempCount == 0) {
        break;
      }
    }
  }
}

//lets see if we foubd the block of the last 
//record in the bucket.
int blockOfRecord = currentBlock;
counter=0;
int records;
// printf("\nThe block of the last record in the bucket is %d",blockOfRecord);
if(BF_ReadBlock(fd, blockOfRecord, &block) < 0) {
      perror("BF_ReadBlock");
      return -1;
    }

Record r;
Record r2;
//The position of the last record in the block is recordInBlock-1.
memcpy(&counter,block+4,sizeof(int));
int pos = counter - 1;
memcpy(&r,block + 8 + (pos * sizeof(Record)),sizeof(Record));
records = counter;
// printf("Records in block %d are %d .\n", blockOfRecord, records);
// printf("\n  The id of the last record is %d" ,r.id);
//-------------------------------------------------------------------
if(counter > 0) {
// records are the records in the last block
// Decrease Records in block
  records--;
  if( *(int*)id == r.id) {
  // printf("we are in the special case \n");
  //fi The wanted record is the last record in our block so we just decrease the number of records at the block
    counter = records;
    memcpy(block+4,&counter,sizeof(int));
    if (BF_WriteBlock(fd, currentBlock) < 0){
		          BF_PrintError("\nError writing block back");
                 exit(EXIT_FAILURE);
        }
    return HT_OK;
  }
}
//----------------------------------------------------------------------------------------
//now that we have the info of the last record lets find the record that 
//we want to be deleted.
// printf("Start traversing again from block number %d .\n", index[hash]);
if(BF_ReadBlock(fd, index[hash], &block) < 0) {
      perror("\nBF_ReadBlock");
      return -1;
    }
currentBlock= index[hash];
memcpy(&nextBlock,block,sizeof(int));
memcpy(&counter,block+ sizeof(int),sizeof(int));
// Go straight to the records
block = block + (2*sizeof(int));
// When found is 1 then our id is found
// So stop searching
int found = 0; 
//Start traversing.
while(found != 1) {

  for(int j = 0; j < counter; j++) {

      memcpy(&r2,block  + (j*sizeof(Record)),sizeof(Record));

      if(*(int*)id == r2.id) {
        //if we fin\d the record to be delete\d.
        //lets get the last record\\ in our han\ds an\d put it 
        //int the position of the record that we want to delete
        // printf("here\n");
        void *tempBlock;
        if(BF_ReadBlock(fd, blockOfRecord, &tempBlock) < 0) {
          perror("\nBF_ReadBlock");
      return -1;
       }
      // printf("block of record %d  \n",blockOfRecord);
        memcpy(tempBlock+4,&records,sizeof(int));
            if (BF_WriteBlock(fd, blockOfRecord) < 0){
		          BF_PrintError("\nError writing block back");
                 exit(EXIT_FAILURE);
        }
        found = 1;
        printf("\n  The record to be deleted is " );
        printf("%d,\"%s\",\"%s\",\"%s\"",
            r2.id, r2.name, r2.surname, r2.city);
        printf("\n  The record we overwrite " );
        printf("  %d,\"%s\",\"%s\",\"%s\"",
           r.id, r.name, r.surname, r.city);
        
      // printf("pos is %d \n",j);
      memcpy(block+ (j*sizeof(Record)),&r,sizeof(Record)); 
      if (BF_WriteBlock(fd, currentBlock) < 0){
		          BF_PrintError("\nError writing block back");
                 exit(EXIT_FAILURE);
        }
      }
  }
  if(nextBlock == -1 || found == 1) {
      break;
    }else {
    
    // nextBlock is now our current block
      currentBlock = nextBlock;
      if(BF_ReadBlock(fd, currentBlock, &block) < 0) {
          perror("\nBF_ReadBlock");
      return -1;
       }
      memcpy(&nextBlock,block,sizeof(int));
      memcpy(&counter,block + sizeof(int),sizeof(int));
      // Go straight to the record
      block = block + 8;
    }
  } if(found == 0) printf("\n No record with id %d found.",*(int*)id);
  return HT_OK;
}



int HashStatistics(char *filename) 
{
  printf("\nHashStatistics: ");
  int fd;  // fd of filename
  void *block;

  if(fd = BF_OpenFile(filename) < 0) {
    perror("BF_OpenFile");
    return -1;
  }

  // int totalBlocks = BF_GetBlockCounter(fd);
  
  // printing number of blocks
  // printf("\n  File \"%s\" has %d blocks.", filename, totalBlocks);

  // get the min,avg,max number of records per buckets
  if(BF_ReadBlock(fd, 1, &block) < 0) {
    perror("BF_ReadBlock");
    return -1;
  }

  int *bucketArr = (int*) block;

  int totalBlocksUsed = 2; // first 2 blocks are usefull all the time
  int currentBlock = 0; // this will be our bucket
  int nextBlock = 0; // var that has the value of nextBlock in block list 
  int recordsNum = 0; // number of records 
  int overflowBuckets = 0; // counting overflow buckets
  int blockRecords[BUCKETS_NUM];
  int blocksPerBuckets[BUCKETS_NUM];
  int overflowBlocks[BUCKETS_NUM];
  
  for(int i = 0; i < BUCKETS_NUM; i++) {
    blockRecords[i] = 0;
    blocksPerBuckets[i] = 0;
    overflowBlocks[i] = 0;
  }

  // this is a temp index array of buckets
  int staticHash[BUCKETS_NUM];

  for(int i = 0; i < BUCKETS_NUM; i++)
    staticHash[i] = bucketArr[i];

  for(int i = 0; i < BUCKETS_NUM; i++) {

    // get bucket i
    currentBlock = staticHash[i];

    if(BF_ReadBlock(fd, currentBlock, &block) < 0) {
      perror("BF_ReadBlock");
      return -1;
    }

    nextBlock = 0; recordsNum = 1;
    while(nextBlock != -1 && recordsNum != 0){

    // increase total blocks
      totalBlocksUsed++;

    // get next block
      memcpy(&nextBlock, block, sizeof(int));
      block += 4;

    // get the records of current block
      memcpy(&recordsNum, block, sizeof(int));
      block += 4;

    // increase counter of record for currentBlock
      blockRecords[i] += recordsNum;
      blocksPerBuckets[i]++;

      if(nextBlock == -1) ;
      else {
        if(BF_ReadBlock(fd, nextBlock, &block) < 0) {
          perror("BF_ReadBlock");
            return -1;
        }
        if(overflowBlocks[i]++ == 0)
          overflowBuckets++;

      } 
      // printf("\nnextBlock = %d, records are %d\n", nextBlock, recordsNum); 
      // totalBlocks++; 
    }

    // printf("\n  Bucket-%d stats:\n    %d records", 
    //         i,blockRecords[i]);
    // printf("\")
    // printf("\n");
  }

  int maxRecords = 0, minRecords = 0, sumRecords = 0;
  maxRecords = minRecords = blockRecords[0];
  for(int i = 0; i < BUCKETS_NUM; i++) {
    if(blockRecords[i] > maxRecords) 
      maxRecords = blockRecords[i];

    if(blockRecords[i] < minRecords)
      minRecords = blockRecords[i];

    sumRecords += blockRecords[i];
  }

  int avgRecords = sumRecords / BUCKETS_NUM;

  int sumBlocks = 0;
  for(int i = 0; i < BUCKETS_NUM; i++)
    sumBlocks += blocksPerBuckets[i];

  int totalOverflowBlocks = 0;
  for(int i = 0; i < BUCKETS_NUM; i++)
    totalOverflowBlocks += overflowBlocks[i]++;
  
  printf("\n  File \"%s\" has %d blocks in use", filename, totalBlocksUsed);
  printf("\n  AVG records : %d, MAX records: %d,  MIN records: %d",
    avgRecords, maxRecords, minRecords);
  printf("\n  Average number of blocks : \"%d\"", (sumBlocks / BUCKETS_NUM) );
  printf("\n  Buckets with overflow blocks \"%d\"\n  Total overflow blocks \"%d\"", 
    overflowBuckets, totalOverflowBlocks - BUCKETS_NUM);

  BF_CloseFile(fd);

  return 1;
  
}










