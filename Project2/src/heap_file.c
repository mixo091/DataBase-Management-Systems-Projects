#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../lib/BF_lib/BF.h"
#include "../include/heap_file.h"

#define RECORDS_IN_BLOCK (BLOCK_SIZE / sizeof(Record)) /* Number of records in a block */


void set_attrName(HP_info *header_ptr, char *name) {
	memset(header_ptr->attrName, 0, sizeof(header_ptr->attrName));
	strcpy(header_ptr->attrName, name);
	
	if(strcmp(name, "id") == 0) {
		header_ptr->attrType = 'i';
		header_ptr->attrLength = sizeof(int);
	} else {
		header_ptr->attrType = 'c';
		header_ptr->attrLength = sizeof(char);
	}
	
}

int HP_CreateFile(char *filename, char attrType,
				char *attrName, int attrLength) {
				
	
	HP_info info;
	void *block;
 	BF_Init();
	
	if(BF_CreateFile(filename) < 0) {
    	BF_PrintError("\nError creating file");;
    	return HP_ERROR;
    
  }else{ 
  
    int fd = BF_OpenFile(filename);
    
    // check fd
    if(fd < 0 ) {
    	perror("\nHP_CreateFile");
    	return HP_ERROR;
    }
 	

    if( BF_AllocateBlock(fd) < 0 ) {
    	BF_PrintError("\nError allocating block");
    	return HP_ERROR;
    }
    
    int blkCnt = BF_GetBlockCounter(fd);
    //printf("File %d has %d blocks\n", fd, blkCnt);
    
    // prosvasi sto block
    if (BF_ReadBlock(fd, blkCnt - 1, &block) < 0) {
		BF_PrintError("\nError getting block");
		return HP_ERROR;
	}

	// set the values of HP_info
	info.fileDesc = fd;
	info.attrLength = attrLength;
	strcpy(info.attrName, attrName);
	info.attrType = attrType;
	
	// write HP_Info to 1st block so that we can identify
	// that is a hp file with some details
	//strncpy(block, attrName, attrLength);
	memcpy(block, (char*)&info, sizeof(HP_info));
	if (BF_WriteBlock(fd, blkCnt - 1) < 0){
		BF_PrintError("\nError writing block back");
		return HP_ERROR;
	}
  }
  return HP_OK;
}

HP_info* HP_OpenFile( char *fileName){

  
	int fd = BF_OpenFile(fileName);
  	if(fd < 0 ) {
  		BF_PrintError("\nError opening file");
  		return NULL;
  	}
  	
  	void *block;
  
  	int blkCnt = BF_GetBlockCounter(fd);
  	
	if(BF_ReadBlock(fd, 0, &block) < 0) {
  		BF_PrintError("\nError getting block");
  		return NULL;
  	}
  	
	HP_info *hp_data = (HP_info*) block;

  	// check for hp_file
  	if(strcmp( hp_data->attrName, "hp") == 0) {
  		printf("\nWe have a heap file");
  	} else {
  		BF_PrintError("\nNot a heap file");
  		return NULL;
  	}
	
  	return hp_data;
}

int HP_CloseFile(HP_info *header_info) {

	if(BF_CloseFile(header_info->fileDesc)  < 0) {
		BF_PrintError("\nError closing file");
		return HP_ERROR;
	}
  
  return HP_OK;
}



int HP_InsertEntry(HP_info header_info, Record record) {
 

  int fd = header_info.fileDesc; // current file descriptor
  int block_num = 0; // number of blocks
  int data_index = record.id % RECORDS_IN_BLOCK; //Index for the storage of Records
  void* block;
  
	// if it is the first block or id == the first entry
  if(record.id % RECORDS_IN_BLOCK == 0 || record.id == 0){
    //Create new block
    
	if (BF_AllocateBlock(fd) < 0) {
		BF_PrintError("\nError allocating block");
		HP_ERROR;
	}
	
	// get Block counter
	block_num = BF_GetBlockCounter(fd);
	
	// get Block
	if(BF_ReadBlock(fd, block_num - 1, &block) < 0 ) {
		BF_PrintError("\nError getting block\n");
		return HP_ERROR;
	}
	memcpy(block, &record, sizeof(Record));
	if(BF_WriteBlock(fd, block_num - 1) < 0 ) { 
		BF_PrintError("\nError writing block\n");
		return HP_ERROR;
	}
	
	return block_num;

  }else{
   
   // get Block counter
   block_num = BF_GetBlockCounter(fd);
  
   
	// get Block
	if(BF_ReadBlock(fd, block_num - 1, &block) < 0 ) {
		BF_PrintError("\nError getting block\n");
		return HP_ERROR;
	}
	
	//Set record to where data_index * sizeof(Record) points
	memcpy((char *)block + sizeof(Record) * data_index, &record, sizeof(Record));
	if(BF_WriteBlock(fd, block_num - 1) < 0 ) { 
		BF_PrintError("\nError writing block\n");
		return HP_ERROR;
	}
  }
  
   return block_num;
}

