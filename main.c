#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

/* Struct for list nodes 
https://www.geeksforgeeks.org/queue-linked-list-implementation/
*/
struct task_queue_s* queue = NULL;
struct list_node_s* list = NULL;
pthread_mutex_t my_mutex;
pthread_cond_t my_cond;
int THREAD_COUNT;
pthread_t ids[];




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

void Task_queue(int n){
    // Create tasks
    for (int i = 0; i < n; i++)
    {   
        // Enqueue the task
        Task_enqueue((rand()), (rand() % 3), rand());
        // Create the thread for given task
        pthread_create(&ids[i],NULL,&get_task,NULL);
        pthread_join(ids[i], NULL);
        pthread_cond_wait(&my_cond,&my_mutex);
    }
    // Broadcast to all threads that task queue is completed!
    pthread_cond_broadcast(&my_cond);
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
    pthread_mutex_lock(&my_mutex);
    temp = queue->front;
    pthread_mutex_unlock(&my_mutex);

    // Assign task values to the local pointers
    *task_num_p = temp->task_num;
    *task_type_p = temp->task_type;
    *value_p = temp->value;
    // Free temp node
    free(temp);
    // We use shared variable so lock mutex
    pthread_mutex_lock(&my_mutex);
    // Change front with next one
    queue->front = queue->front->next;
    // If front becomes NULL change back to NULL as well.
    if(queue->front == NULL){
        queue->back = NULL;
    }
    // We have finished so unlock mutex
    
    pthread_mutex_unlock(&my_mutex);
    return 1;
}

struct list_node_s* create_list_node(int value){
    struct list_node_s* newNode = (struct list_node_s*)malloc(sizeof(struct list_node_s));
    newNode->data = value;
    newNode->next = NULL;
    return newNode;
}

// Creates a new list node with given value and returns 1 after inserting it.
int Insert(int value){
    struct list_node_s* newNode = create_list_node(value);
    // Reaching list so use mutex
    struct list_node_s* tempNode = list;

    printf("tempNode values data: %d next: %p\n", tempNode->data,tempNode->next);
    // If the node is first node
    if(tempNode->data == -1 & tempNode->next == NULL){
        tempNode = newNode;
        return 1;
    }
    // Iterate over nodes
    while(tempNode->next != NULL){
        tempNode = tempNode->next;
    }
    // Last node
    tempNode->next = newNode;

    return 1;
}

// If given value is exists removes it and returns 1, otherwise, returns -1
int Delete(int value){
    struct list_node_s* tempNode = list;
    // If the value is first node
    if(tempNode->data == value){
        // Return data
        // Assign next value to first node
        tempNode = tempNode->next;
        return 0;
    }
    // Iterate over nodes
    while(tempNode->next != NULL){
        // We check first node's next if it is the value we will remove second and put third one to its spot
        if(tempNode->next->data == value){
            struct list_node_s* removedNode = tempNode->next;
            // Removed nodes next node assigned to the tempNode's next to complete connection.
            tempNode->next = removedNode->next;
            free(removedNode);
            return 0;
        }
        // If condition does not hold pass to next node.
        tempNode = tempNode->next;
    }
    // If we come here it means the desired value does not exists.
    printf("The value does not exists in the list so we could not delete it.");
    return -1;
}

// Searches tIf given value is exists removes it and returns 1, otherwise, returns -1
int Search(int value){
    struct list_node_s* tempNode = list;
    printf("Initial list address %p\n",tempNode);
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
// Single thread executes this function dequeues a task and executes the list command accordingly.
void* get_task(){
    long my_rank;
    int* task_num = malloc(sizeof(int));
    int* task_type = malloc(sizeof(int));
    int* value = malloc(sizeof(int));
    // Pass local attributes to the task dequeue function
    pthread_mutex_lock(&my_mutex);
    Task_dequeue(my_rank,task_num,task_type,value);
    pthread_mutex_unlock(&my_mutex);

    // Fill the list with passed arguments
    if(*task_type == 0){
        printf("Thread inserts value -> %d according to task number -> %d\n", *value, *task_num);
        pthread_mutex_lock(&my_mutex);
        if(Insert(*value) == 1){
            printf("Insert process completed successfully\n");
        }
        else{
            printf("Delete process failed\n");
        }
        pthread_mutex_unlock(&my_mutex);

    }
    else if(*task_type == 1){
        printf("Thread deletes value -> %d according to task number -> %d\n", *value, *task_num);
        pthread_mutex_lock(&my_mutex);
        if(Delete(*value) == 1){
            printf("Delete process completed successfully\n");
        }
        else{
            printf("Delete process failed\n");
        }
        pthread_mutex_unlock(&my_mutex);
    }
    else{
        printf("Thread searches value -> %d according to task number -> %d\n", *value, *task_num);
        pthread_mutex_lock(&my_mutex);
        if(Search(*value) == 1){
            printf("Search process completed successfully\n");
        }
        else{
            printf("Search process failed\n");
        }
        pthread_mutex_unlock(&my_mutex);
    }
    return 0;
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

void print_node_list(){
    struct list_node_s* tempNode = list;
    while(tempNode != NULL)
    {
        printf("List data: %d\n", tempNode->data);

        tempNode = tempNode->next;
    }
    
}

int main(){
    // Input from user but for now it will be static
    int THREAD_COUNT = 12;
    // Initialize ids
    pthread_t ids[THREAD_COUNT];
    // Mutex initialize
    pthread_mutex_init(&my_mutex,NULL);
    // Cond initialize
    pthread_cond_init(&my_cond,NULL);
    // Initiate task list
    list = initiate_task_list();
    // Initiate task queue
    queue = initiate_task_queue();
    // Create task queue with given number of tasks
    Task_queue(12);
    print_task_queue();
    // Destroy mutex
    pthread_mutex_destroy(&my_mutex);
    // Destroy cond
    pthread_cond_destroy(&my_cond);

    return 0;
}