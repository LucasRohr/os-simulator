#include "process.h"

// Definição do array de strings dos estados
const char* states[] = {"created","ready","executing","finished","blocked","suspended, blocked","suspended, ready"}; 

// PCB methods Implementation

// Funciona como um "construtor" para alocar memoria e inicializar novo PCB
struct PCB* PCB_new(int id){
    struct PCB* p = malloc(sizeof(PCB));
    p->id = id;
    p->state = CREATE;
    p->next = NULL;
    p->remaining_time = 0;
    p->cpu_time_executed = 0;
    p->block_moment = -1;
    p->block_time = 0;
    return p;
}

// Adiciona PCB no final de uma fila
void Push(PCB **queue, PCB *item) {
    if(item == NULL)
        return;

    // Garante que o novo item sempre será o último e não apontará para lixo
    item->next = NULL;
        
    if(*queue == NULL) {
        *queue = item;
        return;
    }

    // Se a fila não está vazia, usa um ponteiro temp para encontrar o fim
    PCB *current = *queue;

    while (current->next != NULL) {
        current = current->next;
    }
    
    // Adiciona o novo item no final da lista
    current->next = item;
}

// Remove e retorna o primeiro PCB de uma fila
struct PCB* Pop(PCB **queue) {
    if(*queue != NULL) {            // se a fila tem ao menos um elemento
        struct PCB* aux = *queue;   // pega o topo
        *queue = aux->next;         // define o topo como o próximo
        
        aux->next = NULL;           // como topo está saindo da fila, limpa a referência
        return aux;
    }
        
    return NULL;    // se chegou aqui é porque a fila está vazia
}
