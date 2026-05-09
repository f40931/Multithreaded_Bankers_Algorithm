#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define NUMBER_OF_CUSTOMERS 5
#define NUMBER_OF_RESOURCES 4
/* the available amount of each resource */
int available[NUMBER_OF_RESOURCES];
/*the maximum demand of each customer */
int maximum[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
/* the amount currently allocated to each customer */
int allocation[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
/* the remaining need of each customer */
int need[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

pthread_mutex_t mutex;

int is_safe(){
    int work[NUMBER_OF_RESOURCES];
    int finish[NUMBER_OF_CUSTOMERS];

    for (int i = 0; i < NUMBER_OF_RESOURCES; i++){
        work[i] = available[i];
    }
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++){
        finish[i] = 0;
    }

    for (int count = 0; count < NUMBER_OF_CUSTOMERS; count++){
        int found = 0;
        for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++){
            if (finish[i]){
                continue;
            }

            int can_finish = 1;
            for (int j = 0; j < NUMBER_OF_RESOURCES; j++){
                if (need[i][j] > work[j]){
                    can_finish = 0;
                    break;
                }
            }

            if (can_finish){
                for (int j = 0; j < NUMBER_OF_RESOURCES; j++){
                    work[j] += allocation[i][j];
                }
                finish[i] = 1;
                found = 1;
            }
        }

        if (!found){
            break;
        }
    }

    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++){
        if (!finish[i]){
            return 0;
        }
    }
    return 1;
}

int request_resources(int customer_num,int request[]){
    pthread_mutex_lock(&mutex);

    for (int i = 0; i < NUMBER_OF_RESOURCES; i++){
        if (request[i] > need[customer_num][i] || request[i] > available[i]){
            pthread_mutex_unlock(&mutex);
            return -1;
        }
    }

    for (int i = 0; i < NUMBER_OF_RESOURCES; i++){
        available[i] -= request[i];
        allocation[customer_num][i] += request[i];
        need[customer_num][i] -= request[i];
    }

    if (is_safe()){
        pthread_mutex_unlock(&mutex);
        return 0;
    }

    for (int i = 0; i < NUMBER_OF_RESOURCES; i++){
        available[i] += request[i];
        allocation[customer_num][i] -= request[i];
        need[customer_num][i] += request[i];
    }

    pthread_mutex_unlock(&mutex);
    return -1;
}

void release_resources(int customer_num,int release[]){
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++){
        available[i] += release[i];
        allocation[customer_num][i] -= release[i];
        need[customer_num][i] += release[i];
    }
    pthread_mutex_unlock(&mutex);
}

void* customer_thread(void* arg){
    int customer_num=*(int*)arg;

    while(1){
        int request[NUMBER_OF_RESOURCES];
        // 1. Create random need to reflect the real world use cases
        pthread_mutex_lock(&mutex);
        for(int i=0; i< NUMBER_OF_RESOURCES;i++){
            if(need[customer_num][i] > 0){
                request[i]=rand() % (need[customer_num][i]+1);
            }else{
                 // need=0
                request[i]=0;
            }
        }
        pthread_mutex_unlock(&mutex);

        // 2. Request resources
        if (request_resources(customer_num,request)==0){
            printf("Customer %d: Request granted.\n",customer_num);

            //3. Simulation
            sleep(rand() %3 +1);

            //4. Release resources
            release_resources(customer_num,request);
            printf("Customer %d: Resource released.\n",customer_num);
        }else{
            sleep(1); // Wait 1 second and retry
        }
    }
    return NULL;
}

void load_max_file(char *filename){
    //TODO
};

int main(int argc, char *argv[]){
    if (argc!= NUMBER_OF_RESOURCES +1){
        printf("Usage: ./banker <res1> <res2> <res3> <res4>\n");
        return -1;
    }

    srand(time(NULL));

    // 1. Decode arg
    for (int i =0; i<NUMBER_OF_RESOURCES;i++){
        available[i]=atoi(argv[i+1]);
    }

    // 2. Matrix initialization
    load_max_file("max_requests.txt");

    //3. Mutex initialization
    pthread_mutex_init(&mutex,NULL);

    //4. Create N threads of customer
    pthread_t threads[NUMBER_OF_CUSTOMERS];
    int customer_ids[NUMBER_OF_CUSTOMERS];

    for (int i = 0; i<NUMBER_OF_CUSTOMERS;i++){
        customer_ids[i]=i;
        pthread_create(&threads[i],NULL, customer_thread, &customer_ids[i]);
    }

    //5. Collects threads
    for (int i=0; i<NUMBER_OF_CUSTOMERS;i++){
        pthread_join(threads[i],NULL);
    }
    pthread_mutex_destroy(&mutex);
    return 0;
}   
