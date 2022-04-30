#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <time.h>


/* Struct for list nodes 
https://www.geeksforgeeks.org/queue-linked-list-implementation/
*/
/* For program execution time
https://www.techiedelight.com/find-execution-time-c-program/
*/
struct task_queue_s* queue = NULL;
struct list_node_s* list = NULL;
pthread_mutex_t my_mutex;
pthread_cond_t my_cond;
int conditionMet = 0;
int THREAD_COUNT = 1;
int TASK_COUNT = 1;

struct pthread_arg_s {
    long my_rank;
    int* task_num;
    int* task_type; 
    int* value;
};

struct list_node_s {
int data;
struct list_node_s* next;
};
/* Struct for task nodes */
struct task_node_s {
int task_num;
int task_type; // insert:0, delete:1, search:2
int value;
struct task_node_s* next;
};

/* Struct for task queue */
struct task_queue_s {
    struct task_node_s* front;
    struct task_node_s* back;
};
/* List operations */
int Insert(int value);
int Delete(int value);
int Search(int value);
/* Task queue functions */
void Task_queue(int n); //generate random tasks for the task queue
void Task_enqueue(int task_num, int task_type, int value); //insert a new task into task queue
int Task_dequeue(long my_rank, int* task_num_p, int* task_type_p, int* value_p); //take a task from task queue

// Initiate task list
struct list_node_s* initiate_task_list(){
    // Create its space in the memory
    struct list_node_s* tempList = (struct list_node_s*)malloc(sizeof(struct list_node_s));
    // Initiate variables with null and 
    tempList->data = -1;
    tempList->next = NULL;
    return tempList;
}
// Initiate task queue with initial NULL values
struct task_queue_s* initiate_task_queue(){
    // Create its space in the memory
    struct task_queue_s* tempQueue = (struct task_queue_s*)malloc(sizeof(struct task_queue_s));
    // Initiate variables with null
    tempQueue->front = NULL;
    tempQueue->back = NULL;
    return tempQueue;
}

struct task_node_s* create_task_node(int task_num, int task_type, int value){
    struct task_node_s* tempTaskNode = (struct task_node_s *) malloc(sizeof(struct task_node_s));
    tempTaskNode->task_num = task_num;
    tempTaskNode->task_type = task_type;
    tempTaskNode->value = value;
    tempTaskNode->next = NULL;
    return tempTaskNode;
}

