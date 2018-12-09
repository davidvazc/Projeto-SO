#ifndef central_h
#define central_h
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>


#define MAXLENGTH 200
#define SIZENAMES 10

#define MAXSIMULATIONSIZE 5000
#define MINDRONES 1
#define MAXATTEMPS 7072
#define MAXPROD 3
#define MQ_ID 1234
#define SHM_SIZE 1024
#define MAXREQ 1000



#define SEM_NAME "semaforo"


#define NAMED_PIPE "input_pipe"


typedef struct reserva {
    int val;
    struct reserva* next;
} Reserva;

struct coordendadas{
    int x,y;
};

struct xy_double{
    double x,y;
};

struct prod{
    char nome[MAXPROD][SIZENAMES];
    int nr[MAXPROD];
};

typedef struct armazens {
    char nome[SIZENAMES];
    struct coordendadas XY;
    struct prod Prod;
}Armazens;

typedef struct stats{
    int ENC_drones;
    int PROD_carreg;
    int ENC_entreg;
    int PROD_entreg;
    int tempo_medio;
}memStat;

typedef struct drone {
    pthread_t thread_id;
    int dID;
    struct xy_double local;
    bool ocupado;
    struct xy_double dest;
    int estado;
    int encomenda;
    int armazem;
    struct drone* proximo;
} Drone;

typedef struct encomenda{
    int ORDER_NO;
    char nome[SIZENAMES];
    struct prod produto;
    struct xy_double destino;
    int tempo_total;
}REQ;



typedef struct {
    long mtype;
    int id_REQ;
    int drone;
    bool gestor;
    int tempo;
    struct prod Prod;
    REQ encomenda;
} droneMQ;

void alteraDrone(int NewD);
void sigintC(int signum);
int movimentaArmazem (Drone *drones, struct armazens armazem[], int W, REQ enc);
void central(int max_x,int max_y,int D,int S,int Q,int Tempo,int W);
void sigint(int signum);
void Req_start(REQ enc, int x, int y, int n, char nome, int n1);
void *drone_start(void *thread);


#endif /* central_h */
