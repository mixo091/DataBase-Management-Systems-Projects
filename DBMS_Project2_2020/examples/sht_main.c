/**
 * sdi1700076 MANTZOURANIS GEWRGIOS
 * sdi1700091 MICHOPOULOS MIXALIS
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "../include/hash_file.h"
#include "../include/SecondaryHT.h"

// #define FILE_NAME "data"
// #define HP_FILE "hT"

#define CALL_OR_DIE(call)     \
  {                           \
    HT_ErrorCode code = call; \
    if (code != HT_OK) {      \
      printf("Error\n");      \
      exit(code);             \
    }                         \
  }                           \
                            

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
	char* sAttrName="surname";
	int sAttrLength=15;
	int sBuckets=10;

	//Creating and opening the HT file .
    printf("Creating Primary Index\n");
	int createNumCode = HT_CreateIndex(fileName,attrType,attrName,attrLength,buckets);
	if(createNumCode != 0) {
		perror("Error: HT_CreateIndex!\n");
		exit(-1);
	}
	
	HT_info* hinfo;
	printf(" Open Primary  Index\n");
	hinfo = HT_OpenIndex(fileName);
	if(hinfo != NULL && hinfo->attrType == attrType && strcmp(hinfo->attrName,attrName) == 0) {
		;
	} else {
		perror("Error: HT_OpenIndex\n");
		exit(-1);
	}

	FILE *fptr;
	// open file for reading
    if((fptr = fopen("../askisi2_2020/records5K.txt","r")) == NULL) {
        perror("fopen");
        exit(-2);
    }

	char line[128];
	Record r;
	while(fscanf(fptr, "%s", line) == 1) {
            
    // set record struct
        set_record(&r, line);

        int error_insert = HT_InsertEntry(*hinfo, r);

        if(error_insert < 0) {
            perror("HT_InsertEntry");
            exit(error_insert);
        }
    }
    printf("\nRecords inserted.");
	printf("Getting the Entries of the Primary index \n");
	int err=HT_GetAllEntries(*hinfo,NULL);

	//Now lets create our Secondary index (SHT file)
	//which is ''syncronized'' with our Primary index(HT file)
	//In the creation of the SHT file the entries of the primary file are
	//being insetred to the sht file with extra info based on their position to HT file. 
	printf("\n Creating + Openning  Secondary Index.\n");
	int createErrorCode=SHT_CreateSecondaryIndex(sfileName,sAttrName,sAttrLength,sBuckets,fileName);
	if (createErrorCode<0) {
		perror("Error: SHT_CReateSecondaryIndex\n");
		exit(-1);
	}

	SHT_info* shi = SHT_OpenSecondaryIndex(sfileName);
	if(shi == NULL) {
		perror("Error: SHT_OpenSecondaryIndex\n");
		exit(-1);
	}

	// let's do some printing
	for (int i = 0; i < testRecordsNumber *2;i++)
	{
		Record record;
		record.id=i;
		sprintf(record.name,"name_%d",i);
		sprintf(record.surname,"surname_%d",i);
		sprintf(record.city,"city_%d",i);
		SecondaryRecord sr;

		// only the first "testRecordsNumber" must be printed
		int totalBlocks = SHT_SecondaryGetAllEntries(*shi,*hinfo,(void*) &record.surname);
		if(totalBlocks > 1)
			printf("Total blocks readed untill entry: %d\n\n", totalBlocks);
	}

    //Now lets close the SHT file and HT file
	SHT_CloseSecondaryIndex(shi);
	HT_CloseFile(hinfo);

	printf("\nPrint Statistics:	\n\n");
	HashStatistics(fileName);
	printf("\n\n");
	HashStatistics(sfileName);

	fclose(fptr);
	
	return 0;
}
//----------------------------------------------------------------------------------------