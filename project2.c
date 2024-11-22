#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <time.h>
#include <unistd.h>

#define PANTRY_INGREDIENTS 6
#define REFRIGERATOR_INGREDIENTS 3

void* baker_actions(void*);
void gather_pantry_ingredients(int, int);
void gather_refrigerator_ingredients(int, int);
void mix_ingredients(int, int);
void cook_recipe(int, int);
void ramsied(int num_bakers, pthread_t *bakers, int *baker_ids);

int mixer_id, pantry_id, refrigerator_id, bowl_id, spoon_id, oven_id;

void semLock(int semId){
    struct sembuf lock = {0, -1, 0};
    if(semop(semId, &lock, 1) < 0){
        perror("semaphore lock failed\n");
        exit(1);
    }
}

void semUnlock(int semId){
    struct sembuf unlock = {0, 1, 0};
    if(semop(semId, &unlock, 1) < 0){
        perror("semaphore unlock failed\n");
        exit(1);
    }
}

void create_semaphores(){
    // create semaphores for shared kitchen reasources
    mixer_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    bowl_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    spoon_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    oven_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    pantry_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    refrigerator_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);

    semctl(mixer_id, 0, SETVAL, 2); // Mixer = 2
    semctl(pantry_id, 0, SETVAL, 1); // Pantry = 1
    semctl(refrigerator_id, 0, SETVAL, 2); // Refrigerator = 2
    semctl(bowl_id, 0, SETVAL, 3);  // Bowl = 3
    semctl(spoon_id, 0, SETVAL, 5); // Spoon = 5
    semctl(oven_id, 0, SETVAL, 1);  // Oven = 1
}

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


/*
void num_bakers(){
    char input[100];
    pthread_t *baker;
    int *baker_ids;

    printf("Enter the number if bakers: ");
    fgets(input, sizeof(input), stdin);
    int input_number = atoi(input);

    baker = (pthread_t *) malloc(input_number * sizeof(pthread_t));
    baker_ids = (int *) malloc(input_number * sizeof(int));

    for(int i = 0; i < input_number; i++){
        baker_ids[i] = i+1;
        if (pthread_create(&baker[i], NULL, baker_actions, &baker_ids[i]) != 0) {
            perror("Faild to create baker\n");
            free(baker);
            free(baker_ids);
            exit(1);
        }
        printf("Baker %d created\n", i+1);
    }
    for (int i = 0; i < input_number; i++){
        if(pthread_join(baker[i], NULL) != 0){
            perror("Failed to join baker\n");
            free(baker);
            free(baker_ids);
            exit(1);
        }
    }
    free(baker);
    free(baker_ids);

}

*/


int num_bakers(pthread_t **bakers_ptr, int **baker_ids_ptr) {
    char input[100];
    printf("Enter the number of bakers: ");
    fgets(input, sizeof(input), stdin);
    int input_number = atoi(input);

    // Allocate memory for baker threads and IDs
    *bakers_ptr = (pthread_t *)malloc(input_number * sizeof(pthread_t));
    *baker_ids_ptr = (int *)malloc(input_number * sizeof(int));

    pthread_t *bakers = *bakers_ptr;
    int *baker_ids = *baker_ids_ptr;

    for (int i = 0; i < input_number; i++) {
        baker_ids[i] = i + 1; // Assign unique baker ID
        if (pthread_create(&bakers[i], NULL, baker_actions, &baker_ids[i]) != 0) {
            perror("Failed to create baker thread");
            free(bakers);
            free(baker_ids);
            exit(1);
        }
        printf("Baker %d created\n", i + 1);
    }

    return input_number; // Return the number of bakers
}



// TODO: fix null ingredient access (currently commented out in the ingredient retrival for loops)
// TODO: unique color for each baker print statements
void* baker_actions(void* baker){
    int num_recipes = sizeof(list) / sizeof(recipe);
    int baker_id = *(int*)baker;

    // each baker goes through all recipes
    for (int i = 0; i < num_recipes; i++){
        gather_pantry_ingredients(i, baker_id);
        gather_refrigerator_ingredients(i, baker_id);
        mix_ingredients(i, baker_id);
        cook_recipe(i, baker_id);

        printf("Baker %d has completed the %s recipe\n", baker_id, list[i].name);
    }

    printf("Baker %d completed all recipes\n", baker_id);

    return NULL;
}

