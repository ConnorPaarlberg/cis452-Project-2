#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>

#define PANTRY_INGREDIENTS 6
#define REFRIGERATOR_INGREDIENTS 3

void* baker_actions();

 int mixer_id, pantry_id, refrigerator_id[2], bowl_id, spoon_id, oven_id;

struct sembuf lock = {0, -1, 0};  // P() operation
struct sembuf unlock = {0, 1, 0}; // V() operation

void create_semaphores(){
    // create semaphores for shared kitchen reasources
    mixer_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    bowl_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    spoon_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    oven_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    pantry_id = semget(IPC_PRIVATE, PANTRY_INGREDIENTS, IPC_CREAT | 0666);

    semctl(mixer_id, 0, SETVAL, 2); // Mixer = 2
    semctl(bowl_id, 0, SETVAL, 3);  // Bowl = 3
    semctl(spoon_id, 0, SETVAL, 5); // Spoon = 5
    semctl(oven_id, 0, SETVAL, 1);  // Oven = 1

    // create semaphores for ingredients in pantry
    for (int i = 0; i < PANTRY_INGREDIENTS; i++) {
        printf("pantry ingredient %d\n", i+1);
        semctl(pantry_id, i, SETVAL, 1);
    }

    // create semaphores for ingredients in refrigerators
    for (int frige = 0; frige < 2; frige++){
        refrigerator_id[frige] = semget(IPC_PRIVATE, REFRIGERATOR_INGREDIENTS, IPC_CREAT | 0666);
        for (int i = 0; i < REFRIGERATOR_INGREDIENTS; i ++){
            printf("refrigerator %d ingredient %d\n", frige + 1, i + 1);
            semctl(refrigerator_id[frige], i, SETVAL, 1);
        }
    }
    
}

// represent two refrigerators with ingredients
typedef struct{
    const char* ingredients[REFRIGERATOR_INGREDIENTS];
} refrigerator_struct;

refrigerator_struct fridge[2] = {{"Eggs", "Milk", "Butter"}, {"Eggs", "Milk", "Butter"}};

// represent a pantry with ingredients
typedef struct{
    const char* ingredients[PANTRY_INGREDIENTS];
} pantry_struct;

pantry_struct pantry = {{"Flour", "Sugar", "Yeast", "Baking Soda", "Salt", "Cinnamon"}};

// represents a recipe made up of ingredients from frige and pantry
typedef struct{
    const char* name;
    const char* pantry_ingredients[PANTRY_INGREDIENTS];
    const char* refridgerator_ingredients[REFRIGERATOR_INGREDIENTS];
} recipe;

recipe list[] = {
    {"cookies", {"Flour", "Sugar"}, {"Milk", "Butter"}},
    {"pancakes", {"Flour", "Sugar", "Baking Soda", "Salt"}, {"Eggs", "Milk", "Butter"}},
    {"homemade_pizza_dough", {"Yeast", "Sugar", "Salt"}, {}},
    {"soft_pretzels", {"Flour", "Sugar", "Salt", "Yeast", "Baking Sdoa"}, {"Eggs"}},
    {"cinnamon_rolls", {"Flour", "Sugar", "Salt", "Cinnamon"}, {"Butter", "Eggs"}}
};

void num_bakers(){
    char input[100];
    pthread_t *baker;

    printf("Enter the number if bakers: ");
    fgets(input, sizeof(input), stdin);
    int input_number = atoi(input);

    baker = (pthread_t *) malloc(input_number * sizeof(pthread_t));

    for(int i = 0; i < input_number; i++){
        if (pthread_create(&baker[i], NULL, baker_actions, NULL) != 0) {
            perror("Faild to create baker\n");
            free(baker);
            exit(1);
        }
        printf("Baker %d created\n", i+1);
    }
    for (int i = 0; i < input_number; i++){
        if(pthread_join(baker[i], NULL) != 0){
            perror("Failed to join baker\n");
            free(baker);
            exit(1);
        }
    }
    free(baker);
}

// TODO: fill out
void* baker_actions(){
    return NULL;
}

// TODO: fill out
void ramsied(){

}

void cleanup_semaphores(){
    semctl(mixer_id, 0, IPC_RMID);
    semctl(bowl_id, 0, IPC_RMID);
    semctl(spoon_id, 0, IPC_RMID);
    semctl(oven_id, 0, IPC_RMID);
    semctl(pantry_id, 0, IPC_RMID);

    for (int i = 0; i < 2; i++){
        semctl(refrigerator_id[i], 0, IPC_RMID);
    }
}

int main() {
    create_semaphores();
    num_bakers();
    ramsied();

    cleanup_semaphores();
    return 0;
}