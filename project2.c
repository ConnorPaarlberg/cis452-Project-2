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
#define MAX_BAKERS 100

void* baker_actions(void*);
void gather_pantry_ingredients(int, int);
void gather_refrigerator_ingredients(int, int);
void mix_ingredients(int, int);
void cook_recipe(int, int);
void ramsied(int, int);

int mixer_id, pantry_id, refrigerator_id_1, refrigerator_id_2, bowl_id, spoon_id, oven_id;
const char *baker_colors[MAX_BAKERS];

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
    refrigerator_id_1 = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    refrigerator_id_2 = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);

    semctl(mixer_id, 0, SETVAL, 2);
    semctl(pantry_id, 0, SETVAL, 1);
    semctl(refrigerator_id_1, 0, SETVAL, 1);
    semctl(refrigerator_id_2, 0, SETVAL, 1);
    semctl(bowl_id, 0, SETVAL, 3);
    semctl(spoon_id, 0, SETVAL, 5);
    semctl(oven_id, 0, SETVAL, 1);
}

// represents a recipe made up of ingredients from refrigerator and pantry
typedef struct{
    const char* name;
    const char* pantry_ingredients[PANTRY_INGREDIENTS];
    const char* refridgerator_ingredients[REFRIGERATOR_INGREDIENTS];
} recipe;

recipe list[] = {
    {"cookies", {"Flour", "Sugar"}, {"Milk", "Butter"}},
    {"pancakes", {"Flour", "Sugar", "Baking Soda", "Salt"}, {"Egg", "Milk", "Butter"}},
    {"homemade_pizza_dough", {"Yeast", "Sugar", "Salt"}, {}},
    {"soft_pretzels", {"Flour", "Sugar", "Salt", "Yeast", "Baking Soda"}, {"Egg"}},
    {"cinnamon_rolls", {"Flour", "Sugar", "Salt", "Cinnamon"}, {"Butter", "Eggs"}}
};

// struct of arguments for backer_actions()
typedef struct {
    int baker_id;        // ID of the baker
    int unlucky_baker;   // ID of the unlucky baker
} baker_actions_args;

void num_bakers(){
    const char *colors[] = {
        "\033[1;32m", // Green
        "\033[1;33m", // Yellow
        "\033[1;34m", // Blue
        "\033[1;35m", // Magenta
        "\033[1;36m", // Cyan
        "\033[1;37m", // White
        "\033[1;92m", // Bright Green
        "\033[1;93m", // Bright Yellow
        "\033[1;94m", // Bright Blue
        "\033[1;95m", // Bright Magenta
        "\033[1;96m", // Bright Cyan
        "\033[1;97m"  // Bright White
    };

    int num_colors = sizeof(colors) / sizeof(colors[0]);


    char input[100];
    pthread_t *baker;
    baker_actions_args *baker_args;

    printf("Enter the number of bakers: ");
    fgets(input, sizeof(input), stdin);
    int input_number = atoi(input);

    //seed random number generator
    srand(time(NULL));
    // assign the baker that will get ramsied
    int unlucky_baker = (rand() % input_number) + 1;

    baker = (pthread_t *) malloc(input_number * sizeof(pthread_t));
    baker_args = (baker_actions_args *) malloc(input_number * sizeof(baker_actions_args));

    for(int i = 0; i < input_number; i++){
        baker_args[i].baker_id = i+1;
        baker_args[i].unlucky_baker = unlucky_baker;
        baker_colors[i + 1] = colors[i % num_colors]; // Assign a color based on baker ID

        if (pthread_create(&baker[i], NULL, baker_actions, &baker_args[i]) != 0) {
            perror("Faild to create baker\n");
            free(baker);
            free(baker_args);
            exit(1);
        }
        printf("%sBaker %d created\033[0m\n", baker_colors[i + 1], i + 1);
    }
    for (int i = 0; i < input_number; i++){
        if(pthread_join(baker[i], NULL) != 0){
            perror("Failed to join baker\n");
            free(baker);
            free(baker_args);
            exit(1);
        }
    }
    free(baker);
    free(baker_args);
}

void* baker_actions(void* args){
    baker_actions_args *baker_data = (baker_actions_args *)args;
    int baker_id = baker_data->baker_id;
    int unlucky_baker = baker_data->unlucky_baker;

    int num_recipes = sizeof(list) / sizeof(recipe);

    const char *baker_color = baker_colors[baker_id]; 

    // decide what recipe ramsy will step in
    int ramsay_time = (rand() % num_recipes);
    // only happens once
    int ramsy_disruption = 0;
    // each baker goes through all recipes
    for (int i = 0; i < num_recipes; i++){
        int restart_recipe = 0;
        do{
            restart_recipe = 0;

            gather_refrigerator_ingredients(i, baker_id);
            gather_pantry_ingredients(i, baker_id);
            mix_ingredients(i, baker_id);
            // at the chosen random recipe, ramsey is invoked on the unlucky baker
            if (ramsay_time == i && baker_id == unlucky_baker && ramsy_disruption == 0){
                printf("\033[1;31mBaker %d has been chosen to be ramsied on the %s recipe\033[0m\n", baker_id, list[i].name);
                ramsied(i, baker_id);
                ramsy_disruption = 1;
                restart_recipe = 1;
                continue;
            }
            cook_recipe(i, baker_id);
        } while(restart_recipe);
        printf("%sBaker %d has completed the %s recipe\033[0m\n", baker_color, baker_id, list[i].name);
    }

    printf("%sBaker %d completed all recipes\033[0m\n", baker_color, baker_id);

    return NULL;
}

