#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/SecondaryHT.h"
#include "../lib/BF_lib/BF.h"

void initSHT(SHT_info *info, 
            int fd, 
            char *attrName, 
            int attrLen, 
            int buckets,
            char *fileName)
{
    info->fileDesc = fd;
    strcpy(info->attrName, attrName);
    info->attrLength = attrLen;
    strcpy(info->fileName, fileName);
    info->numBuckets = buckets;
}

int SHT_CreateSecondaryIndex(char *sfileName, 
                            char *attrName, 
                            int attrLength,
                            int buckets,
                            char *fileName ) 
{
    // int bucketArr[buckets];
    int fdSHT = 0;
    int blockCnt = 0;
    void *block = NULL;

    // for(int i = 0; i < buckets; i++) bucketArr[i] = -1;

    if(BF_CreateFile(sfileName) != BFE_OK) 
        return HT_ERROR;
     else{
        if((fdSHT = BF_OpenFile(sfileName)) < 0) {
		    BF_PrintError("Error opening file!\n");
		    return -1;
	    }
    }

    // Allocate first block in SHT
    if(BF_AllocateBlock(fdSHT) < 0 ) {
		BF_PrintError("Error allocating new Block!\n");
		return -1;
	}
	
	if((blockCnt = BF_GetBlockCounter(fdSHT) - 1) < 0) {
		BF_PrintError("Error finding block: No block has been allocated!\n");
		return -1;
	}
	
	if(BF_ReadBlock(fdSHT, blockCnt, &block) < 0) {
		BF_PrintError("Cannot read block!\n");
		return -1;
	}
 
    SHT_info sInfo;

    // Init the first block with SHT_info data
    initSHT(&sInfo, fdSHT, attrName, attrLength, buckets, sfileName);

    // write data to disk
    memcpy(block, (char*) &sInfo, sizeof(SHT_info));
    if(BF_WriteBlock(fdSHT, 0) < 0){
		BF_PrintError("Error writing block back");
	}

    /**
     * Init the secondary index in second block
     */ 

    if(BF_AllocateBlock(fdSHT)<0){
        BF_PrintError("Error Allocating the second block \n");
		exit(EXIT_FAILURE);
    }

    blockCnt = BF_GetBlockCounter(fdSHT);
    if(BF_ReadBlock(fdSHT, 1, &block)<0){
        BF_PrintError("Error reading the block");
        exit(EXIT_FAILURE);
    } 

    int *bucketArr = malloc(sizeof(int) * buckets);
    for(int i = 0; i < buckets; i++)
        bucketArr[i] = -1;

    memcpy(block,bucketArr,sizeof(int) * buckets);

    if (BF_WriteBlock(fdSHT, 1) < 0){
        BF_PrintError("Error writing block back");
        exit(EXIT_FAILURE);
            
    }

    /**
     * We use the fileName to insert some records from
     * primary hash table. For every bucket in primary Hash Table
     * we create a SecondaryRecord based on the list o blocks.
     */  

    int fd;
    if((fd = BF_OpenFile(fileName)) < 0) {
		BF_PrintError("Error opening file!\n");
		return -1;
	}

    HT_info hInfo; 
    int fdHT = hInfo.fileDesc;

    
    if(BF_ReadBlock(fd, 0, &block)<0){
        BF_PrintError("Error reading the block");
        exit(EXIT_FAILURE);
    } 

    memcpy(&hInfo, block, sizeof(HT_info));
    // hInfo = (HT_info *) block;

    int totalBuckets=hInfo.numBuckets;
    // printf("fileDesc of ht = %d\n", hInfo.fileDesc);

    if(BF_ReadBlock(fd, 1, &block)<0){
        BF_PrintError("Error reading the block");
        exit(EXIT_FAILURE);
    } 

    // get the primary index
    int *staticHash = (int*) block;
    int currentBlock = -1;
    int nextBlock = 0;
    int blockRecords = 0;
    Record r; //  we will set this record from ht in order to insert it to SHT
    SecondaryRecord sRecord;

    void *new_block;

    int *index = malloc(sizeof(int) * totalBuckets);
    for(int i = 0; i < totalBuckets; i++) {
        index[i] = staticHash[i];
        // printf("%d\n", index[i]);
    }

    SecondaryRecord arr[MAX_INSERTIONS];
    for(int i = 0; i < MAX_INSERTIONS; i++) {
        arr[i].blockId = -1;
    }

    int count = 0;
    int totalRecords = 0;
    for(int i = 0; i < totalBuckets; i++) {
        // get current block of i-th bucket
        // currentBlock = index[i];
        // printf("Bucket %d \n", index[i]);

        if((nextBlock = index[i]) == -1) continue;
        while(nextBlock != -1) {
            // printf("fd = %d\n", fd);
            if(BF_ReadBlock(fd, nextBlock, &new_block) < 0) {
                printf("Error: BF_ReadBlock\n");
                return -1;
            }

            currentBlock = nextBlock;

            memcpy(&nextBlock, new_block, sizeof(int));
            memcpy(&blockRecords, new_block + 4, sizeof(int));
            new_block += 8;

            for(int j = 0; j < blockRecords; j++) {
                // sRecord.record = *(Record *) block;
                // r = malloc(sizeof(Record));
                memcpy(&r, new_block + j*sizeof(Record), sizeof(Record));

                // Record *temp = malloc(sizeof(Record));
                // sRecord.blockId = currentBlock;

                // if(strcmp(attrName, "surname") == 0)
                //     strcpy(sRecord.str, r->surname);
                // else if(strcmp(attrName, "name") == 0) 
                //     strcpy(sRecord.str, r->name);
                // else 
                //     strcpy(sRec  ord.str, r->city);
                // printf("to insert %s\n", r.name);

                if(strcmp(attrName, "surname") == 0)
                    strcpy(arr[count].str, r.surname);
                else if(strcmp(attrName, "name") == 0) 
                    strcpy(arr[count].str, r.name);
                else 
                    strcpy(arr[count].str, r.city);

                arr[count++].blockId = currentBlock;

                // count++;
                // printf("to insert name: %s\n", r->name);

                // SHT_SecondaryInsertEntry(sInfo, sRecord);
                // printf("inserting id %d with name %s\n", tempS->record.id, tempS->record.name);

                // move to next Record
                // block += sizeof(Record)
                // memset(sRecord.str,0,sizeof(sRecord.str));

                // free(r);
                // r = NULL;
            }
        }
    } 

    for(int i = 0; i < MAX_INSERTIONS; i++) {
        // printf("str = %s\n", arr[i].str);
        if(arr[i].blockId != -1)
            SHT_SecondaryInsertEntry(sInfo, arr[i]);
        else break;
    }
    // printf("Total records inserted = %d\n", totalRecords);

    // printf("fd = %d\n", fdHT);

    if(BF_CloseFile(fdSHT) < 0) {
        printf("Error: BF_CloseFile\n");
        return -1;
    }

    if(BF_CloseFile(fd) < 0) {
        printf("Error: BF_CloseFile\n");
        return -1;
    }

    free(index);

    return 1;
}

