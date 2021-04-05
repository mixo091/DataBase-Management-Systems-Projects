#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "../include/hash_file.h"
#include "../include/SecondaryHT.h"

#define FILE_NAME "data"
#define HP_FILE "hT"


#define CALL_OR_DIE(call)     \
  {                           \
    HT_ErrorCode code = call; \
    if (code != HT_OK) {      \
      printf("Error\n");      \
      exit(code);             \
    }                         \
  }                           \
                              

void set_record(Record *r, char *str) {
	
	int count = 0; //
	
	char *token = strtok(str, ",");
	
	while(token) {
		
		if(count == 0)
			r->id = atoi(&token[1]);
		else if(count == 1)
			snprintf(r->name,strlen(token)-1,"%s",&token[1]);
		else if(count == 2)
			snprintf(r->surname,strlen(token)-1,"%s",&token[1]);
		else if(count == 3)
			snprintf(r->city,strlen(token)-2,"%s",&token[1]);
		else {
			return ;
		}
		count++;
		
		token = strtok(NULL, ",");
		
	}
}


int main(int argc, char **argv) {
	char attrType='i';
	char* attrName="id";
	int attrLength=4;
	int buckets= BUCKETS_NUM;
	char* sfileName="secondary.index";
	char* sAttrName="name";
	int sAttrLength=15;
	int sBuckets= BUCKETS_NUM;



    //start testing SHT 
    //Initializing file descriptor array for HT and SHT.
    int fdArray[MAX_OPEN_FILES];
    int fdArraySHT[MAX_OPEN_FILES];
    //initializing BF layer.
    BF_Init();    
    //initializing HT  layer.
    CALL_OR_DIE(HT_Init());
    //initializing SHT layer.
  //CALL_OR_DIE(SHT_Init());
    ///--------------------------------------------------
    char filename[20]; //  char array of filenames
    char line[128]; // this is stdin buffer
    FILE *fptr;
    //initializing the info structs for HT and SHT files .
    // this will be our info structure stored in block 0
    HT_info *info = malloc(sizeof(HT_info) * MAX_OPEN_FILES); 
    SHT_info *SHTinfo = malloc(sizeof(SHT_info) * MAX_OPEN_FILES);
    Record r; // record struct
    // Open the file to read records.
    if((fptr = fopen("../SHT/records1K.txt","r")) == NULL) {
        perror("fopen");
        exit(-2);
    }
    srand(time(NULL));
    //----------------------------------------------------
    //Creating HT files.
    for(int i = 0; i < MAX_OPEN_FILES; i++) 
    {
        sprintf(filename, "dataHT_%d", i);
        printf("\nCreating file \"%s\"", filename);
        //OPEN-INSERT-PRINT(HT).
        CALL_OR_DIE(HT_CreateIndex(filename,'i',"ena",4, BUCKETS_NUM ) );
        // open filename created
        info[i] = *HT_OpenIndex(filename);
        // printf("info_fd = %d\n", info[i].fileDesc);
        // read a line from file and skip '\n'
        while(fscanf(fptr, "%s", line) == 1) { 
        // set record struct
            set_record(&r, line);
            int error_insert = HT_InsertEntry(info[i], r);
            if(error_insert < 0) {
                perror("HT_InsertEntry");
                exit(error_insert);
            }
        }
        printf("\nRecords inserted.");
        int  error_GetId = HT_GetAllEntries(info[i], NULL);
         if(error_GetId < 0) {
            perror("HT_GetAllEntries");
            exit(error_GetId);
         } else {
             printf("\n  Total blocks readed \"%d\"",error_GetId);
         }
          printf("\nCreating file \"%s\"", filename);
           printf("\nCreating file \"%s\"", sfileName);
             printf("\nCreating file \"%s\"", sAttrName);
           printf("\nCreating file \"%d\"", sAttrLength);
              printf("\nCreating file \"%d\"", sBuckets);

  printf("Testing1: Create Secondary/ Open Index\n");
	int createErrorCode=SHT_CreateSecondaryIndex(sfileName,sAttrName,sAttrLength,sBuckets,filename);
	if (createErrorCode<0)
	{
		printf("Testing 1: FAILED\n");
		return -1;
	}
	SHT_info* shi=SHT_OpenSecondaryIndex(sfileName);
	if(shi!=NULL)
	{
		printf("Testing 1: SUCCESS\n");
	}
	else
	{
		printf("Testing 1: FAIL\n");
	}
  //Getting Records!
	printf("Testing 2 : SHT - Get all entries \n");
  int flag=0;
	for (int i=0;i<5000;i++)
	{
		Record record;
		record.id=i;
		sprintf(record.name,"name_%d",i);
		sprintf(record.surname,"surname_%d",i);
		sprintf(record.city,"address_%d",i);
		int err=SHT_SecondaryGetAllEntries(*shi,*info,(void*)record.name);
		if (err<0)
		{  
	    // printf("Testing 2: FAIL\n");
       flag=1;
		}
  }
  if(flag=!1){
    printf("Testing 2: SUCCES\n");
  }

        // close hash file
        if(HT_CloseFile(&info[i]) < 0) {
            perror("HT_CloseFile");
            exit(-1);
        } 
        printf("\n");
        int hash_stats = HashStatistics(filename);

        // move to the start of file
        fseek(fptr, 0, SEEK_SET);

        printf("\n");
    }

    // close fptr
    fclose(fptr);

    // free memory
    free(info);

}


//------------------------------------------------------------------------------------------------------s