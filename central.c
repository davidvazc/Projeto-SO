#include "central.h"
#include "drone_movement.h"

sem_t *mutex;

time_t rawtime;
struct tm * timeinfo;
FILE *logC;

pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cv;

Drone *drones=NULL,*dronesaux = NULL;
REQ *encomenda;
long ORDER_NO;

int W;
int D;
int MQ;

struct stats *statis;
int SHM_w;
int unidade_tempo;

struct armazens *warehouse;
int SHM_e;

int fd;

sem_t *semWarehouse;
sem_t *pedidos_espera;
sem_t *repouso_drones;



void alteraDrone(int NewD);

//------------------------- CRIAÇÃO DO NAMED PIPE --------------------------------------------------

int termina=0;
int termina_drones=0;
void sigur(int signum){
    termina_drones=1;
    Drone * my_data = drones;
    while (my_data->proximo != NULL) {
        pthread_join(drones->thread_id, NULL);
        my_data = my_data->proximo;
    }
    
    termina=1;
    shmdt(warehouse);
    shmctl(SHM_w, IPC_RMID, NULL);
    shmctl(SHM_e, IPC_RMID, NULL);
    sem_close(mutex);
    
    close(fd);
}

//------------------------- CRIAÇÃO DO NAMED PIPE --------------------------------------------------

int valida_msg(char msg[10][SIZENAMES]){
    if(strcmp(msg[2], "prod:")!=0)
        return 1;
    if(strcmp(msg[3],"A")!=0&&strcmp(msg[3], "B")!=0&&strcmp(msg[3], "C")!=0&&strcmp(msg[3], "D")!=0&&strcmp(msg[3], "E")!=0&&strcmp(msg[3], "F")!=0&&strcmp(msg[3], "G")!=0&&strcmp(msg[3], "H")!=0&&strcmp(msg[3], "I")!=0&&strcmp(msg[3], "J")!=0&&strcmp(msg[3], "K")!=0)
        return 1;
    if(strcmp(msg[5], "to:")!=0)
        return 1;
    int x,y;
    x=atoi(msg[6]);
    y=atoi(msg[7]);
    if(x>MAXSIMULATIONSIZE||y>MAXSIMULATIONSIZE)
        return 1;
    return 0;
}