// TODO: fix so that refrigerator 1 and 2 are used. Currently, only refrigerator 1 is used
void gather_pantry_ingredients(int recipe_index, int baker_id){
    printf("Baker %d is ready to gather ingredients to make %s\n", baker_id, list[recipe_index].name);

    // only enter the pantry if the baker needs ingredients from the pantry
    if (list[recipe_index].pantry_ingredients[0] != NULL){
        semLock(pantry_id);
        printf("Baker %d has entered the pantry\n", baker_id);
        // get pantry ingredients
        for (int i = 0; i < PANTRY_INGREDIENTS; i++){
            const char* ingredient = list[recipe_index].pantry_ingredients[i];
            // printf("Baker %d took %s from the pantry for the %s recipe\n", baker_id, ingredient, list[recipe_index].name);

            // simulate baker walking to take next ingredient. If no more ingredients, 
            // baker walks out of pantry 
            sleep(1);
        }
        printf("Baker %d has gotten all pantry ingredients necessary for the %s recipe. Leaving pantry...\n", baker_id, list[recipe_index].name);
        semUnlock(pantry_id);
    } else{
        printf("Baker %d does not require any pantry ingredients for the %s recipe. Moving on...\n", baker_id, list[recipe_index].name);
    }
}

void gather_refrigerator_ingredients(int recipe_index, int baker_id){
    // only enter a refrigerator if the baker needs ingredients from a refrigerator
    if (list[recipe_index].refridgerator_ingredients[0] != NULL){
        semLock(refrigerator_id);
        int refrigerator_value = semctl(refrigerator_id, 0, GETVAL);

        // Question: how would I get the value of the semaphore and put it here? so like it would say refrigerator 1
        printf("Baker %d has entered refrigerator %d\n", baker_id, refrigerator_value);
        // get refriger ingredients
        for (int i = 0; i < REFRIGERATOR_INGREDIENTS; i++){
            const char* ingredient = list[recipe_index].refridgerator_ingredients[i];
            // printf("Baker %d took %s from refrigerator %d for the %s recipe\n", baker_id, ingredient, refrigerator_value, list[recipe_index].name);

            // simulate baker walking to take next ingredient. If no more ingredients, 
            // baker walks out of refrigerator 
            sleep(1);
        }
        printf("Baker %d has gotten all refrigerator ingredients necessary for the %s recipe. Leaving refrigerator %d...\n", baker_id, list[recipe_index].name, refrigerator_value);
        semUnlock(refrigerator_id);
    } else{
        printf("Baker %d does not require any refrigerator ingredients for the %s recipe. Moving on...\n", baker_id, list[recipe_index].name);
    }

    printf("Baker %d has gathered all ingredients for the %s recipe.\n", baker_id, list[recipe_index].name);
}

void mix_ingredients(int recipe_index, int baker_id){
    semLock(bowl_id);
    semLock(spoon_id);
    semLock(mixer_id);

    printf("Baker %d is ready to mix ingredients together for the %s recipe\n", baker_id, list[recipe_index].name);
    sleep(2);

    semUnlock(bowl_id);
    semUnlock(spoon_id);
    semUnlock(mixer_id);
}

void cook_recipe(int recipe_index, int baker_id){
    semLock(oven_id);

    printf("Baker %d is ready to cook the recipe for %s\n", baker_id, list[recipe_index].name);
    sleep(3);

    semUnlock(oven_id);
}


void* delayed_ramsied(void* args) {
    struct {
        int num_bakers;
        pthread_t* bakers;
        int* baker_ids;
    } *ramsay_args = args;

    srand(time(NULL));
    int delay = rand() % 11 + 5;
    printf("Chef Ramsay will intervene in %d seconds...\n", delay);

    sleep(delay);

    ramsied(ramsay_args->num_bakers, ramsay_args->bakers, ramsay_args->baker_ids);

    return NULL;
}



