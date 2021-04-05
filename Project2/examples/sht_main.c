
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
//includes
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
	//input records
    int testRecordsNumber = 1500;
    //initializing bf layer
    BF_Init();
    //file infos
    char* fileName="primary.index";
	char attrType='i';
	char* attrName="id";
	int attrLength=4;
	int buckets=10;
	char* sfileName="secondary.index";
	char* sAttrName="name";
	int sAttrLength=15;
	int sBuckets=10;

	//Creating and opening the HT file .
    printf("Creating Primary Index\n");
	int createNumCode=HT_CreateIndex(fileName,attrType,attrName,attrLength,buckets);
	if (createNumCode==0)
	{
		printf(" SUCCESS\n");
	}
	else
	{
		printf(" FAILURE\n");
	}
	HT_info* hi;
	printf(" Open Primary  Index\n");
	hi=HT_OpenIndex(fileName);
	if(hi!=NULL && hi->attrType==attrType && strcmp(hi->attrName,attrName)==0)
	{
		printf(": SUCCESS\n");
	}
	else
	{
		printf(" FAILURE\n");
	}
    //inserting the Records to the HT file. 
	printf(" Insert Records to the Primary Index\n");
	for (int i=0;i<testRecordsNumber;i++)
	{
		Record record;
		record.id=i;
		sprintf(record.name,"name_%d",i);
		sprintf(record.surname,"surname_%d",i);
		sprintf(record.city,"city_%d",i);
		HT_InsertEntry(*hi,record);
	}
    //Getting the records in the HT file.
	printf("Getting the Entries of the Primary index \n");
	int err=HT_GetAllEntries(*hi,NULL);
	
    //Now lets create our Secondary index (SHT file)
	//which is ''syncronized'' with our Primary index(HT file)
	//In the creation of the SHT file the entries of the primary file are
	//being insetred to the sht file with extra info based on their position to HT file. 
	printf("\n Creating + Openning  Secondary Index.\n");
	int createErrorCode=SHT_CreateSecondaryIndex(sfileName,sAttrName,sAttrLength,sBuckets,fileName);
	if (createErrorCode<0)
	{
		printf(" FAILURE\n");
		return -1;
	}
	SHT_info* shi=SHT_OpenSecondaryIndex(sfileName);
	if(shi!=NULL)
	{
		printf("\nSUCCESS\n");
	}
	else
	{
		printf("FAILURE\n");
	}
	//now lets what we have.
	//lets get the Entries of the Secondary index.
	for (int i=0;i<testRecordsNumber;i++)
	{
		Record record;
		record.id=i;
		sprintf(record.name,"name_%d",i);
		sprintf(record.surname,"surname_%d",i);
		sprintf(record.city,"city_%d",i);
		SecondaryRecord sr;

		strcpy(sr.str, record.name);
		int totalBlocks = SHT_SecondaryGetAllEntries(*shi,*hi,(void*) &record.name);
		//if(totalBlocks > 1)
		//	printf("Total blocks readed untill entry: %d\n\n", totalBlocks);

	}
    //Now lets close the SHT file.
	SHT_CloseSecondaryIndex(shi);

	return 0;




}
//----------------------------------------------------------------------------------------