// Executes tasks creating local variables and making processes accordingly
// Must called with unlocked thread.
int exec_task(int thread_id){

    long my_rank;
    int* task_num = malloc(sizeof(int));
    int* task_type = malloc(sizeof(int));
    int* value = malloc(sizeof(int));
    *value = -1;

    pthread_mutex_lock(&my_mutex);
    //printf("Thread %d is LOCKED for dequeue\n", thread_id);
    int returnValue = Task_dequeue(my_rank,task_num,task_type,value);
    pthread_mutex_unlock(&my_mutex);
    //printf("Thread %d is UNLOCKED after dequeue\n", thread_id);

    if(returnValue == -1){
        //printf("Thread %d is UNLOCKED could not take\n", thread_id);
        return 0;
    }

  
    // Fill the list with passed arguments
    if(*task_type == 0){
        pthread_mutex_lock(&my_mutex);
        //printf("Thread %d is LOCKED for insert process\n", thread_id);

        //printf("Thread %d | INSERT | Value: %d | Task Number: %d\n", thread_id, *value, *task_num);
        if(Insert(*value) == 1){
            printf("OKEY | INSERT | THREAD: %d | TASK: %d | VALUE: %d\n", thread_id, *task_num, *value);

        }
        else{
            printf("FAIL | INSERT | THREAD: %d | TASK: %d | VALUE: %d\n", thread_id, *task_num, *value);

        }
        //printf("Thread %d is UNLOCKED insert process finished\n", thread_id);
        pthread_mutex_unlock(&my_mutex);
        return 1;

    }
    else if(*task_type == 1){
        pthread_mutex_lock(&my_mutex);
        //printf("Thread %d is LOCKED for insert process\n", thread_id);
        // printf("Thread %d | DELETE | Value: %d | Task Number: %d\n", thread_id, *value, *task_num);
        if(Delete(*value) == 1){
            printf("SUCCESSFUL | DELETE | THREAD: %d | TASK: %d | VALUE: %d\n", thread_id, *task_num, *value);

        }
        else{
            printf("FAIL | DELETE | THREAD: %d | TASK: %d | VALUE: %d\n", thread_id, *task_num, *value);

        }
        //printf("Thread %d is UNLOCKED delete process finished\n", thread_id);
        pthread_mutex_unlock(&my_mutex);
        return 1;

    }
    
    else if(*task_type == 2){
        pthread_mutex_lock(&my_mutex);
        //printf("Thread %d is LOCKED for insert process\n", thread_id);
        //printf("Thread %d | SEARCH | Value: %d | Task Number: %d\n", thread_id, *value, *task_num);
        if(Search(*value) == 1){
            printf("SUCCESSFUL | SEARCH | THREAD: %d | TASK: %d | VALUE: %d\n", thread_id, *task_num, *value);
        }
        else{
            printf("FAIL | SEARCH | THREAD: %d | TASK: %d | VALUE: %d\n", thread_id, *task_num, *value);
        }
        //printf("Thread %d is UNLOCKED search process finished\n", thread_id);
        pthread_mutex_unlock(&my_mutex);
        return 1;

    }

    //printf("FAILED | UNKNOWN PROCESS | THREAD: %d\n", thread_id);
    return 0;
}

// Single thread executes this function dequeues a task and executes the list command accordingly.
// Continues to execute while task queue is not empty.
void* get_task(){
    // Get thread id
    pid_t thread_id = gettid() % THREAD_COUNT;
    
    // Lock for wait condition
    pthread_mutex_lock(&my_mutex);
    //printf("Thread %d is locked\n",thread_id);
    if(!conditionMet){
        //printf("Thread %d is in wait condition\n",thread_id);
        pthread_cond_wait(&my_cond, &my_mutex);
        //printf("Thread %d is signaled and exited from wait condition\n", thread_id);
    }
    //printf("Thread %d will be unlocked\n", thread_id);
    pthread_mutex_unlock(&my_mutex);

    exec_task(thread_id);
    // When process finishes put it to the wait again
    pthread_mutex_lock(&my_mutex);
    //printf("Thread %d is locked\n",thread_id);
    while (!conditionMet) {
        //printf("Thread %d is blocked and waiting for broadcast\n", thread_id);
        pthread_cond_wait(&my_cond, &my_mutex);
    }
    // Broadcast happened
    //printf("Thread %d is unlocked!\n", thread_id);
    pthread_mutex_unlock(&my_mutex);

    // Get remaining tasks
    pthread_mutex_lock(&my_mutex);
    //printf("Thread %d is locked for queue->front\n",thread_id);
    struct task_node_s* frontNode = queue->front;

    if(queue->front == NULL){
        //printf("Thread %d is unlocked! EMPTY queue->front\n", thread_id);
        pthread_mutex_unlock(&my_mutex);
        //printf("All tasks are finished my job is done!! | THREAD %d\n", thread_id);
        return 0;
    }
    
    while(frontNode != NULL){
        //printf("Thread %d has been trying to lock!\n", thread_id);
        pthread_mutex_trylock(&my_mutex); 
        //printf("Thread %d is unlocked! after queue->front\n", thread_id);
        pthread_mutex_unlock(&my_mutex);
        if(exec_task(thread_id) != 0){
            // Update front node
            //printf("Thread %d has been trying to lock!\n", thread_id);
            pthread_mutex_trylock(&my_mutex); 
            frontNode = queue->front;
            //printf("Thread %d is unlocked! after queue->front\n", thread_id);
            pthread_mutex_unlock(&my_mutex);
        };
    }

    return 0;
}