SHT_info *SHT_OpenSecondaryIndex(char *sfileName) {
    int fd;
    void *block;
    SHT_info *shtInfo = malloc(sizeof(SHT_info));
    SHT_info temp;

    if((fd = BF_OpenFile(sfileName)) < 0) {
        BF_PrintError("Error Opening File!\n");
        return NULL;
    }

    // let's get the 1st block, which is block 0
    if(BF_ReadBlock(fd, 0, &block) < 0) {
        BF_PrintError("Error while getting block 0!\n");
        return NULL;
    }

    memcpy(&temp, block, sizeof(SHT_info));
    // printf("file = %s\n", temp.fileName);
    // printf("Attr name = %s\n", temp.attrName);

    if((strcmp(temp.fileName, "secondary.index")) != 0) {
        perror("Not a SHT file!\n");
        return NULL;
    }

    shtInfo->attrLength = temp.attrLength;
    strcpy(shtInfo->attrName, temp.attrName);
    shtInfo->fileDesc = temp.fileDesc;
    shtInfo->numBuckets = temp.numBuckets;
    strcpy(shtInfo->fileName, temp.fileName);


    return shtInfo;
}

int SHT_CloseSecondaryIndex(SHT_info* header_info) {

	int fd;
	void *block;

	if(BF_CloseFile(header_info->fileDesc) < 0) {
		BF_PrintError("Error closing file!\n");
		return -1;
	}

	free(header_info);
	header_info = NULL;

	return 0;
}

// helper function for secondary block init
void insertRecordToSHT_block(SHT_block **sht_block, SecondaryRecord r) {
    (*sht_block)->SCrecords[(*sht_block)->nRecords++] = r;
}


