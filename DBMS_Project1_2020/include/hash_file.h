#ifndef HASH_FILE_H
#define HASH_FILE_H

#define MAX_OPEN_FILES 20
#define RECORDS_IN_BUCKETS 8
#define BUCKETS_NUM 13


typedef enum HT_ErrorCode {
  HT_OK,
  HT_ERROR
} HT_ErrorCode;

typedef struct Record {
	int id;
	char name[15];
	char surname[20];
	char city[20];
} Record;


typedef struct info{
int fileDesc; /* αναγνωριστικός αριθμός ανοίγματος αρχείου από το επίπεδο block */ 
char attrType; /* ο τύπος του πεδίου που είναι κλειδί για το συγκεκριμένο αρχείο, 'c' ή'i' */
char attrName[8]; /* το όνομα του πεδίου που είναι κλειδί για το συγκεκριμένο αρχείο */ 
int attrLength; /* το μέγεθος του πεδίου που είναι κλειδί για το συγκεκριμένο αρχείο */
long int numBuckets; /* το πλήθος των “κάδων” του αρχείου κατακερματισμού */
 }HT_info;


void set_attrName(HT_info **hp_info, char *name);
// int Hashing(int id, int buckets);
unsigned short Hashing(char *str, int buckets);

/*
 * Η συνάρτηση HP_CreateFile χρησιμοποιείται για τη δημιουργία και
 * κατάλληλη αρχικοποίηση ενός άδειου αρχείου σωρού με όνομα fileName.
 * Σε περίπτωση που εκτελεστεί επιτυχώς, επιστρέφεται HP_OK, ενώ σε
 * διαφορετική περίπτωση κάποιος κωδικός λάθους.
 */
HT_ErrorCode HT_CreateIndex(  char *fileName,
					char attrType,
					char *attrName,
					int attrLength, 
                    int buckets
	);

HT_ErrorCode HT_Init();

/*
 * Η συνάρτηση HP_OpenFile ανοίγει το αρχείο με όνομα filename και διαβάζει
 * από το πρώτο μπλοκ την πληροφορία που αφορά το αρχείο σωρού. Επιστρέφει
 * στην μεταβλητή fileDesc τον αναγνωριστικό αριθμό ανοίγματος αρχείου, όπως
 * αυτός επιστράφηκε από το επίπεδο διαχείρισης μπλοκ. Σε περίπτωση που
 * εκτελεστεί επιτυχώς, επιστρέφεται HP_OK, ενώ σε διαφορετική περίπτωση
 * κάποιος κωδικός λάθους. Αν το αρχείο που ανοίχτηκε δεν πρόκειται για αρχείο
 * σωρού, τότε αυτό θεωρείται επίσης περίπτωση σφάλματος.
 */
HT_info* HT_OpenIndex( const char *fileName	);

/*
 * Η συνάρτηση HT_CloseFile κλείνει το αρχείο που προσδιορίζεται από τον
 * αναγνωριστικό αριθμό ανοίγματος fileDesc. Σε περίπτωση που εκτελεστεί
 * επιτυχώς, επιστρέφεται HT_OK, ενώ σε διαφορετική περίπτωση κάποιος
 * κωδικός λάθους.
 */
int HT_CloseFile(HT_info* header_info);

/*
 * Η συνάρτηση HT_InsertEntry χρησιμοποιείται για την εισαγωγή μίας
 * εγγραφής στο αρχείο σωρού. Ο αναγνωριστικός αριθμός ανοίγματος του
 * αρχείου δίνεται με την fileDesc ενώ η εγγραφή προςεισαγωγή προσδιορίζεται
 * από τη δομή record. Η εγγραφή προστίθεται στο τέλος του αρχείου, μετά την
 * τρέχουσα τελευταία εγγραφή. Σε περίπτωση που εκτελεστεί επιτυχώς,
 * επιστρέφεται HT_OK, ενώ σε διαφορετική περίπτωση κάποιος κωδικός λάθους.
 */
int HT_InsertEntry(	HT_info header_info, Record record);

/*
 * Η συνάρτηση HT_PrintAllEntries χρησιμοποιείται για την εκτύπωση όλων των
 * εγγραφών που υπάρχουν στο αρχείο σωρού. Το fileDesc είναι ο αναγνωριστικός
 * αριθμός ανοίγματος του αρχείου, όπως αυτός έχει επιστραφεί από το επίπεδο
 * διαχείρισης μπλοκ. Σε περίπτωση που εκτελεστεί επιτυχώς, επιστρέφεται HT_OK,
 * ενώ σε διαφορετική περίπτωση κάποιος κωδικός λάθους.
 */
int HT_GetAllEntries(	HT_info header_info,void *value );

int HashStatistics(char *filename);
	
int HT_DeleteEntry(HT_info info, void *id);
	

#endif // HEAP_FILE_H