void Task_queue(int n){
    write(1,"Tasks will be created in for loop\n",sizeof("Tasks will be created in for loop\n"));
    // Create tasks
    for (int i = 0; i < n; i++)
    {   
        // Enqueue the task
        Task_enqueue((rand() % TASK_COUNT), (rand() % 3), (rand()% TASK_COUNT));
        
        // Signal the thread to dequeue the task
        pthread_cond_signal(&my_cond);
    }   
}

void Task_enqueue(int task_num, int task_type, int value){
    struct task_node_s* task_node = create_task_node(task_num, task_type, value);
    // Queue is shared variable so we must use mutex here
    pthread_mutex_lock(&my_mutex);
    // If the queue is empty assign initial node to rear and front.
        if(queue->front == NULL){
            queue->front = task_node;
            queue->back = task_node;
        }
        else{
            queue->back->next = task_node;
            queue->back = task_node;
        }
    // We finished our job we can unlock it
    pthread_mutex_unlock(&my_mutex);


}

int Task_dequeue(long my_rank, int* task_num_p, int* task_type_p, int* value_p){
    struct task_node_s* temp = (struct task_node_s*)malloc(sizeof(struct task_node_s));
    // If the queue is empty
    if(queue->front == NULL){
        return -1;
    }    
    // Store front node in the temp node.
    temp = queue->front;

    // Assign task values to the local pointers
    *task_num_p = temp->task_num;
    *task_type_p = temp->task_type;
    *value_p = temp->value;
    // Free temp node
    free(temp);
    // We use shared variable so lock mutex
    // Change front with next one
    queue->front = queue->front->next;
    // If front becomes NULL change back to NULL as well.
    if(queue->front == NULL){
        queue->back = NULL;
    }
    
    return 1;
}

struct list_node_s* create_list_node(int value){
    struct list_node_s* newNode = (struct list_node_s*)malloc(sizeof(struct list_node_s));
    newNode->data = value;
    newNode->next = NULL;
    return newNode;
}

void print_node_list(){
    struct list_node_s* tempNode = list;
    printf("List data: ");
    while(tempNode != NULL)
    {
        printf("%d ", tempNode->data);

        tempNode = tempNode->next;
    }
    printf("\n");
}

// Creates a new list node with given value and returns 1 after inserting it.
// It ensures ascending order insertion while doing that.
// Does not allow duplicate values!
int Insert(int value){
    struct list_node_s* newNode = create_list_node(value);
    
    // If list is empty
    if(list == NULL){
        
        list = newNode;
        return 1;
    }

    // List is head pointer so we will store it in local variable to not change it.
    struct list_node_s* headNode = list;

    // If inserted node is smaller than first(head) node or equal to first node!
    if(headNode == list){
        if(headNode->data == newNode->data){
            write(1, "Duplicate value can not be accepted to the list\n", sizeof("Duplicate value can not be accepted to the list\n"));
            return 0;
        }
        // First node must be updated
        else if(headNode->data > newNode->data){
            struct list_node_s* tempNode = headNode;    
            // Add head node to the new node's tail
            newNode->next = headNode;
            // Change head pointer and update head with new node
            list = newNode;
            return 1;
        }
    }
    

    while(headNode->next != NULL){
        if(headNode->data == newNode->data){
            write(1, "Duplicate value can not be accepted to the list\n", sizeof("Duplicate value can not be accepted to the list\n"));
            return 0;
        }
        // Node 1 (headNode) - New node - Node 2 (headNode->next)
        else if(headNode->data < newNode->data && headNode->next->data > newNode->data){
            // Node 2 is stored in tempNode
            struct list_node_s* tempNode = headNode->next;
            // New node added to node1's tail
            headNode->next = newNode;
            // Node2 is added to new node's tail
            newNode->next = tempNode;
            return 1;
        }
        headNode = headNode->next;
    }
    if(headNode->data == newNode->data){
        write(1, "Duplicate value can not be accepted to the list\n", sizeof("Duplicate value can not be accepted to the list\n"));
        return 0;
    }
    // If conditions are met add new node to the tail
    headNode->next = newNode;
    return 1;
}

