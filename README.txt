O trabalho foi organizado da seguinte maneira:

- main.c: arquivo principal de execução
- process.h: arquivo de cabeçalho para struct base e operações de lista
- process.c: arquivo de implementação do process.h
- scaling.h: arquivo de cabeçalho para enum de tipos de escalonamento e funções auxiliaries para algoritmos de escalonamento
- scaling.c: arquivo de implementação do scaling.h

= Compilando e executando =

Exemplo com gcc:

gcc main.c process.c scaling.c -o simulator
./simulator "2 5|2|0 10|0 10"