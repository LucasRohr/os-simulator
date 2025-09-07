#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"
#include "scaling.h"

// state and memory lists (listas encadeadas para cada estado)
struct PCB *_create, *_ready, *_running, *_finish, *_blocked, *_susBlocked, *_susReady;

int _memSize, _diskSize;
int _quantum = -1; // quantum RoundRobin
int _time; // tempo do sistema
char _scaling_type = 'd'; // char para algoritmo de escalonamento, com valor default 'd'
int _nprocs;
int _pcount;

// default input no caso de não receber nada
const char *default_input = "1 3\n2\n0 5 1b2\n0 6";

///////////////////////
// MAIN
//////////////////////
int main(int argc, char *argv[]) 
{
    // read in the command line using like this
    //     "1 3|2|0 5 1b2|0 6"
    // else is using default string
    const char *input;
    if (argc > 1) input = argv[1];
    else          input = default_input;
    
    // Initialize reading variables
    char *buffer = malloc(strlen(input) + 1);
    strcpy(buffer, input);
    int count = 0;
    char line [strlen(input) + 1];
    char* buffp = buffer;
    
    // truque pra melhorar escrita por input
    for(int i = 0; i < strlen(input); i++)
        if(buffer[i] == '|')
            buffer[i] = '\n';
    
    // reads line, scan memmory size and quantum
    sscanf(buffp, "%99[^\n]%n", line, &count);
    buffp += count + 1; // advances line + \n
    
    sscanf(line, "%d %d", &_memSize, &_quantum);

    // If the quantum reading is null, then it reads the scaling algorithm
    if (_quantum == -1) {
        sscanf(line, "%d %c", &_memSize, &_scaling_type);
        init_scaling(_scaling_type);
    }

    _diskSize = 100; // fixed size, should be enough
    
    // read line, scan number of processes
    sscanf(buffp, "%99[^\n]%n", line, &count);
    buffp += count + 1; // advances line + \n
    sscanf(line, "%d", &_nprocs);
    
    /*debug*/
    // printf("Mem:%d Disk:%d Quantum:%d Scaling:%c NProcs:%d\n", _memSize, _diskSize, _quantum, _scaling_type, _nprocs);
    
    // Initialize queues with empty
    _create = _ready = _running = _finish = _blocked = _susBlocked = _susReady = NULL;    
    
    // read first process time
    sscanf(buffp, "%99[^\n]%n", line, &count);
    buffp += count + 1; // advances line + \n
    
    int nextT = -1;
    sscanf(line, "%d", &nextT);
    
    // Começa simulação
    _time = 0;          // reseta tempo do SO
    _pcount = 0;        // reseta contador de processos

    int nFinished = 0;  // reseta número de terminados
    int memory_process_total = 0; // Count para processos na memoria
    int quantum_timer = 0; // Timer para quantum de processo
    PCB* p = NULL;      // variavel auxiliar

    // se finalizamos todos os processos, terminamos a simulação
    while(nFinished < _nprocs) {
        // Adiciona na fila de create todos os processos que possuem inicio nesse tempo
        while(nextT > -1 && _time == nextT) {
            // Cria novo bloco e lê valores dele
            p = PCB_new(++_pcount);

            int arrival_time; // Variavel auxiliar para nao usar nextT
            int params = sscanf(line, "%d %d %db%d", &arrival_time, &p->remaining_time, &p->block_moment, &p->block_time);

            p->arrival_time = arrival_time;
            p->total_execution_time = p->remaining_time;

            // se não conseguimos ler valores para o block, reseta o block moment
            if (params < 4)
                p->block_moment = -1;

            Push(&_create, p);
            printf("%02d:P%d -> %s (%d)\n", _time, p->id, states[p->state], p->remaining_time);
            
            // avança a leitura
            if(sscanf(buffp, "%99[^\n]%n", line, &count) >= 1) {
                buffp += count + 1; // advances line + \n
                sscanf(line, "%d", &nextT);
            } else { // se terminamos de ler o arquivo, não temos mais próximo tempo de processo
                nextT = -1; 
            }
        }
        
        ///////////////
        // Implementacao do processamento de cada estado
        //////////////

        // Processo executando (running)
        if(_running != NULL) {
            _running->remaining_time--;  // Decrementa o tempo de vida
            _running->cpu_time_executed++;
            quantum_timer++;             // Incrementa o tempo que processo tá na CPU

            // Se processo terminou, tira da fila de running e coloca na de finished
            if (_running->remaining_time <= 0) {
                PCB* p = Pop(&_running);
                p->state = FINISH;

                Push(&_finish, p);

                printf("%02d:P%d -> %s (%d)\n", _time, p->id, states[p->state], p->remaining_time);

                memory_process_total--; // Libera um espaço na memória
                quantum_timer = 0; // Reseta quantum
            } else if (_running->block_moment != -1 && (_running->cpu_time_executed == _running->block_moment)) { // Se bloqueou por block_moment, bota na fila de blocked
                PCB* p = Pop(&_running);
                p->state = BLOCK;

                Push(&_blocked, p);

                printf("%02d:P%d -> %s (%d)\n", _time, p->id, states[p->state], p->remaining_time);

                quantum_timer = 0; // Reseta quantum
            }

            // Call for preemption check based on scaling algorithm
            PCB* process = check_preemption(_running, &_ready, &quantum_timer, _quantum);

            if (process != NULL) {
                Pop(&_running);

                process->state = READY;

                Push(&_ready, process); // Volta para o fim da fila ready, dando lugar ao proximo processo
                printf("%02d:P%d -> %s (%d)\n", _time, process->id, states[process->state], process->remaining_time);

                quantum_timer = 0; // Reseta o timer do quantum
            }
        }

        // Processo bloqueado (blocked)
        if (_blocked != NULL) {
            PCB *current = _blocked;
            PCB *prev = NULL;

            while (current != NULL) {
                current->block_time--;

                if (current->block_time <= 0) { // Se processo bloqueado está liberado
                    PCB* unblocked_process = current;
                    
                    // Remove da lista de bloqueados
                    if (prev == NULL) {
                        _blocked = current->next;
                    } else {
                        prev->next = current->next;
                    }
                    current = current->next;

                    // Adiciona na fila de prontos (ready)
                    unblocked_process->state = READY;
                    Push(&_ready, unblocked_process);

                    printf("%02d:P%d -> %s (%d)\n", _time, unblocked_process->id, states[unblocked_process->state], unblocked_process->remaining_time);
                } else {
                    // Continua para o próximo da lista
                    prev = current;
                    current = current->next;
                }
            }
        }

        // Verifica processos suspensos E bloqueados, para mover para suspenso/pronto caso tenha desbloqueio
        if (_susBlocked != NULL) {
            PCB *current = _susBlocked;
            PCB *prev = NULL;

            while (current != NULL) {
                current->block_time--;

                if (current->block_time <= 0) {
                    PCB* unblocked_process = current;

                    // Remove da lista de bloqueados/suspensos
                    if (prev == NULL) {
                        _susBlocked = current->next;
                    } else {
                        prev->next = current->next;
                    }
                    current = current->next; // Avança o ponteiro

                    // Move para a fila de PRONTOS/SUSPENSOS (_susReady)
                    unblocked_process->state = SUS_READY;
                    Push(&_susReady, unblocked_process);

                    printf("%02d:P%d -> %s (%d)\n", _time, unblocked_process->id, states[unblocked_process->state], unblocked_process->remaining_time);
                } else {
                    prev = current;
                    current = current->next;
                }
            }
        }

        // Se a memória está cheia e tem processos esperando para entrar
        if (memory_process_total >= _memSize && (_create != NULL || _susReady != NULL)) {
            // Se tiver processo bloqueado
            if (_blocked != NULL) {
                PCB* p = Pop(&_blocked);
                p->state = SUS_BLOCK;
                Push(&_susBlocked, p); // Adiciona bloqueado na fila de suspensos pois memoria está cheia

                memory_process_total--; // Liberou memória

                printf("%02d:P%d -> %s (%d)\n", _time, p->id, states[p->state], p->remaining_time);
            }
        }

        // Checa a fila de criados para admitir novos processos como ready ou ready suspenso
        while (_create != NULL) {
            PCB* p = Pop(&_create);

            if (memory_process_total < _memSize) {
                p->state = READY;
                Push(&_ready, p);
                memory_process_total++;
            } else {
                p->state = SUS_READY;
                Push(&_susReady, p);
            }

            printf("%02d:P%d -> %s (%d)\n", _time, p->id, states[p->state], p->remaining_time);
        }

        // Checa por fila dos processos suspensos prontos caso a memoria possua slots para executar
        while (memory_process_total < _memSize && _susReady != NULL) {
            PCB* p = Pop(&_susReady);
            p->state = READY;

            Push(&_ready, p);

            memory_process_total++; 

            printf("%02d:P%d -> %s (%d)\n", _time, p->id, states[p->state], p->remaining_time);
        }
        
        // Se nao houver processo executando mas houver processo pronto, adiciona o processo na execucao
        if (_running == NULL && _ready != NULL) {
            PCB* next_process = select_process(&_ready);

            if (next_process) {
                next_process->state = RUN;

                Push(&_running, next_process);

                quantum_timer = 0;

                printf("%02d:P%d -> %s (%d)\n", _time, next_process->id, states[next_process->state], next_process->remaining_time);
            }
        }

        // finalizando
        while((p = Pop(&_finish)) != NULL) {
            free(p);
            nFinished++;
        }
        
        // Incrementa tempo
        _time++;
    }
    
 
    // cleanup
    free(buffer);
    return 0;
}