int SHT_SecondaryInsertEntry(SHT_info header_info, SecondaryRecord sr) {
    unsigned int hashValue;
    void *block;
    int blockCount;
    int currentBlock;

    SecondaryRecord *r  = malloc(sizeof(SecondaryRecord));
    memcpy(r, &sr, sizeof(SecondaryRecord));

    // printf("Hashing according to: %s\n", header_info.attrName);
    char *attrName = malloc(sizeof(char) * strlen(header_info.attrName));
    strcpy(attrName, header_info.attrName);

    // printf("to insert %s\n", sr.str);

    if(strcmp(header_info.attrName,"name") == 0) {
        hashValue = Hashing(r->str, header_info.numBuckets);
    } else if(strcmp(header_info.attrName,"surname") == 0) {
        hashValue = Hashing(r->str, header_info.numBuckets);
    } else if(strcmp(header_info.attrName,"city") == 0) {
        hashValue = Hashing(r->str, header_info.numBuckets);
    } else {
        ;
        
    }

    // let's get the secondary's hash table
    if(BF_ReadBlock(header_info.fileDesc,1, &block) < 0) {
        printf("Error: BR_ReadBlock\n");
        return -1;
    }
    // printf("To insert record: %s with blockId %d\n", r.record.name, r.blockId);

    int index[header_info.numBuckets]; 
    int hashTable[header_info.numBuckets];

    memcpy(&index, block, sizeof(int) * header_info.numBuckets);

    for(int i = 0; i < header_info.numBuckets; i++)
        hashTable[i] = index[i];

    
    // printf("edw %d\n", hashTable[hashValue]);
    if(hashTable[hashValue] == -1) {
        // empty bucket
        // create new block
        if(BF_AllocateBlock(header_info.fileDesc) < 0) {
            printf("Error: BF_AllocateBlock\n") ;
            return -1;
        }

        // get block number
        if((blockCount = BF_GetBlockCounter(header_info.fileDesc)) < 0) {
            printf("Error: BF_GetBlockCounter\n");
            return -1;
        }  

        // update hash table
        hashTable[hashValue] = blockCount - 1;
        memcpy(block, hashTable, sizeof(int) * header_info.numBuckets);
        // printf("inserted at %d with block num %d\n", hashValue, hashTable[hashValue]);
        
        if(BF_WriteBlock(header_info.fileDesc, 1) < 0) {
            printf("Error: BF_WriteBlock\n");
            exit(EXIT_FAILURE);
        }

        if(BF_ReadBlock(header_info.fileDesc, blockCount-1, &block)<0){
            printf("Error: BF_ReadBlock\n");
            exit(EXIT_FAILURE);
        } 

        SHT_block *sht_block = malloc(sizeof( SHT_block));
        sht_block->nextBlock = -1;
        sht_block->nRecords = 0;
        for(int i = 0; i < RECORDS_IN_BUCKETS; i++)
        {
            sht_block->SCrecords[i].blockId = 0;
            // sht_block->SCrecords[i].record = 0;
            memset(&sht_block->SCrecords[i], 0, sizeof(Record));
        }

        insertRecordToSHT_block(&sht_block, *r);

        memcpy(block, sht_block, sizeof( SHT_block));

        // write data to block
        if (BF_WriteBlock(header_info.fileDesc, blockCount-1) < 0){
		    printf("Error: BF_WriteBlock\n");
            exit(EXIT_FAILURE);
	    }

        free(sht_block);
        sht_block = NULL;

        return blockCount - 1;
    } else {
        // current bucket has some records we need to traverse 
        // the list of blocks if a lists exists
        
        // let's get the 1st block of the list
        if(BF_ReadBlock(header_info.fileDesc, hashTable[hashValue], &block)<0){
            printf("Error: BF_ReadBlock\n");
            exit(EXIT_FAILURE);
        } 
        // printf("bucket = %d\n",hashTable[hashValue] );

        SHT_block *sht_block = malloc(sizeof( SHT_block));
        memcpy(sht_block, block, sizeof(SHT_block));

       int currentBlock;

        if(sht_block->nRecords == RECORDS_IN_BUCKETS) {
            // sht_block->nRecords = 0;
            // traverse the list of blocks
            int nextBlock = sht_block->nextBlock;
            currentBlock = hashTable[hashValue];

            while(nextBlock != -1) {

                // currentBlock = nextBlock;

                if(BF_ReadBlock(header_info.fileDesc, nextBlock, &block) < 0) {
					BF_PrintError("Error getting block!\n");
					return -1;
				}
                free(sht_block);
                sht_block = NULL;

                sht_block = malloc(sizeof( SHT_block));
                memcpy(sht_block, block, sizeof(SHT_block));

                // if there is space
                if(sht_block->nRecords < RECORDS_IN_BUCKETS) {
                    insertRecordToSHT_block(&sht_block, *r);
                    // write block to memory
                    memcpy(block, sht_block, sizeof(SHT_block));

                    if(BF_WriteBlock(header_info.fileDesc, nextBlock) < 0) {
                        printf("Error writing block!\n");
                        return -1;
                    }	

                    free(sht_block);
                    sht_block = NULL;
                    return 0;
                } else {
                    // currentBlock = nextBlock;
                    // go to next Block
                    // nextBlock = sht_block->nextBlock;
                    if(sht_block->nextBlock == -1)
                        break;
                    else 
                        nextBlock = sht_block->nextBlock;
                }
            }

            if(nextBlock != -1) currentBlock = nextBlock;

            // printf("next block = %d\n", sht_block->nextBlock);
             if(BF_AllocateBlock(header_info.fileDesc) < 0) {
                printf("Error: BF_AllocateBlock\n") ;
                return -1;
            }

            // get block number
            if((blockCount = BF_GetBlockCounter(header_info.fileDesc)) < 0) {
                printf("Error: BF_GetBlockCounter\n");
                return -1;
            }  

            // save next block's value
            int newBlock = blockCount - 1;

            // insert Record to the new block
            if(BF_ReadBlock(header_info.fileDesc, newBlock, &block) < 0) {
				BF_PrintError("Error getting block!\n");
				return -1;
			}

            SHT_block *toInsert = malloc(sizeof(SHT_block));
            toInsert->nRecords = 0;
            insertRecordToSHT_block(&toInsert, *r);
            toInsert->nextBlock = -1; // make new block the last block

            memcpy(block, toInsert, sizeof(SHT_block));
            if(BF_WriteBlock(header_info.fileDesc, newBlock) < 0) {
                printf("Error: BF_WriteBlock\n");
                exit(-1);
            }

            free(sht_block);
            sht_block = NULL;
            // make a link with new block
            // sht_block->nextBlock = newBlock;
            if(BF_ReadBlock(header_info.fileDesc, currentBlock, &block) < 0) {
				BF_PrintError("Error getting block!\n");
				return -1;
			}
            // printf("nextBlock = %d\n", sht_block->nextBlock);
            sht_block = malloc(sizeof( SHT_block));
            memcpy(sht_block, block, sizeof(SHT_block));

            sht_block->nextBlock = newBlock;

            memcpy(block, sht_block, sizeof(SHT_block));
            if(BF_WriteBlock(header_info.fileDesc, currentBlock) < 0) {
                printf("Error: BF_WriteBlock\n");
                exit(-1);
            }

            free(toInsert);
            free(sht_block);

            return 0;
            // sht_block->nextBlock = blockCount - 1;
        } else {
            insertRecordToSHT_block(&sht_block, *r);

            memcpy(block, sht_block, sizeof(SHT_block));
            if(BF_WriteBlock(header_info.fileDesc, hashTable[hashValue] ) < 0 ) {
                printf("Error: BF_WriteBlock\n");
                return -1;
            } 

            free(sht_block);
            sht_block = NULL;
        }
    }
}

