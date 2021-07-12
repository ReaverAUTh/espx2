//! CHANGE THESE VALUES IN THE C FILE AS WELL
#define DAYS 30
#define MAX_BT_VALUE 10000
#define SPEEDUP 100
//! CHANGE THESE VALUES IN THE C FILE AS WELL

FILE *delays_ptr;
FILE *server_ptr;
pthread_mutex_t lock;
struct timespec program_starting_time;

typedef struct uint48 {
	uint64_t val : 48 ;
} uint48;

typedef struct macaddress {
    uint48 address;
    struct timespec timestamp;
} macaddress;

typedef struct addressDB {
    macaddress addresses[MAX_BT_VALUE];
    macaddress close_contacts[MAX_BT_VALUE];
} addressDB;

//Functions----------------------------------------------------
bool testCOVID();
int msleep(long msec);
void *BTDeviceFound(void *BTreceiver);
void *forgetAddresses(void *BTreceiver);
void *informServer(void *close_contacts);
void *forgetContacts(void *close_contacts);
void uploadContacts(macaddress *close_contacts);
double elapsed_time(struct timespec start,struct timespec end);
//-------------------------------------------------------------

macaddress BTnearMe(){
    macaddress nearMe;
    int address_number;
    // generate a random number between 0 and MAX_BT_VALUE for the MAC address
    nearMe.address.val = (uint64_t)(rand() % MAX_BT_VALUE); 
    struct timespec timestamp;
    clock_gettime(CLOCK_MONOTONIC, &timestamp);
    nearMe.timestamp = timestamp;
    return nearMe;
}

void *BTDeviceFound(void *BTreceiver){
    delays_ptr = fopen("delays.bin","wb");
    double buffer[1];
    double search_offset = 0; // the expected time offset between calls 
    while(1){
        macaddress nearMe = BTnearMe();
        buffer[0] = elapsed_time(program_starting_time, nearMe.timestamp) - search_offset;

        fwrite(buffer, sizeof(buffer), 1, delays_ptr);
        search_offset += (double)10/(double)SPEEDUP;

        double time_diff = elapsed_time(((addressDB*)BTreceiver)->addresses[nearMe.address.val].timestamp, nearMe.timestamp);
        
        // if between 4 mins and 20 mins, add to closeContacts
        if ((double)4*(double)60/(double)SPEEDUP <= time_diff && time_diff <= (double)20*(double)60/(double)SPEEDUP){
            pthread_mutex_lock(&lock);
            ((addressDB*)BTreceiver)->close_contacts[nearMe.address.val] = nearMe;
            pthread_mutex_unlock(&lock);
        }
        pthread_mutex_lock(&lock);
        ((addressDB*)BTreceiver)->addresses[nearMe.address.val] = nearMe;
        pthread_mutex_unlock(&lock);

        // sleep for 10 seconds
        msleep(10/(double)SPEEDUP*1000); 
    }
}

bool testCOVID(){
    // 3% chance to be positive - can be altered
    if (rand() % 100 < 3) 
        return true;
    else
        return false;
}

void *forgetContacts(void *close_contacts){
    // sleep for 13 days and check every day to see if there are close contacts to be removed
    msleep(13*86400/(double)SPEEDUP*1000);
    struct timespec current_time;
    while(1){
        clock_gettime( CLOCK_MONOTONIC, &current_time);
        for(int i=0; i< MAX_BT_VALUE; i++){
            pthread_mutex_lock(&lock);
            if (elapsed_time(((macaddress *)close_contacts)[i].timestamp, current_time) > (double)14*(double)86400/(double)SPEEDUP){
                ((macaddress *)close_contacts)[i].timestamp.tv_sec = 0;
            }
            pthread_mutex_unlock(&lock);
        }

        msleep(86400/(double)SPEEDUP*1000); 
    }
}

void *forgetAddresses(void *BTreceiver){
    // sleep for 19 minutes and then every minute check if an address needs to be forgotten
    msleep(19*60/(double)SPEEDUP*1000);
    struct timespec current_time;
    while(1){
        clock_gettime( CLOCK_MONOTONIC, &current_time);
        for(int i=0; i< MAX_BT_VALUE; i++){
            pthread_mutex_lock(&lock);
            // 20 minutes have passed
            if (((addressDB*)BTreceiver)->close_contacts[i].timestamp.tv_sec == 0 && 
                elapsed_time(((addressDB*)BTreceiver)->addresses[i].timestamp, current_time) > (double)20*(double)60/(double)SPEEDUP){

                ((addressDB*)BTreceiver)->addresses[i].timestamp.tv_sec = 0; 
            }
            pthread_mutex_unlock(&lock);
        }

        msleep(60/(double)SPEEDUP*1000); 
    }
}

void uploadContacts(macaddress *close_contacts){
    int uploaded_contacts = 0;
    uint64_t buffer[1];
    bool close_contacts_existed = false;
    for(int i=0; i<MAX_BT_VALUE; i++){
        // if this contact is a close contact, write it to file
        if(close_contacts[i].timestamp.tv_sec != 0){ 
            buffer[0] = (close_contacts[i].address.val + (uint64_t)1);
            fwrite(buffer, sizeof(buffer), 1, server_ptr);
            close_contacts_existed = true;
            uploaded_contacts++;
        }
    }
    // add a zero after all contacts were uploaded to separate tests
    if(close_contacts_existed){
        buffer[0] = 0;
        fwrite(buffer, sizeof(buffer), 1, server_ptr);
        printf("Uploaded %d contacts after positive test.\n", uploaded_contacts);
    }
}

void *informServer(void *close_contacts){
    server_ptr = fopen("close_contacts_COVID.bin","wb");
    while(1){
        // if the test was positive, upload contacts
        if(testCOVID()){
            pthread_mutex_lock(&lock);
            uploadContacts(((macaddress *)close_contacts));
            pthread_mutex_unlock(&lock);
        }
        msleep(4*3600/(double)SPEEDUP*1000); 
    }
}

//! msleep makes thread sleep for the time given in milliseconds
//  DEPENDENCIES: StackOverflow - https://stackoverflow.com/a/1157217
int msleep(long msec){
    struct timespec ts;
    int res;
    if (msec < 0){
        errno = EINVAL;
        return -1;
    }
    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;
    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

// fuction to calculate elapsed time between two timespecs (same as the one in the PDS deliverables)
double elapsed_time(struct timespec start,struct timespec end){
        struct timespec temp;
        if ((end.tv_nsec - start.tv_nsec) < 0)
		{
            temp.tv_sec = end.tv_sec - start.tv_sec - 1;
            temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
        }
        else{
            temp.tv_sec = end.tv_sec - start.tv_sec;
            temp.tv_nsec = end.tv_nsec - start.tv_nsec;
        }
        return (double)temp.tv_sec +(double)((double)temp.tv_nsec/(double)1000000000);
}