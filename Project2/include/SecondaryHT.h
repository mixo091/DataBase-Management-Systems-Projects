#ifndef SHT_H_
#define SHT_H_  

#include "hash_file.h"

#define MAX_INSERTIONS 10000

/**
 * Secondary's hash table structs and functions
 */
typedef struct {	
	int fileDesc;
	char attrName[20];
	int attrLength;
	long int numBuckets;
	char fileName[100];
} SHT_info;

typedef struct SecondaryRecord{
	// Record record;
	char str[100];
	int blockId;
} SecondaryRecord;

typedef struct SecondaryData {
	SecondaryRecord SCrecords[RECORDS_IN_BUCKETS];
	int nRecords;
	int nextBlock;
} SHT_block;

void insertRecordToSHT_block(SHT_block **sht_block, SecondaryRecord r);

// typedef struct {
// 	int numberOfrecords;
// 	int next_block;
// 	SHT_Record SecRecords[RECORDS_IN_BUCKETS];
// } SHT_Block;

void initSHT(SHT_info *sInfo, int fd, char *attrName, int attrLen, int buckets, char *fileName);

int SHT_CreateSecondaryIndex(char *sfileName, char *attrName, int attrLength, int buckets, char *fileName);
SHT_info* SHT_OpenSecondaryIndex(char *sfileName);
int SHT_CloseSecondaryIndex(SHT_info* header_info);
int SHT_SecondaryInsertEntry(SHT_info header_info, SecondaryRecord record);
int SHT_SecondaryGetAllEntries(SHT_info header_info_sht, HT_info header_info_ht, void *value);


#endif