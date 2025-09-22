// Nome: Lucas Rohr Carreño
// Esse arquivo na verdade é um .h, tive que botar como .txt no final porque no Moodle não aceitava .h
// Favor mudar para .h ao executar trabalho, obrigado!

#ifndef SCALING_H
#define SCALING_H

#include "process.h"

typedef enum {
    FIFO, SJF, SRT, ROUND_ROBIN
} ScalingType;

void init_scaling(char scaling_type);

PCB* select_process(PCB** ready_queue);

PCB* check_preemption(PCB *running_process, PCB** ready_queue, int* quantum_timer, int quantum);

#endif