int pipeini(void){
    
    
    
    
    char *aux;
    char auxRead[10][SIZENAMES];
    char msg[MAXLENGTH];
    
    
    //    b=read (para ler o tamanho a ler
    long b=read(fd, msg, MAXLENGTH);
    
    if(b==0){
        return 0;
    }
    //    substituir o pelo numero de caracteres a ler
    else{
        aux=strtok(msg," ");
        if(strcmp(aux,"ORDER")==0){
            int i=0;
            while (aux != NULL){
                strncpy(auxRead[i], aux,SIZENAMES);
                if(i==2 || i==5){
                    aux = strtok (NULL, ", ");
                }
                else
                    aux = strtok (NULL, " ");
                i++;
            }
            if(valida_msg(auxRead)==0){
                
                char* str = "Prod_";
                char dest[SIZENAMES];
                strcpy( dest, str );
                strcat( dest, auxRead[3] );
                strncpy(encomenda[ORDER_NO].produto.nome[0],dest,SIZENAMES);
                strncpy(encomenda[ORDER_NO].nome,auxRead[1],SIZENAMES);
                encomenda[ORDER_NO].produto.nr[0]=atoi(auxRead[4]);
                encomenda[ORDER_NO].destino.x=atoi(auxRead[6]);
                encomenda[ORDER_NO].destino.y=atoi(auxRead[7]);
                encomenda[ORDER_NO].ORDER_NO=ORDER_NO;
                
                
                time ( &rawtime );
                timeinfo = localtime ( &rawtime );
                printf("%d:%d:%d Encomenda %s-%ld recebida pela Central\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec,encomenda[ORDER_NO].nome,ORDER_NO);
                
                sem_wait(mutex);
                FILE *logC = fopen("log.txt", "a");
                if (logC == NULL){
                    printf("Não foi possível abrir o ficheiro de log");
                    exit(EXIT_FAILURE);
                }
                fprintf(logC,"%d:%d:%d Encomenda %s-%ld recebida pela Central\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec,encomenda[ORDER_NO].nome,ORDER_NO);
                fclose(logC);
                sem_post(mutex);
                ORDER_NO++;
                return 0;
            }
            else{
                time ( &rawtime );
                timeinfo = localtime ( &rawtime );
                printf("%d:%d:%d Pedido recebido no pipe de forma errada!\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
                
                
                sem_wait(mutex);
                FILE *logC = fopen("log.txt", "a");
                if (logC == NULL){
                    printf("Não foi possível abrir o ficheiro de log");
                    exit(EXIT_FAILURE);
                }
                fprintf(logC,"%d:%d:%d Pedido recebido no pipe de forma errada!\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
                fclose(logC);
                sem_post(mutex);
                
            }
        } else if(strcmp(aux, "DRONE")==0){
            int i=0;
            while (aux != NULL){
                strncpy(auxRead[i], aux,SIZENAMES);
                aux = strtok (NULL," ");
                i++;
            }
            if(strcmp(auxRead[1], "SET")==0){
                
                D=atoi(auxRead[2]);
                
                alteraDrone(D);
                
            }
            else{
                time ( &rawtime );
                timeinfo = localtime ( &rawtime );
                printf("%d:%d:%d Pedido recebido no pipe de forma errada!\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
                
                
                sem_wait(mutex);
                FILE *logC = fopen("log.txt", "a");
                if (logC == NULL){
                    printf("Não foi possível abrir o ficheiro de log");
                    exit(EXIT_FAILURE);
                }
                fprintf(logC,"%d:%d:%d Pedido recebido no pipe de forma errada!\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
                fclose(logC);
                sem_post(mutex);
            }
        } else {
            time ( &rawtime );
            timeinfo = localtime ( &rawtime );
            printf("%d:%d:%d Pedido recebido no pipe de forma errada!\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
            
            
            
            sem_wait(mutex);
            FILE *logC = fopen("log.txt", "a");
            if (logC == NULL){
                printf("Não foi possível abrir o ficheiro de log");
                exit(EXIT_FAILURE);
            }
            fprintf(logC,"%d:%d:%d Pedido recebido no pipe de forma errada!\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
            fclose(logC);
            sem_post(mutex);
        }
    }
    return 0;
}


//-------------- DRONES -------------------------------------------------------------


//INICIO DOS DRONES

int movimenta_drones(double *ax, double *ay, double t_x, double t_y, int max_attempts,long id_enc){
    int i, result;
    for(i = 0; i< max_attempts; i++){
        result = move_towards(ax, ay, t_x, t_y);
        
        //        sleep(unidade_tempo);
        encomenda[id_enc].tempo_total+=1;
        if(result == 0){
            return 0;
        } else if(result < 0){
            return -1;
        }
    }
    return 0;
}

int proximidade_base(double ax, double ay,long drone){
    
    int base=1;
    int resultado=distance(ax, ay, 0, MAXSIMULATIONSIZE);
    if(distance(ax, ay, MAXSIMULATIONSIZE, MAXSIMULATIONSIZE)<resultado){
        base=2;
    }
    if(distance(ax, ay, MAXSIMULATIONSIZE,0)<resultado){
        base=3;
    }
    if(distance(ax, ay, 0,0)<resultado){
        base=4;
    }
    Drone * my_data = drones;
    while (my_data->proximo != NULL) {
        if(my_data->dID==drone){
            if(base==1){
                my_data->dest.x=0;
                my_data->dest.y=MAXSIMULATIONSIZE;
            } else if (base==2){
                my_data->dest.x=MAXSIMULATIONSIZE;
                my_data->dest.y=MAXSIMULATIONSIZE;
            } else if(base==3){
                my_data->dest.x=MAXSIMULATIONSIZE;
                my_data->dest.y=0;
            } else{
                my_data->dest.x=0;
                my_data->dest.y=0;
            }
            
            int i, result = 0;
            my_data->estado=false;
            for(i = 0; i< MAXATTEMPS; i++){
                result = move_towards(&my_data->local.x, &my_data->local.y, my_data->dest.x, my_data->dest.y);
                
                //                sleep(unidade_tempo);
                encomenda[my_data->encomenda].tempo_total+=1;
                if(my_data->estado==2){
                    return 2;
                }
                
                if(result == 0){
                    return 0;
                } else if(result < 0){
                    return -1;
                }
            }
        }
        my_data = my_data->proximo;
        
    }
    
    
    return 0;
}


void *drone_start(void *drone){
    
    //        FALTA SEMAFORO*******************
    
    Drone *my_data  = (Drone*)drone;
    droneMQ pedido;
    
    int result;
    int Arma;
    long aux_mq;
    
    
    
    while(!termina_drones){
        pthread_mutex_lock(&thread_mutex);
        switch(my_data->estado){
            case 1: // Repouso
                
                
                    pthread_cond_wait(&cv, &thread_mutex);
                
                
                
                
                break;
            case 2: // Deslocação para carregamento
                
                Arma=my_data->armazem-1;
                
                my_data->ocupado=true;
                //printf("2 o drone esta em (%f,%f) e vai para (%d,%d)\n",my_data->local.x,my_data->local.y,warehouse[Arma].XY.x,warehouse[Arma].XY.y);

                result=movimenta_drones(&my_data->local.x, &my_data->local.y, warehouse[Arma].XY.x, warehouse[Arma].XY.y, MAXATTEMPS,my_data->encomenda);
                //printf("2.1 o dorne está em (%f,%f)\n",my_data->local.x,my_data->local.y);
                if(result<0){
                    printf("\nERRO: O drone %ld não chegou ao seu destino!\n",my_data->dID);
                    printf("Ficou parado na posição (%f,%f).\n",my_data->local.x,my_data->local.y);
                    my_data->estado=1;
                }
                else{
                    
                    my_data->estado=3;
                }
                break;
            case 3: // Carregamento
                //printf("3\n");
                aux_mq=(long)my_data->armazem;
                pedido.mtype=aux_mq;
                pedido.id_REQ=my_data->encomenda;
                pedido.drone=my_data->dID;
                pedido.gestor=false;
                pedido.encomenda=encomenda[my_data->encomenda];
                msgsnd(MQ, &pedido, sizeof(droneMQ)-sizeof(long), 0);
                
                msgrcv(MQ, &pedido, sizeof(droneMQ)-sizeof(long), my_data->dID, 0);
                
                statis->PROD_carreg=statis->PROD_carreg+1;
                
                encomenda[my_data->encomenda].tempo_total=encomenda[my_data->encomenda].tempo_total+pedido.tempo;
                my_data->estado=4;
                break;
            case 4: // Deslocação para entrega
                
                result = movimenta_drones(&my_data->local.x, &my_data->local.y, encomenda[my_data->encomenda].destino.x, encomenda[my_data->encomenda].destino.y,MAXATTEMPS,my_data->encomenda);
                if(result<0){
                    printf("\nERRO: O drone %ld não chegou ao seu destino!\n",my_data->dID);
                    printf("Ficou parado na posição (%f,%f).\n",my_data->local.x,my_data->local.y);
                    my_data->estado=1;
                }
                else{
//                    printf("4:o dorne está em (%f,%f)\n",my_data->local.x,my_data->local.y);
                    statis->tempo_medio=encomenda[my_data->encomenda].tempo_total/my_data->encomenda;
                    my_data->estado=5;
                    my_data->ocupado=false;
                }
                break;
            case 5: // Retorno à base
                statis->ENC_entreg=statis->ENC_entreg+1;
                
                result=proximidade_base(my_data->local.x, my_data->local.y,my_data->dID);
                
                if(result<0){
                    printf("\nERRO: O drone %ld não chegou ao seu destino!\n",my_data->dID);
                    printf("Ficou parado na posição (%f,%f).\n",my_data->local.x,my_data->local.y);
                    my_data->estado=1;
                } else if (result == 2){
                    my_data->estado=2;
                }
                else{
//                    printf("5:o dorne está em (%f,%f)\n",my_data->local.x,my_data->local.y);
                    time ( &rawtime );
                    timeinfo = localtime ( &rawtime );
                    printf("%d:%d:%d Encomenda %s-%ld entregue no destino pelo drone %ld\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec,encomenda[my_data->encomenda].nome,my_data->encomenda,my_data->dID);
                    
                    sem_wait(mutex);
                    FILE *logC = fopen("log.txt", "a");
                    if (logC == NULL){
                        printf("Não foi possível abrir o ficheiro de log");
                        exit(EXIT_FAILURE);
                    }
                    fprintf(logC,"%d:%d:%d Encomenda %s-%ld entregue no destino pelo drone %ld\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec,encomenda[my_data->encomenda].nome,my_data->encomenda,my_data->dID);
                    fclose(logC);
                    sem_post(mutex);
                    
                    my_data->estado=1;
                    
                }
                break;
        }
        pthread_mutex_unlock(&thread_mutex);
        
        
    }
    pthread_exit(NULL);
    
    
    
    return NULL;
}


void threads(){
    Drone * current = drones;
    
    while (current != NULL) {
        pthread_create(&drones->thread_id, NULL, drone_start, current);
        current = current->proximo;
    }
}

//------------- ALTERA DRONE -------------------------------------------------------------

void alteraDrone(int NewD){
    int conta=1;
    Drone * current = drones;
    while (current->proximo != NULL) {
        current = current->proximo;
        conta++;
    }
    while(conta<NewD){
        current->proximo=(Drone*)malloc(sizeof(Drone));
        current=current->proximo;
        conta++;
    }
    //    opção de retirar tambem um drone
    int robin=0;
    for(int i=conta; i<NewD; i++){
        current->proximo=(Drone*)malloc(sizeof(Drone));
        current=current->proximo;
        current->dest.x=0;
        current->estado=1;
        current->ocupado = false;
        current->dest.x=0;
        current->dest.y=0;
        current->dID=i+W+1;
        if(robin==0){
            current->local.y = MAXSIMULATIONSIZE;
            current->local.x=0;
            robin++;
        } else if(robin==1){
            current->local.y = MAXSIMULATIONSIZE;
            current->local.x= MAXSIMULATIONSIZE;
            robin++;
        }   else if(robin==2){
            current->local.y = 0;
            current->local.x= MAXSIMULATIONSIZE;
            robin++;
        } else {
            current->local.y = 0;
            current->local.x= 0;
            robin=0;
        }
        current->proximo=NULL;
        pthread_create(&current->thread_id, NULL, drone_start, current);
    }
    D=NewD;
    
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    printf("%d:%d:%d Número de drones foi alterado para: %d\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec,NewD);
    
    
    sem_wait(mutex);
    FILE *logC = fopen("log.txt", "a");
    if (logC == NULL){
        printf("Não foi possível abrir o ficheiro de log");
        exit(EXIT_FAILURE);
    }
    fprintf(logC,"%d:%d:%d Número de drones foi alterado para: %d\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec,NewD);
    fclose(logC);
    sem_post(mutex);
    
}

//------------- ESCOLHA DO DRONE -------------------------------------------------------------
long escolheDrone(int W,long id_REQ){
    
    int rtd,verifica_stock=-1;
    int rtl;
    long droneId=-1;
    int drone_escolhido,armazem_escolhido=0,K=0;
    int distancia_total=MAXSIMULATIONSIZE*2;
    for(int i=0;i<W;i++){
        for(int k=0;k<MAXPROD;k++){
            if(strcmp(warehouse[i].Prod.nome[k],encomenda[id_REQ].produto.nome[0])==0){
                if(warehouse[i].Prod.nr[k]>=encomenda[id_REQ].produto.nr[0]){
                    verifica_stock=i;
                    
                    rtd=distance(warehouse[i].XY.x, warehouse[i].XY.y, encomenda[id_REQ].destino.x, encomenda[id_REQ].destino.y);
                    
                    drone_escolhido=0;
                    
                    Drone * current = drones;
                    while (current != NULL) {
                        if(current->ocupado==false){
                            
                            rtl=distance(warehouse[i].XY.x, warehouse[i].XY.y,current->local.x,current->local.y);
                            
                            int aux=rtd+rtl;
//                            printf("o drone %d vai comparar a distancia rtd=%d com rtl=%d, obtendo:%d\n",current->dID,rtd,rtl,aux);
                            if(aux<distancia_total){
                                
                                distancia_total=rtl+rtd;
                                droneId=current->dID;
                                armazem_escolhido=i+1;
                                K=k;
                                
//                                printf("DISTANCIA DO ARMAZEM %d com o drone %d : %d\n",armazem_escolhido,current->dID,distancia_total);
                            }
                        }
                        drone_escolhido++;
                        current=current->proximo;
                    }
                }
            }
        }
    }
    //não ha em stock
    if(verifica_stock==-1)
        return -1;
    //não ha drones
    if(droneId==-1)
        return -2;
    //    há drone e retira os produtos do armazem reservando-os:
    Drone * current = drones;
    while (current != NULL) {
        if(current->dID==droneId){
            current->estado=2;
            current->dest.x=warehouse[armazem_escolhido-1].XY.x;
            current->dest.x=warehouse[armazem_escolhido-1].XY.y;
            current->encomenda=id_REQ;
            current->armazem=armazem_escolhido;
            
            
            sem_wait(mutex);
            FILE *logC = fopen("log.txt", "a");
            if (logC == NULL){
                printf("Não foi possível abrir o ficheiro de log");
                exit(EXIT_FAILURE);
            }
            fprintf(logC,"%d:%d:%d Encomenda %s-%ld enviada ao drone %ld\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec,encomenda[id_REQ].nome,id_REQ,droneId);
            fclose(logC);
            sem_post(mutex);
            
            pthread_cond_broadcast(&cv);
        }
        current=current->proximo;
    }
    
    statis->ENC_drones=statis->ENC_drones+1;
    
    warehouse[armazem_escolhido].Prod.nr[K]=warehouse[armazem_escolhido].Prod.nr[K]-encomenda[id_REQ].produto.nr[0];
    return droneId;
}




//------------MAIN--------------------------------------------------------------

void central(int max_x,int max_y,int nr_drones,int S,int Q,int Tempo,int n_armazens){
    signal(SIGUSR1,sigur);
    unidade_tempo=Tempo;
    D=nr_drones;
    MQ=msgget(MQ_ID,0777);
    
    
    encomenda=malloc(sizeof(REQ)*MAXREQ);
    
    semWarehouse= sem_open("semArmazem",O_CREAT,0644,1);
    if(semWarehouse == SEM_FAILED){
        perror("Não foi possível criar semáforo");
        sem_unlink("semArmazem");
        exit(-1);
    }
    
    pedidos_espera= sem_open("semArmazem",O_CREAT,0644,1);
    if(pedidos_espera == SEM_FAILED){
        perror("Não foi possível criar semáforo");
        sem_unlink("semArmazem");
        exit(-1);
    }
    
    mutex = sem_open(SEM_NAME,0,0644,0);
    if(mutex == SEM_FAILED)
    {
        perror("reader:unable to execute semaphore");
        sem_close(mutex);
        exit(-1);
    }
    
    repouso_drones=sem_open("espera",O_CREAT,0644,1);
    if(repouso_drones == SEM_FAILED){
        perror("Não foi possível criar semáforo");
        sem_unlink("semArmazem");
        exit(-1);
    }
    W=n_armazens;
    
    //------------------ MAPEAR SHARED MEMORY ----------------------------------------------------------------------------------------------------------------------------------
    
    
    SHM_w = shmget(1234, (sizeof(struct armazens)*W), 0777);
    warehouse = shmat(SHM_w, NULL, 0);
    
    
    SHM_e = shmget(1235, (sizeof(struct stats)), 0777);
    statis = shmat(SHM_e, NULL, 0);
    
    //------------------ CRIAR DRONES ---------------------------------------------------------------------------------------------------------------------------------------
    
    
    int robin=1;
    for(int i=0; i<D; i++){
        if(drones==NULL){
            drones=(Drone*)malloc (1*sizeof(Drone));
            drones->local.x=0;
            drones->local.y = MAXSIMULATIONSIZE;
            drones->ocupado = false;
            drones->dest.x=0;
            drones->dest.y=0;
            drones->encomenda=0;
            drones->dID=i+W+1;
            drones->estado=1;
            drones->proximo=NULL;
            dronesaux=drones;
        }
        else{
            dronesaux->proximo=(Drone*)malloc (1*sizeof(Drone));
            dronesaux=dronesaux->proximo;
            dronesaux->dID=i+W+1;
            dronesaux->encomenda=0;
            if(robin==0){
                dronesaux->local.x=0;
                dronesaux->local.y=0;
                robin++;
            } else if(robin==1){
                dronesaux->local.x=MAXSIMULATIONSIZE;
                dronesaux->local.y=MAXSIMULATIONSIZE;
                robin++;
            } else if(robin==2){
                dronesaux->local.x=MAXSIMULATIONSIZE;
                dronesaux->local.y=0;
                robin++;
            } else{
                dronesaux->local.x=0;
                dronesaux->local.y=MAXSIMULATIONSIZE;
                robin=0;
            }
            dronesaux->ocupado = false;
            dronesaux->dest.x=0;
            dronesaux->dest.y=0;
            dronesaux->estado=1;
            dronesaux->proximo=NULL;
        }
    }
    threads();
    
    //------------------PIPE-----------------------------------------------------------------------------------------------------------------------------------
    long aux;
    long aux1;
    Reserva *reserva=NULL;
    Reserva *reserva_aux=NULL;
    
    
    unlink(NAMED_PIPE);
    if ((mkfifo(NAMED_PIPE, O_CREAT|O_EXCL|0600)<0) && (errno!= EEXIST)) {
        perror("Não foi possivel criar pipe.\n");
        exit(0);
    }
    
    if ((fd=open(NAMED_PIPE, O_RDWR)) < 0){
        perror("Erro ao abrir o pipe.\n");
    }
    
    ORDER_NO=1;
    while(!termina){
        
        aux=ORDER_NO;
        //        if(reserva!=NULL){
        //            Reserva * current = reserva;
        //            while (current != NULL) {
        //                aux1=escolheDrone(W,current->val);
        //                if(aux1==-1){
        //                } else if(aux1==-2){
        //                    printf("\nNão existem drones disponíveis!\n");
        //                    while(escolheDrone(W,current->val)==aux1){
        //                        sleep(1);
        //                    }
        //                    printf("\nNova encomendas %d, o drone escolhido é o %ld\n",ORDER_NO,aux1);
        //                    sem_post(pedidos_espera);
        //                } else{
        //                    printf("\nNova encomendas %d, o drone escolhido é o %ld\n",ORDER_NO,aux1);
        //                    sem_post(pedidos_espera);
        //                }
        //
        //                current = current->next;
        //            }
        //        }
        
        pipeini();
        
        if(aux!=ORDER_NO){
            for(long i=aux;i<ORDER_NO;i++){
                
                aux1=escolheDrone(W,i);
                if(aux1==-1){
                    
                    time ( &rawtime );
                    timeinfo = localtime ( &rawtime );
                    printf("%d:%d:%d Encomenda %s-%ld suspensa por falta de stock\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec,encomenda[i].nome,i);
                    
                    sem_wait(mutex);
                    FILE *logC = fopen("log.txt", "a");
                    if (logC == NULL){
                        printf("Não foi possível abrir o ficheiro de log");
                        exit(EXIT_FAILURE);
                    }
                    fprintf(logC,"%d:%d:%d Encomenda %s-%ld suspensa por falta de stock\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec,encomenda[i].nome,i);
                    fclose(logC);
                    sem_post(mutex);
                    
                    if(reserva==NULL){
                        reserva =(Reserva*)malloc(sizeof(Reserva));
                        reserva->val = i;
                        reserva->next = NULL;
                        reserva_aux=reserva;
                    }
                    else{
                        reserva_aux->next=(Reserva*)malloc(sizeof(Reserva));
                        reserva_aux=reserva_aux->next;
                        reserva_aux->val=i;
                        reserva_aux->next=NULL;
                    }
                } else if(aux1==-2){
                    printf("\nNão existem drones disponíveis!\n");
                    while(escolheDrone(W,i)==aux1){
                        
                        sleep(1);
                        encomenda[i].tempo_total+=1;
                    }
                    time ( &rawtime );
                    timeinfo = localtime ( &rawtime );
                    printf("%d:%d:%d Encomenda %s-%ld enviada ao drone %ld\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec,encomenda[i].nome,i,aux1);
                } else{
                    time ( &rawtime );
                    timeinfo = localtime ( &rawtime );
                    printf("%d:%d:%d Encomenda %s-%ld enviada ao drone %ld\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec,encomenda[i].nome,i,aux1);
                }
            }
        }
    }
    exit(0);
    
}



