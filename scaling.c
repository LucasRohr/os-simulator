#include "scaling.h"

static ScalingType current_scaling;

// Atribui algoritmo de escalonamento
void init_scaling(char algorithm_char) {
    switch (algorithm_char) {
        case 'i': current_scaling = FIFO; break;
        case 's': current_scaling = SJF; break;
        case 'r': current_scaling = SRT; break;
        default:  current_scaling = ROUND_ROBIN; break;
    }
}

// Seleciona processo pronto com base no algoritmo
PCB* select_process(PCB** ready_queue) {
    if (*ready_queue == NULL) return NULL;

    switch (current_scaling) {
        case FIFO:
        case ROUND_ROBIN:
            // Para FIFO e Round Robin, pega o primeiro da fila
            return Pop(ready_queue);

        case SJF:
            // Para Shortest Job First, encontra o com menor tempo de execucao na fila ready
            if ((*ready_queue)->next == NULL) {
                return Pop(ready_queue);
            }

            PCB* current = *ready_queue;
            PCB* prev = NULL;

            struct PCB* shortest = *ready_queue;
            PCB* prev_shortest = NULL;

            // Utiliza variaveis prev para religar a fila corretamente sem perder o ponteiro do processo que vem antes do menor
            while(current != NULL) {
                if (current->total_execution_time < shortest->total_execution_time) {
                    shortest = current;
                    prev_shortest = prev;
                }

                prev = current;
                current = current->next;
            }

            //Remove o processo mais curto da lista
            if (shortest == *ready_queue) {\
                *ready_queue = shortest->next;
            } else {
                // Se estiver no meio ou no fim, refaz a ligação
                prev_shortest->next = shortest->next;
            }

            shortest->next = NULL;
            return shortest;

        case SRT:
            // Para Shortest Remaining Time, encontra o com menor tempo restante na fila ready
            if ((*ready_queue)->next == NULL) {
                return Pop(ready_queue);
            }

            PCB* current_srt = *ready_queue;
            PCB* prev_srt = NULL;

            struct PCB* shortest_srt = *ready_queue;
            PCB* prev_shortest_srt = NULL;

            // Utiliza variaveis prev para religar a fila corretamente sem perder o ponteiro do processo que vem antes do menor
            while(current_srt != NULL) {
                if (current_srt->remaining_time < shortest_srt->remaining_time) {
                    shortest_srt = current_srt;
                    prev_shortest_srt = prev_srt;
                }

                prev_srt = current_srt;
                current_srt = current_srt->next;
            }

            //Remove o processo mais curto da lista
            if (shortest_srt == *ready_queue) {\
                *ready_queue = shortest_srt->next;
            } else {
                // Se estiver no meio ou no fim, refaz a ligação
                prev_shortest_srt->next = shortest_srt->next;
            }

            shortest_srt->next = NULL;
            return shortest_srt;
    }

    return NULL;
}


// Confere se há preempção e qual lógica usar conforme algoritmo
PCB* check_preemption(PCB *running_process, PCB** ready_queue, int* quantum_timer, int quantum) {
    if (running_process == NULL) {
        return NULL;
    }

    switch (current_scaling) {
        case ROUND_ROBIN:
            // Para RR, há preempção caso estourar o quantum
            if (*quantum_timer == quantum) {
                *quantum_timer = 0;

                return running_process;
            }

            return NULL;

        case SRT:
            // Para Shortest Remaining Time, verifica se há um remaining time menor na fila de ready
            if (*ready_queue == NULL) {
                return NULL;
            }

            PCB* current = *ready_queue;
            PCB* shortest = *ready_queue;

            while(current != NULL) {
                if (current->remaining_time < shortest->remaining_time) {
                    shortest = current;
                }

                current = current->next;
            }

            // Se o da fila de ready for mais curto que o que está executando, há preempção
            if (shortest->remaining_time < running_process->remaining_time) {
                return running_process;
            }

            return NULL;

        case FIFO:
        case SJF:
            // Algoritmos não-preemptivos nunca sofrem preempção
            return NULL;
    }

    return NULL;
}
