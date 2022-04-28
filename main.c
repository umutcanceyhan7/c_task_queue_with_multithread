#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

/* Struct for list nodes 
https://www.geeksforgeeks.org/queue-linked-list-implementation/
*/
struct task_queue_s* queue = NULL;
struct list_node_s* list = NULL;

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
    // Initiate variables with null
    tempList->data = 0;
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
    // Initiate task queue
    queue = initiate_task_queue();
    // Create tasks
    for (int i = 0; i < n; i++)
    {   
        // Enqueue the task
        Task_enqueue((rand()), (rand() % 3), rand());
    }
}

void Task_enqueue(int task_num, int task_type, int value){
    struct task_node_s* task_node = create_task_node(task_num, task_type, value);
    // If the queue is empty assign initial node to rear and front.
        if(queue->front == NULL){
            queue->front = task_node;
            queue->back = task_node;
        }
        else{
            queue->back->next = task_node;
            queue->back = task_node;
        }

}

int Task_dequeue(long my_rank, int* task_num_p, int* task_type_p, int* value_p){
    printf("BURADAYIM\n");
    // If the queue is empty
    if(queue->front == NULL){
        return -1;
    }    
    // Store front node in the temp node.
    struct task_node_s* temp = (struct task_node_s*)malloc(sizeof(struct task_node_s));
    temp = queue->front;
    write(1,"deneme\n",sizeof("deneme\n"));
    // Assign task values to the local pointers
    printf("%d temp task type\n",temp->task_type);
    *task_num_p = temp->task_num;
    *task_type_p = temp->task_type;
    *value_p = temp->value;
    printf("%p task type adressss\n", task_type_p);
    printf("%d task type p\n", *task_type_p);

    /*
    
    // Change front with next one
    queue->front = queue->front->next;
    // If front becomes NULL change back to NULL as well.
    if(queue->front == NULL){
        queue->back = NULL;
    }
    free(temp);
    
    */
    return 1;
    

}

struct list_node_s* create_list_node(int value){
    struct list_node_s* newNode = (struct list_node_s*)malloc(sizeof(struct list_node_s));
    newNode->data = value;
    newNode->next = NULL;
    return newNode;
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

// Creates a new list node with given value and returns 1 after inserting it.
int Insert(int value){
    struct list_node_s* newNode = create_list_node(value);
    struct list_node_s* tempNode = list;
    if(tempNode == NULL){
        tempNode = newNode;
        return 1;
    }
    // Iterate over nodes
    while(tempNode->next != NULL){
        tempNode = tempNode->next;
    }
    // Last node
    tempNode->next = newNode;
    //!!!! What is return value?
    return 1;
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

void* get_task(){
    long my_rank;
    int* task_num = malloc(sizeof(int));
    int* task_type = malloc(sizeof(int));
    int* value = malloc(sizeof(int));
    printf("%p bu sayı sihirli başta null olmalı\n", task_type);
    // Pass local attributes to the task dequeue function
    Task_dequeue(my_rank,task_num,task_type,value);
    printf("%d bu sayı sihirli\n",*task_type);
    printf("%d bu sayı sihirli\n",*task_num);
    printf("%d bu sayı sihirli\n",*value);
    return 0;
}
void print_task_queue(){
    struct task_node_s* tempNode = queue->front;
    for (int i = 0; i < 5; i++)
    {
        printf("Task value: %d\n", tempNode->value);
        printf("Task task type: %d\n", tempNode->task_type);
        printf("Task task number: %d\n", tempNode->task_num);

        tempNode = tempNode->next;
    }
}
int main(){
    // Create task queue with given number of tasks
    Task_queue(5);
    print_task_queue();
    pthread_t id;
    write(1,"THREAD create'den ONCE\n",sizeof("THREAD create'den ONCE\n")); 
    pthread_create(&id,NULL,&get_task,NULL);
    write(1,"THREAD join'den ONCE\n",sizeof("THREAD join'den ONCE\n")); 
    pthread_join(id,NULL);
    return 0;
}