// TODO: fill out
//void ramsied(){

    // randomly select a baker through baker_id and ramsie
    // requirement is that the baker
    // drop all semaphores and recall baker_actions() to restart
void ramsied(int num_bakers, pthread_t *bakers, int *baker_ids) {
    // Randomly select a baker by their ID (1 to num_bakers)
    srand(time(NULL));
    int ramsied_baker_id = rand() % num_bakers + 1;

    printf("\033[1;31mChef Ramsay has intervened! Baker %d must restart their tasks!\033[0m\n", ramsied_baker_id); //colored blue

    // Drop semaphores held by the selected baker
    semUnlock(mixer_id);
    semUnlock(bowl_id);
    semUnlock(spoon_id);
    semUnlock(oven_id);
    semUnlock(pantry_id);
    semUnlock(refrigerator_id);

    printf("\033[1;34mBaker %d's semaphores have been reset. Restarting their tasks...\033[0m\n", ramsied_baker_id); //colored blue

    // Cancel the baker's existing thread
    if (pthread_cancel(bakers[ramsied_baker_id - 1]) != 0) {
        perror("Failed to cancel baker's thread\n");
        return;
    }

    // Restart the baker's thread
    if (pthread_create(&bakers[ramsied_baker_id - 1], NULL, baker_actions, &baker_ids[ramsied_baker_id - 1]) != 0) {
        perror("Failed to restart baker's thread\n");
        exit(1);
    }

    printf("\033[1;32mBaker %d has restarted their tasks!\033[0m\n", ramsied_baker_id); //colored green
    
}

void cleanup_semaphores(){
    semctl(mixer_id, 0, IPC_RMID);
    semctl(bowl_id, 0, IPC_RMID);
    semctl(spoon_id, 0, IPC_RMID);
    semctl(oven_id, 0, IPC_RMID);
    semctl(pantry_id, 0, IPC_RMID);
    semctl(refrigerator_id, 0, IPC_RMID);
}


/*
int main() {
    create_semaphores();
    num_bakers();
    ramsied();

    cleanup_semaphores();
    return 0;

}
*/

/*int main() {
    pthread_t *bakers;
    int *baker_ids;
    int num_bakers_count;

    // Initialize semaphores
    create_semaphores();

    // Get the number of bakers and create their threads
    num_bakers_count = num_bakers(&bakers, &baker_ids);

    // Trigger Ramsay's intervention
    ramsied(num_bakers_count, bakers, baker_ids);

    // Wait for all bakers to complete
    for (int i = 0; i < num_bakers_count; i++) {
        if (pthread_join(bakers[i], NULL) != 0) {
            perror("Failed to join baker thread");
            free(bakers);
            free(baker_ids);
            cleanup_semaphores();
            exit(1);
        }
    }

    // Free allocated memory
    free(bakers);
    free(baker_ids);

    // Cleanup semaphores
    cleanup_semaphores();

    return 0;
}
*/



int main() {
    pthread_t *bakers;
    int *baker_ids;
    int num_bakers_count;

    create_semaphores();

    num_bakers_count = num_bakers(&bakers, &baker_ids);

    pthread_t ramsay_tid;
    struct {
        int num_bakers;
        pthread_t* bakers;
        int* baker_ids;
    } ramsay_args = { num_bakers_count, bakers, baker_ids };

    if (pthread_create(&ramsay_tid, NULL, delayed_ramsied, &ramsay_args) != 0) {
        perror("Failed to create Ramsay thread");
        free(bakers);
        free(baker_ids);
        cleanup_semaphores();
        exit(1);
    }

    for (int i = 0; i < num_bakers_count; i++) {
        if (pthread_join(bakers[i], NULL) != 0) {
            perror("Failed to join baker thread");
            free(bakers);
            free(baker_ids);
            cleanup_semaphores();
            exit(1);
        }
    }

    if (pthread_join(ramsay_tid, NULL) != 0) {
        perror("Failed to join Ramsay thread");
        free(bakers);
        free(baker_ids);
        cleanup_semaphores();
        exit(1);
    }

    free(bakers);
    free(baker_ids);

    cleanup_semaphores();

    return 0;
}