int HP_GetAllEntries(HP_info header_info, void *value) {
	
	printf("\nHP_GetAllEntries:");
	int total_blocks = 0;
	int fd = header_info.fileDesc;
	Record r;
	
	total_blocks = BF_GetBlockCounter(fd);
	// printf("\nvalue = %d and attrName = %s",*(int*)value, header_info.attrName);
	
	void *block;
	
	// for every block
	for(int i = 1; i < total_blocks; i++) {

	// get Block i
		if(BF_ReadBlock(fd, i, &block) < 0 ) {
			BF_PrintError("\nError getting block\n");
			return HP_ERROR;
		}
	
		// for every record in a block
			for(int j = 0; j < RECORDS_IN_BLOCK; j++ ) {
				// get current record
				memcpy(&r, (char *)block + (j * sizeof(Record)), sizeof(Record));
				
				if(value == NULL) 
					printf("\n	%d %s %s %s\n",r.id,r.name,r.surname,r.city);
				else {
					if(strcmp(header_info.attrName,"id") == 0){
          				if(*(int*)value == r.id) {
            				printf("\n	%d %s %s %s",r.id,r.name,r.surname,r.city);
            				return i+1;
            			}		
        			}else if(strcmp(header_info.attrName,"name") == 0){
          				if(strcmp(value,r.name) == 0) {
            				printf("\n	%d %s %s %s",r.id,r.name,r.surname,r.city);
            				return i+1;
            			}
        			}else if(strcmp(header_info.attrName,"surname") == 0){
          				if(strcmp(value,r.surname) == 0) {
            				printf("\n	%d %s %s %s",r.id,r.name,r.surname,r.city);
            				return i+1;
            			}
       				}else if(strcmp(header_info.attrName,"city") == 0){
          				if(strcmp(value,r.city) == 0) {
             				printf("\n	%d %s %s %s",r.id,r.name,r.surname,r.city);
							return i+1;
						}             				
        			}else{
          				printf("\n	You gave wrong attrName. Please gine another input.");
          				return HP_ERROR;
        			}	
			}
		}
	}
	printf("\n	Record with attrName \"%s\" not found.", header_info.attrName);
	
	return total_blocks;
}


int HP_DeleteEntry(HP_info header_info, void *value) {
	printf("\nHP_DeleteEntry:");
	int total_blocks = 0;
	int fd = header_info.fileDesc;
	Record r, r1;
	
	memset(&r1,0,sizeof(Record));
	
	total_blocks = BF_GetBlockCounter(fd);
	//printf("We have \"%d\"blocks used.\n", total_blocks);
	//printf("value = %d and attrName = %s\n",*(int*)value, header_info.attrName);
	
	void *block;
	
	int offset = 0;
	// for every block
	for(int i = 1; i < total_blocks; i++) {

	// get Block i
		if(BF_ReadBlock(fd, i, &block) < 0 ) {
			BF_PrintError("\nError getting block\n");
			return HP_ERROR;
		}
		//printf("We are in block \"%d\"\n",i+1);
		
		// for every record in a block
			for(int j = 0; j < RECORDS_IN_BLOCK; j++ ) {
			// get current record
				memcpy(&r, block + (j * sizeof(Record)), sizeof(Record));
				offset = j * sizeof(Record); // buff offset
				
				if(value == NULL) 
					return HP_ERROR;
				else {
				
					if(strcmp(header_info.attrName,"id") == 0){
						if(*(int*)value == r.id) {
            				
            				memcpy(block + offset, &r1, sizeof(Record));
            				if(BF_WriteBlock(fd, i) < 0 ) { 
								BF_PrintError("\nError writing block\n");
								return HP_ERROR;
							}
            				
            				return HP_OK;
            			}		
        			}else if(strcmp(header_info.attrName,"name") == 0){
          				if(strcmp(value,r.name) == 0) {
            				
            				memcpy(block + offset, &r1, sizeof(Record));
            				if(BF_WriteBlock(fd, i) < 0 ) { 
								BF_PrintError("\nError writing block\n");
								return HP_ERROR;
							}
            				return HP_OK;
            			}
        			}else if(strcmp(header_info.attrName,"surname") == 0){
          				if(strcmp(value,r.surname) == 0) {
            				
            				memcpy(block + offset, &r1, sizeof(Record));
            				if(BF_WriteBlock(fd, i) < 0 ) { 
								BF_PrintError("\nError writing block\n");
								return HP_ERROR;
							}
            				return HP_OK;
            			}
       				}else if(strcmp(header_info.attrName,"city") == 0){
          				if(strcmp(value,r.city) == 0) {
             				
             				memcpy(block + offset, &r1, sizeof(Record));
            				if(BF_WriteBlock(fd, i) < 0 ) { 
								BF_PrintError("\nError writing block\n");
								return HP_ERROR;
							}
							return HP_OK;
						}             				
        			}else{
          				printf("\n	You gave wrong attrName. Please gine another input.\n");
          				return HP_ERROR;
        			}	
				}
			}
		}
		return -2;
}

