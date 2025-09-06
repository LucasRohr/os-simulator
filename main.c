#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"

// state and memory lists (listas encadeadas para cada estado)
struct PCB *_create, *_ready, *_running, *_finish, *_blocked, *_susBlocked, *_susReady;

int _memSize, _diskSize;
int _quantum; // quantum RoundRobin
int _time; // tempo do sistema
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
    _diskSize = 100; // fixed size, should be enough
    
    // read line, scan number of processes
    sscanf(buffp, "%99[^\n]%n", line, &count);
    buffp += count + 1; // advances line + \n
    sscanf(line, "%d", &_nprocs);
    
    /*debug*/
    //printf("Mem:%d Disk:%d Q:%d NProcs:%d\n", _memSize, _diskSize, _quantum, _nprocs);
    
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
    PCB* p = NULL;      // variavel auxiliar

    // se finalizamos todos os processos, terminamos a simulação
    while(nFinished < _nprocs) { 
        // Adiciona na fila de create todos os processos que possuem inicio nesse tempo
        while(nextT > -1 && _time == nextT) {
            // Cria novo bloco e lê valores dele
            p = PCB_new();   

            int params = sscanf(line, "%d %d %db%d", &nextT, &p->remaining_time, &p->block_moment, &p->block_time);

            // se não conseguimos ler valores para o block, reseta o block moment
            if (params < 4)
                p->block_moment = -1;

            Push(&_create, p);
            printf("%02d:P%d -> %s (%d)\n", _time, p->id, states[p->state], p->remaining_time);
            
            /*Debug*/
            //printf("P:Time:%d Life:%d BT:%d BA:%d\n", _time, p->remaining_time, p->block_moment, p->block_time);
            
            // avança a leitura
            if(sscanf(buffp, "%99[^\n]%n", line, &count) >= 1) {
                buffp += count + 1; // advances line + \n
                sscanf(line, "%d", &nextT);
            } else { // se terminamos de ler o arquivo, não temos mais próximo tempo de processo
                nextT = -1; 
            }
        }
        
        ///////////////
        // Implementar o processamento de cada estado (por enquanto está de qualquer jeito)
        //////////////
        
        // executando
        if((p = Pop(&_running)) != NULL) {
            Push(&_finish, p);
            p->state = FINISH;
            printf("%02d:P%d -> %s (%d)\n", _time, p->id, states[p->state], p->remaining_time);
        }
        
        // pronto
        if(_ready != NULL && _running == NULL) {
            p = Pop(&_ready);
            Push(&_running, p);
            p->state = RUN;
            printf("%02d:P%d -> %s (%d)\n", _time, p->id, states[p->state], p->remaining_time);
        }
        
        // criando
        while((p = Pop(&_create)) != NULL) {
            Push(&_ready, p);
            p->state = READY;
            printf("%02d:P%d -> %s (%d)\n", _time, p->id, states[p->state], p->remaining_time);
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