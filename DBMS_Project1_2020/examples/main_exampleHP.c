#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "../include/heap_file.h"

#define FILE_NAME "data"
#define HP_FILE "hp"


void set_record(Record *r, char *str);

int main(int argc, char** argv) {
	
	BF_Init();
	
	int fd;
	char line[128];
	Record r; //  this is our record's struct
	char filename[20];
	
	strcpy(filename, FILE_NAME);

	// read from file
	FILE *fptr;
	if((fptr = fopen("../askisi1_2020/records10K.txt","r")) == NULL) {
		perror("Error! opening file");
		exit(-2);
	}	

	HP_info *hp_data = malloc(sizeof(HP_info) * MAX_OPEN_FILES);
	
	srand(time(NULL));
	for(int i = 0; i < MAX_OPEN_FILES; i++) {
		
		sprintf(filename,"dataHP_%d", i);
		printf("\nCreating file \"%s\"", filename);

		if( HP_CreateFile(filename,'c', 
			HP_FILE, sizeof(HP_FILE)) < 0 )  
		{
			perror("HP_CreateFile");
			exit(-1);
		}
		
		hp_data[i] = *HP_OpenFile(filename);
		if(&hp_data[i] == NULL) {
			perror("HP_OpenFile");
			return -1;
		}
	
		int error_insert = 0;
		// read text untill '\n' occur
		while(fscanf(fptr, "%s", line) == 1) {
		// set record values
			set_record(&r, line);
		
		// insert entry
			error_insert = HP_InsertEntry(hp_data[i], r);
			if(error_insert < 0 ) {
				perror("HP_InsertEntry");
				exit(error_insert);
			} else {
				;
			}		
		}
		printf("\nRecords Inserted.");

		// set attr to "name"
		set_attrName(&hp_data[i], "name");

		int error_GetAllEntries = HP_GetAllEntries(hp_data[i], (void*) "name_500" );
		if(error_GetAllEntries < 0) {
			perror("HP_GetAllEntries");
			exit(error_GetAllEntries);
		} else 
			printf("\nTotal searched blocks \"%d\"",error_GetAllEntries);
	
		int x = 0;
	
		// set an attr to "id"
		set_attrName(&hp_data[i], "id");
		for(int j = 0; j < 10; j++) {
			x = rand() % 5000;
		// delete by "id" 
			int error_Delete = HP_DeleteEntry(hp_data[i], (void*)&x );
			if(error_Delete ==  HP_ERROR ) {
				perror("\nHP_DeleteEntry");
				exit(error_Delete);
			} else if(error_Delete == -2) {
				printf("	Id \"%d\" not found.", x);
			} else if(error_Delete == HP_OK) {
				printf("	Entry with id \"%d\" deleted", x);
			} else ;
				
		}

		x = rand() % 1000;
		// get deleted id
		error_GetAllEntries = HP_GetAllEntries(hp_data[i], (void*) &x );
		if(error_GetAllEntries < 0) {
			perror("\nHP_GetAllEntries");
			exit(error_GetAllEntries);
		} else {
			;
		}

		HP_CloseFile(&hp_data[i]);

		// read again from the start of file
		fseek(fptr, 0, SEEK_SET);	

		printf("\n");
	}	

	fclose(fptr);

	free(hp_data);
	
	
	return 0;
}

// this function, sets the record values
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