int SHT_SecondaryGetAllEntries(SHT_info header_info_sht, HT_info header_info_ht, void *value) {
    void *block;
    int hashTable[header_info_sht.numBuckets];
    SHT_block *sht_block;

    // get the hash table
    if(BF_ReadBlock(header_info_sht.fileDesc, 1, &block) < 0) {
        printf("Error: BF_ReadBlock\n");
        return -1;
    }
    // get the hash table
    memcpy(hashTable, block, sizeof(int) * header_info_sht.numBuckets);

    // print everything
    if(value == NULL) {

    // print all surnames
        int totalRecords = 0;
        int k = 0;
        for(int i = 0; i < header_info_sht.numBuckets; i++) {

            // printf("\n  Bucket-%d:  \n", hashTable[i]);
            if(hashTable[i] != -1) {

                if(BF_ReadBlock(header_info_sht.fileDesc, hashTable[i], &block) < 0) {
                    printf("Error: BF_ReadBlock\n");
                    return -1;
                }

                int nextBlock = 0;
                SecondaryRecord r;

                while (nextBlock != -1) {
                // get block's data
                    sht_block = malloc( sizeof(SHT_block) );
                    memcpy(sht_block, block, sizeof(SHT_block));
                    // printf( "records are %d\n", sht_block->nRecords);
                        
                    for(int j = 0; j < sht_block->nRecords; j++) {
                        totalRecords++;

                        memcpy(&r, &sht_block->SCrecords[j] , sizeof(SecondaryRecord));
                            // printf("%s\n", r.str);    
                            // now we should read from primary hash table
                        if(BF_ReadBlock(header_info_ht.fileDesc, r.blockId, &block) < 0) {
                            printf("Error: BF_ReadBlock\n");
                            exit(-1);
                        }
                        
                    } printf("\n");

                        // go to next block
                    nextBlock = sht_block->nextBlock;

                    if(nextBlock == -1) ;
                    else {
                        // read next block 
                        if(BF_ReadBlock(header_info_sht.fileDesc, nextBlock, &block) < 0 ) {
                            perror("BF_ReadBlock");
                            return -1;
                        }

                        free(sht_block);
                        sht_block = NULL;                    // printf("next block = %d\n", nextBlock);
                    }
                }
            }
        }
        printf("Total records = %d\n", totalRecords);
        return totalRecords;

    } else {
        char *key = (char *) value;
        unsigned int hashValue = Hashing(key, header_info_sht.numBuckets);
        printf("Searching for name \"%s\" at bucket = %d\n", key, hashTable[hashValue]);

        if(hashTable[hashValue] != -1) {

            int currentBucket = hashTable[hashValue];
            int nextBlock = 0;
            int countBlocks = 1;

            if(strcmp(header_info_sht.attrName, "name") == 0) {

                if(BF_ReadBlock(header_info_sht.fileDesc, currentBucket, &block) < 0) {
                    printf("Error: BF_ReadBlock\n");
                    return -1;
                }

                int countBlocks = 1;
                SecondaryRecord sr;
                while (nextBlock != -1) {

                    countBlocks++;
                // get block's data
                    sht_block = malloc( sizeof(SHT_block) );
                    memcpy(sht_block, block, sizeof(SHT_block));
                    // printf( "records are %d\n", sht_block->nRecords);
                        
                    for(int j = 0; j < sht_block->nRecords; j++) {
                        // memcpy(&sr, &sht_block->SCrecords[j] , sizeof(SecondaryRecord));
                         
                        sr = sht_block->SCrecords[j];
                        // printf("%s\n", sr.str);   

                        if(strcmp(sr.str, key) == 0) {
                            // now we should read from primary hash table
                            if(BF_ReadBlock(header_info_ht.fileDesc, sr.blockId, &block) < 0) {
                                printf("Error: BF_ReadBlock\n");
                                exit(-1);
                            }

                            // save number of records for this block
                            int blockRecords;
                            memcpy(&blockRecords, block + 4, sizeof(int));
                            block += 8; // move to 1st record
                            Record rec;
                                // int count = 0;
                            int k;
                            int found = 0;
                            for( k=0; k < blockRecords; k++) {
                                memcpy(&rec, block , sizeof(Record));
                                    
                                if(strcmp(sr.str, rec.name) == 0) {
                                    printf("Entry with name \"%s\" found!\n", sr.str);
                                    found = 1;
                                    
                                    return countBlocks;
                                } else {
                                    ;
                                }

                                block += sizeof(Record);
                            }
                            // if entry doesn't exist in primary hash table   
                            if(k == blockRecords - 1 && found == 0) {
                                printf("Entry %s doesn't exist!\n", sr.str);
                                return -1;
                            }
                                
                            printf("\n");
                        } 
                    }
                    // go to next block
                    nextBlock = sht_block->nextBlock;

                    if(nextBlock == -1) ;
                    else {
                    // read next block 
                        if(BF_ReadBlock(header_info_sht.fileDesc, nextBlock, &block) < 0 ) {
                            perror("BF_ReadBlock");
                            return -1;
                        }
                        // printf("nextBlock = %d\n", nextBlock);

                        free(sht_block);
                        sht_block = NULL;                    
                    }
                }
            }
        }   
        return -1;
    }
}