void gather_refrigerator_ingredients(int recipe_index, int baker_id){
    const char *baker_color = baker_colors[baker_id]; 

    // only enter a refrigerator if the baker needs ingredients from a refrigerator
    if (list[recipe_index].refridgerator_ingredients[0] != NULL){
        // alternate between refrigerator 1 and 2
        int chosen_refrigerator = baker_id % 2 + 1;
        if (chosen_refrigerator == 1){
            semLock(refrigerator_id_1);
        } else{
            semLock(refrigerator_id_2);
        }
        printf("%sBaker %d is ready to gather ingredients to make %s\033[0m\n",baker_color, baker_id, list[recipe_index].name);
        // get refriger ingredients
        for (int i = 0; i < REFRIGERATOR_INGREDIENTS; i++){
            if (list[recipe_index].refridgerator_ingredients[i] != NULL){
                printf("%sBaker %d took %s from refrigerator %d for the %s recipe\033[0m\n", baker_color, baker_id, 
                list[recipe_index].refridgerator_ingredients[i], chosen_refrigerator, list[recipe_index].name);
            }
            // simulate baker walking to take next ingredient. If no more ingredients, 
            // baker walks out of refrigerator 
            sleep(1);
        }
        printf("%sBaker %d has gotten all refrigerator ingredients necessary for the %s recipe. Leaving refrigerator %d...\033[0m\n", 
        baker_color, baker_id, list[recipe_index].name, chosen_refrigerator);
        if (chosen_refrigerator == 1){
            semUnlock(refrigerator_id_1);
        } else{
            semUnlock(refrigerator_id_2);
        }
    } else{
        printf("%sBaker %d does not require any refrigerator ingredients for the %s recipe. Moving on...\033[0m\n", baker_color, baker_id, 
        list[recipe_index].name);
    }
}


void gather_pantry_ingredients(int recipe_index, int baker_id){
    const char *baker_color = baker_colors[baker_id]; 

    // only enter the pantry if the baker needs ingredients from the pantry
    if (list[recipe_index].pantry_ingredients[0] != NULL){
        semLock(pantry_id);
        printf("%sBaker %d has entered the pantry\033[0m\n", baker_color, baker_id);
        // get pantry ingredients
        for (int i = 0; i < PANTRY_INGREDIENTS; i++){
            if (list[recipe_index].pantry_ingredients[i] != NULL){
                printf("%sBaker %d took %s from the pantry for the %s recipe\033[0m\n", baker_color, 
                baker_id, list[recipe_index].pantry_ingredients[i], list[recipe_index].name);
            }
            // simulate baker walking to take next ingredient. If no more ingredients, 
            // baker walks out of pantry 
            sleep(1);
        }
        printf("%sBaker %d has gotten all pantry ingredients necessary for the %s recipe. Leaving pantry...\033[0m\n", 
        baker_color, baker_id, list[recipe_index].name);
        semUnlock(pantry_id);
    } else{
        printf("%sBaker %d does not require any pantry ingredients for the %s recipe. Moving on...\033[0m\n", 
        baker_color, baker_id, list[recipe_index].name);
    }
}

void mix_ingredients(int recipe_index, int baker_id){
    const char *baker_color = baker_colors[baker_id]; 

    semLock(bowl_id);
    semLock(spoon_id);
    semLock(mixer_id);

    printf("%sBaker %d is ready to mix ingredients together for the %s recipe\033[0m\n", baker_color, baker_id, list[recipe_index].name);
    sleep(2);

    semUnlock(bowl_id);
    semUnlock(spoon_id);
    semUnlock(mixer_id);
}

void cook_recipe(int recipe_index, int baker_id){
    const char *baker_color = baker_colors[baker_id]; 

    semLock(oven_id);

    printf("%sBaker %d is ready to cook the recipe for %s\033[0m\n", baker_color, baker_id, list[recipe_index].name);
    sleep(3);

    semUnlock(oven_id);
}

void ramsied(int recipe_index, int baker_id){
    printf("\033[1;31mChef Ramsay has intervened! Baker %d must restart their tasks!\033[0m\n", baker_id);
    // Drop semaphores held by the selected baker
    semUnlock(mixer_id);
    semUnlock(bowl_id);
    semUnlock(spoon_id);
    semUnlock(oven_id);
    semUnlock(pantry_id);
    semUnlock(refrigerator_id_1);
    semUnlock(refrigerator_id_2);
}

void cleanup_semaphores(){
    semctl(mixer_id, 0, IPC_RMID);
    semctl(bowl_id, 0, IPC_RMID);
    semctl(spoon_id, 0, IPC_RMID);
    semctl(oven_id, 0, IPC_RMID);
    semctl(pantry_id, 0, IPC_RMID);
    semctl(refrigerator_id_1, 0, IPC_RMID);
    semctl(refrigerator_id_2, 0, IPC_RMID);

}

int main() {
    create_semaphores();
    num_bakers();

    printf("All Bakers have completed all recipes. Time to cleanup...\n");
    cleanup_semaphores();
    return 0;
}