#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>


/* Struct for list nodes 
https://www.geeksforgeeks.org/queue-linked-list-implementation/
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
void exec_task(int thread_id){

    long my_rank;
    int* task_num = malloc(sizeof(int));
    int* task_type = malloc(sizeof(int));
    int* value = malloc(sizeof(int));

    printf("Thread %d is locked for dequeue\n", thread_id);
    pthread_mutex_lock(&my_mutex);
    Task_dequeue(my_rank,task_num,task_type,value);
    printf("Dequeue finished thread %d will be unlocked\n", thread_id);
    pthread_mutex_unlock(&my_mutex);

    // Fill the list with passed arguments
    if(*task_type == 0){
        printf("Thread %d | INSERT | Value: %d | Task Number: %d\n", thread_id, *value, *task_num);
        pthread_mutex_lock(&my_mutex);
        if(Insert(*value) == 1){
            printf("SUCCESSFULL | INSERT PROCESS | THREAD %d\n", thread_id);
        }
        else{
            printf("FAILED | INSERT PROCESS | THREAD: %d\n", thread_id);
        }
        pthread_mutex_unlock(&my_mutex);

    }
    else if(*task_type == 1){
        printf("Thread %d | DELETE | Value: %d | Task Number: %d\n", thread_id, *value, *task_num);
        pthread_mutex_lock(&my_mutex);
        if(Delete(*value) == 1){
            printf("SUCCESSFUL | DELETE PROCESS | THREAD: %d\n", thread_id);
        }
        else{
            printf("FAILED | DELETE PROCESS | THREAD: %d\n", thread_id);
        }
        pthread_mutex_unlock(&my_mutex);
    }
    else{
        printf("Thread %d | SEARCH | Value: %d | Task Number: %d\n", thread_id, *value, *task_num);
        pthread_mutex_lock(&my_mutex);
        if(Search(*value) == 1){
            printf("SUCCESSFUL | SEARCH PROCESS | THREAD: %d\n", thread_id);
        }
        else{
            printf("FAILED | SEARCH PROCESS | THREAD: %d\n", thread_id);
        }
        pthread_mutex_unlock(&my_mutex);
    }

}

// Single thread executes this function dequeues a task and executes the list command accordingly.
void* get_task(){
    // Get thread id
    int thread_id = syscall(__NR_gettid);

    thread_id = thread_id % THREAD_COUNT;
    
    // Lock for wait condition
    pthread_mutex_lock(&my_mutex);
    printf("Thread %d is in wait condition\n",thread_id);
    pthread_cond_wait(&my_cond, &my_mutex);
    printf("Thread %d is exited from wait condition\n", thread_id);
    pthread_mutex_unlock(&my_mutex);

    exec_task(thread_id);
    // When process finishes put it to the wait again
    pthread_mutex_lock(&my_mutex);
    while (!conditionMet) {
        printf("Thread %d is blocked and waiting for broadcast\n", thread_id);
        pthread_cond_wait(&my_cond, &my_mutex);
    }
    // Broadcast happened
    printf("Thread %d awakeee mutex unlocked!\n", thread_id);
    pthread_mutex_unlock(&my_mutex);

    pthread_mutex_lock(&my_mutex);
    while(queue->front != NULL){
        
        pthread_mutex_unlock(&my_mutex);
        exec_task(thread_id);
    }
    
    return 0;
}


void Task_queue(int n){
    write(1,"Tasks will be created in for loop\n",sizeof("Tasks will be created in for loop\n"));
    // Create tasks
    for (int i = 0; i < n; i++)
    {   
        // Enqueue the task
        Task_enqueue((rand() % TASK_COUNT), (rand() % 3), (rand()% 5));
        
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
    // We have finished so unlock mutex
    
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
    
    printf("print test list address: %p tempNode address: %p\n", list, tempNode);
    while(tempNode != NULL)
    {
        printf("List data: %d\n", tempNode->data);

        tempNode = tempNode->next;
        printf("In FOR LOOP | print test list address: %p tempNode address: %p\n", list, tempNode);

    }
    
}

// Creates a new list node with given value and returns 1 after inserting it.
int Insert(int value){

    //0 1 7 8 12

    
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

int main(){
    THREAD_COUNT = 2;
    TASK_COUNT = 15;
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
    conditionMet = 1;
    write(1,"Condition is met\n",sizeof("Condition is met\n"));
    pthread_cond_broadcast(&my_cond);
    
    join_threads(THREAD_COUNT,ids);

    print_node_list();
    
    // Destroy mutex
    pthread_mutex_destroy(&my_mutex);
    // Destroy cond
    pthread_cond_destroy(&my_cond);

    return 0;
}