// If given value is exists removes it and returns 1, otherwise, returns -1
int Delete(int value){
    struct list_node_s* tempNode = list;

    if(tempNode == NULL){
        return -1;
    }
    // If the value is first node
    if(tempNode->data == value){
        // Return data
        // Assign next value to first node
        list = tempNode->next;
        return 1;
    }
    // Iterate over nodes
    while(tempNode->next != NULL){
        // We check first node's next if it is the value we will remove second and put third one to its spot
        if(tempNode->next->data == value){
            struct list_node_s* removedNode = tempNode->next;
            // Removed nodes next node assigned to the tempNode's next to complete connection.
            tempNode->next = removedNode->next;
            free(removedNode);
            return 1;
        }
        // If condition does not hold pass to next node.
        tempNode = tempNode->next;
    }
    // If we come here it means the desired value does not exists.
    return -1;
}

// Searches tIf given value is exists removes it and returns 1, otherwise, returns -1
int Search(int value){    
    struct list_node_s* tempNode = list;
    if(tempNode == NULL){
        return -1;
    }
    // Iterate over nodes
    while(tempNode != NULL){
        if(tempNode->data == value){
            return 1;
        }
        tempNode = tempNode->next;
    }
    // Last node
    return -1;
}

void print_task_queue(){
    struct task_node_s* tempNode = queue->front;
    while(tempNode != queue->back)
    {
        printf("Task value: %d\n", tempNode->value);
        printf("Task task type: %d\n", tempNode->task_type);
        printf("Task task number: %d\n", tempNode->task_num);

        tempNode = tempNode->next;
    }
}



void create_threads(int thread_count, pthread_t ids[]){
    for (int i = 0; i < thread_count; i++)
    {
        pthread_create(&ids[i],NULL,&get_task,NULL);
    }
}

void join_threads(int thread_count, pthread_t ids[]){
    for (int i = 0; i < thread_count; i++)
    {
        pthread_join(ids[i],NULL);
    }
    
}

int main(int argc, char* argv[]){
    // Time calculator
    time_t begin = time(NULL);
    // User input checker
    if(argc != 3){
        printf("Missing arguments!\n");
        printf("Usage: <filename> <thread_count> <task_count>\n");
        exit(-1);
    }
    // User input
    int THREAD_COUNT_INPUT = atoi(argv[1]);   
    int TASK_COUNT_INPUT = atoi(argv[2]);
    // User input finishes

    // Initializes const variables
    THREAD_COUNT = THREAD_COUNT_INPUT;
    TASK_COUNT = TASK_COUNT_INPUT;
    pthread_t ids[THREAD_COUNT];
    // Mutex initialize
    pthread_mutex_init(&my_mutex,NULL);
    // Cond initialize
    pthread_cond_init(&my_cond,NULL);
    // Initiate task queue
    queue = initiate_task_queue();

    create_threads(THREAD_COUNT, ids);

    // Create task queue with given number of tasks
    write(1,"Task queue will be executed!\n",sizeof("Task queue will be executed!\n"));    
    Task_queue(TASK_COUNT);
    // After inserting all tasks broadcast to all threads
    conditionMet = 1;
    write(1,"Condition is met\n",sizeof("Condition is met\n"));
    pthread_cond_broadcast(&my_cond);

    // Join threads
    join_threads(THREAD_COUNT,ids);
    // Print list
    print_node_list();

    // Destroy mutex
    pthread_mutex_destroy(&my_mutex);
    // Destroy cond
    pthread_cond_destroy(&my_cond);
    // Time calculator end!
    time_t end = time(NULL);

    // calculate elapsed time by finding difference (end - begin)
    printf("The elapsed time is %d seconds", (end - begin));

    return 0;
}