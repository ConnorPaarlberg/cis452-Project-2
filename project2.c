#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// shared memory of shared resources
typedef struct{
    int mixer;
    int pantry;
    int refrigerator;
    int bowl;
    int spoon;
    int oven;
} resources;

// shared memory of pantry ingredients
// (assume there is enough for all bakers)
typedef struct{
    char flour;
    char sugar;
    char yeast;
    char baking_soda;
    char salt;
    char cinnamon;
} pantry;

// shared memory of refrigerator ingredients 
// (assume there is enough for all bakers)
typedef struct{
    char eggs;
    char milk;
    char butter;
} refrigerator;


void resource_values(){
    resources shared_kitchen;

    shared_kitchen.mixer = 2;
    shared_kitchen.pantry = 1;
    shared_kitchen.refrigerator = 2;
    shared_kitchen.bowl = 3;
    shared_kitchen.spoon = 5;
    shared_kitchen.oven = 1;
}

void* baker_actions();

void num_bakers(){
    char input[100];
    pthread_t baker;

    printf("Enter the number if bakers: ");
    fgets(input, sizeof(input), stdin);
    int input_number = atoi(input);

    int thread_count = 0;
    while(thread_count != input_number){
        if (pthread_create(&baker, NULL, baker_actions, NULL) != 0) {
            perror("Faild to create baker");
            exit(1);
        }
        thread_count ++;
        printf("Baker %d created\n", thread_count);
    }
}

// TODO: fill out
void* baker_actions(){
    return NULL;
}

// TODO: fill out
void ramsied(){

}


int main() {
    resource_values();

    num_bakers();

    ramsied();

    return 0;
}