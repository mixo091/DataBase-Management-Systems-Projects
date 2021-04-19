#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "../include/hash_file.h"

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

    int fdArray[MAX_OPEN_FILES];
    BF_Init();    
    CALL_OR_DIE(HT_Init());
    char filename[20]; //  char array of filenames
    char line[128]; // this is stdin buffer
    FILE *fptr;

    HT_info *info = malloc(sizeof(HT_info) * MAX_OPEN_FILES); // this will be our info structure stored in block 0
    Record r; // record struct

    // open file for reading
    if((fptr = fopen("../askisi1_2020/records1K.txt","r")) == NULL) {
        perror("fopen");
        exit(-2);
    }

    srand(time(NULL));
    // create files data_
    for(int i = 0; i < MAX_OPEN_FILES; i++) 
    {
        sprintf(filename, "dataHT_%d", i);
        printf("\nCreating file \"%s\"", filename);

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
        // printf("Give an id you want to search for.");
        
        int id = rand() % 1000;
        // scanf("%d", &id);
        // int error_GetId = HT_GetAllEntries(info[i],(void*)&id);
        int error_GetId = 0; // HT_GetAllEntries(info[i],NULL);

        // if(error_GetId < 0) {
        //     perror("HT_GetAllEntries");
        //     exit(error_GetId);
        // } else {
        //     printf("\nTotal blocks readed \"%d\"",error_GetId);
        // }
        for(int j = 0; j < 80; j++) {
            id = rand() % 1000;
            int error_DeleteEntry = HT_DeleteEntry(info[i], (void *) &id);
            if(error_DeleteEntry < 0) {
                perror("HT_DeleteEntry");
                return -1;
            } else {
                printf("\n  Record with id \"%d\" deleted", id);
            }
        }

        // id = rand() % 500;
        // error_GetId = HT_GetAllEntries(info[i], NULL);
        // if(error_GetId < 0) {
        //     perror("HT_GetAllEntries");
        //     exit(error_GetId);
        // } else {
        //     printf("\n  Total blocks readed \"%d\"",error_GetId);
        // }

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
