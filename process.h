#ifndef PROCESS_H
#define PROCESS_H

#include <stdio.h>
#include <stdlib.h>

// criando(C), pronto(P), executando(E), finalizado(F), bloqueado(B), 
// bloqueado suspenso(b), pronto suspenso(p)
enum States {
    CREATE,    // 0
    READY,     // 1
    RUN,       // 2
    FINISH,    // 3
    BLOCK,     // 4
    SUS_BLOCK, // 5
    SUS_READY  // 6
};

// Mapeamento amigavel dos estados do enum para mostrar em tela
extern const char* states[]; 

// PCB (Process Controle Block)
typedef struct PCB {
    int id;
    char state; // usa o enum de estados
    int remaining_time;
    int cpu_time_executed;
    int total_execution_time; // SJF
    int arrival_time;         // FIFO
    int block_moment;
    int block_time;
    struct PCB *next; // ponteiro para proximo PCB
} PCB;

// Funções de manipulação do PCB e das filas
struct PCB* PCB_new(int id);
struct PCB* Pop(PCB **queue);
void Push(PCB **queue, PCB *item);

#endif