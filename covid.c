#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include "covidTrace.h"

//! CHANGE THESE VALUES IN THE HEADER FILE AS WELL
#define DAYS 30
#define MAX_BT_VALUE 10000
#define SPEEDUP 100
//! CHANGE THESE VALUES IN THE HEADER FILE AS WELL

int main()
{
    srand(time(NULL));
    // dymamicaly allocate memory for all possible addresses (assume MAX_BT_VALUE for this example)
    addressDB *BTreceiver = (addressDB *)malloc(sizeof(addressDB));
    if (BTreceiver == NULL)
        return(-1);
    for(int i=0; i<MAX_BT_VALUE; i++){
        BTreceiver->addresses[i].timestamp.tv_sec = 0;
        BTreceiver->close_contacts[i].timestamp.tv_sec = 0;
    }
    clock_gettime(CLOCK_MONOTONIC, &program_starting_time);
    int status;
    // create threads to handle all simultaneous jobs (devices/forgetContacts/forgetAddresses/uploadContacts)
    pthread_t threads[4];
    status = pthread_create(&threads[0], NULL, &BTDeviceFound, (void *)BTreceiver);
    if (status){
        printf("Thread[0] error.\n"); exit(-1);
    }
    status = pthread_create(&threads[1], NULL, &forgetContacts, (void *)BTreceiver->close_contacts);
    if (status){
        printf("Thread[1] error.\n"); exit(-1);
    }
    status = pthread_create(&threads[2], NULL, &forgetAddresses, (void *)BTreceiver);
    if (status){
        printf("Thread[2] error.\n"); exit(-1);
    }
    status = pthread_create(&threads[3], NULL, &informServer, (void *)BTreceiver->close_contacts);
    if (status){
        printf("Thread[3] error.\n"); exit(-1);
    }
    // check if desired running time has elapsed; if so, cancel threads and return
    struct timespec current_time;
    while(1){
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        if(elapsed_time(program_starting_time, current_time) >= DAYS*(double)86400/(double)SPEEDUP){ 
			for(int i=0; i<4; i++){
				pthread_cancel(threads[i]);
				pthread_join(threads[i], NULL);
			}
            fclose(delays_ptr);
            fclose(server_ptr);
            break;
        }
        // sleep for 5 hours, then recheck if time has passed
        msleep(5*3600/(double)SPEEDUP*1000); 
    }
    pthread_exit(NULL);
    return 0;
}
