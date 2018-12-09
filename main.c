#include "central.h"
#include <signal.h>

int done=0;
int MQm,centralPID;
int SHM_wm;
int SHM_em;

memStat* esta;
Armazens* shm;


sem_t *mutex;

time_t rawtime;
struct tm * timeinfo;

void sigint(int signum){
    //    wait(NULL);
    
    
    done=1;
    msgctl(MQm,IPC_RMID,NULL);
}

void my_handler(int signum){
    //para funcionar enviar noutra consola kill -USR1 pid
    
    printf("\nNúmero total de encomendas atribuidas aos drones:%d\n",esta->ENC_drones);
    printf("Número total de produtos carregados de armazem:%d\n",esta->PROD_carreg);
    printf("Número total de encomendas entregues:%d\n",esta->ENC_entreg);
    printf("Número total de produtos entregues:%d\n",esta->PROD_entreg);
    printf("Tempo médio para conclusão de uma encomenda:%d\n\n",esta->tempo_medio);
    
}

void criaArmazens(long i, int tempo){
    droneMQ pedido;
    
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    printf("Processo do Armazém %ld criado com o PID: %i\n",i, getpid());
    sem_wait(mutex);
    FILE *log = fopen("log.txt", "a");
    if (log == NULL){
        printf("Não foi possível abrir o ficheiro de log");
        exit(EXIT_FAILURE);
    }

    fprintf(log,"%d:%d:%d Processo do Armazém %ld criado com o PID: %i\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec,i, getpid());
    fclose(log);
    sem_post(mutex);
    
    long aux;
    
    
    while(!done){
        msgrcv(MQm, &pedido, sizeof(droneMQ)-sizeof(long), i, 0);
        
        if(pedido.gestor==false){
            
            aux=(long)pedido.drone;
            
            pedido.mtype=aux;
            int processo=tempo*(sizeof(pedido.encomenda.produto.nr)/sizeof(pedido.encomenda.produto.nr[0]));
            pedido.tempo=processo;
            
            sleep(processo);
            
            msgsnd(MQm, &pedido, sizeof(droneMQ)-sizeof(long), 0);
            
            
        } else{
            //função que recebe as encomendas
            time ( &rawtime );
            timeinfo = localtime ( &rawtime );
            printf("%d:%d:%d Armazém %ld recebeu novo stock\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec,i);
            sem_wait(mutex);
            log = fopen("log.txt", "a");
            if (log == NULL){
                printf("Não foi possível abrir o ficheiro de log");
                exit(EXIT_FAILURE);
            }
            fprintf(log,"%d:%d:%d Armazém %ld recebeu novo stock\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec,i);
            fclose(log);
            sem_post(mutex);
            
            
            
            //ATUALIZAR O STOCK EM FUNÇÃO DA MENSAGEM
            pedido.mtype=MAXREQ*2;
            msgsnd(MQm, &pedido, sizeof(droneMQ)-sizeof(long), 0);
        }
        
        
    }
    //    fclose(log);
    exit(0);
}



int main(int argc, char const *argv[]){
    
    signal(SIGQUIT,SIG_IGN);
    signal(SIGTSTP,SIG_IGN);
    signal(SIGTERM,SIG_IGN);
    signal(SIGSTOP,SIG_IGN);
    signal(SIGABRT,SIG_IGN);
    signal(SIGINT,sigint);
    //
    
    //------------------ CRIAR LOG -----------------------------------------------------------------------------------------------------------------------------------------
    /* create, initialize semaphore */
    
    unlink(SEM_NAME);
    mutex = sem_open(SEM_NAME,O_CREAT,0644,1);
    if(mutex == SEM_FAILED){
        perror("Não foi possível criar semáforo");
        sem_unlink(SEM_NAME);
        exit(-1);
    }
    
    
    sem_wait(mutex);
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    FILE *log = fopen("log.txt", "a");
    if (log == NULL){
        printf("Não foi possível criar ficheiro de log");
        exit(EXIT_FAILURE);
    }
    
    fprintf(log,"%d:%d:%d Simulação iniciada!\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
    
    printf("%d:%d:%d Simulação iniciada!\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
    fclose(log);
    sem_post(mutex);
    //-----------------------------------LER FICHEIRO CONFIG E ATRIBUIR VARIAVEIS---------------------------------------------------------------------------------------------
    int max_x,max_y,D,S,Q,Tempo,W;
    char aux[MAXLENGTH];
    char tiposProd[MAXPROD][SIZENAMES];
    int quantidade=0;
    int i =0;
    
    FILE *fp;
    fp=fopen("config.txt", "r");
    if (fp == NULL){
        printf("Não foi possível abrir a configuração!!\n");
        exit(EXIT_FAILURE);
    }
    
    fscanf(fp,"%d %*s %d\n",&max_x,&max_y);            // Comprimento maximo (max_x), Altura maxima (max_y)
    
    if (max_x<=0 || max_y<=0 || MAXSIMULATIONSIZE<max_x || MAXSIMULATIONSIZE<max_y){
        printf("%d %d\n",max_x,max_y);
        printf("Dimensões da simulação incorretas!!\n");
        exit(EXIT_FAILURE);
    }
    printf("%d %d\n",max_x,max_y);
    fscanf(fp,"%[^\n]s\n",aux);
    
    char *prod;
    prod = strtok (aux,", ");
    
    while (prod != NULL){
        strncpy(tiposProd[i], prod,SIZENAMES);
        quantidade++;
        i++;
        prod = strtok (NULL, ", ");
        printf("%s, ",tiposProd[i-1]);
    }
    
    fscanf(fp,"%d\n",&D);                           // Número de drones no sistema (D)
    if (D<MINDRONES){
        printf("%d\n",D);
        printf("Número de drones incorreto!!\n");
        exit(EXIT_FAILURE);
    }
    printf("\n%d",D);
    fscanf(fp,"%d %*s %d %*s %d\n\n",&S,&Q,&Tempo);       //Frequencia de Abastecimento (S, em unidades de tempo)
    if(S<0 ||Q<0||Tempo<0){
        printf("Unidades de tempo de arranque incorretas!!\n");
        exit(EXIT_FAILURE);
    }
    printf("\n%d, %d, %d\n",S,Q,Tempo);             //quantidade (Q), unidade de Tempo (em segundos)
    fscanf(fp,"%d",&W);
    if(W<1){
        printf("Numero de armazens incorreto!!\n");
        exit(EXIT_FAILURE);
    }
    printf("\n%d",W);                               //Número de armazéns no sistema (W)
    
    //-----------------------------------LER FICHEIRO CONFIG E ATRIBUIR VARIAVEIS------------------------------------------------------------------------------------------------
    
    struct armazens armazem[W];
    
    for(int i=0;i<W;i++){
        fscanf(fp,"%s %*s %d %*s %d %*s %[^\n]",armazem[i].nome,&armazem[i].XY.x,&armazem[i].XY.y,aux);
        if(armazem[i].XY.x<0 || MAXSIMULATIONSIZE<armazem[i].XY.x || armazem[i].XY.y<0 || MAXSIMULATIONSIZE<armazem[i].XY.y){
            printf("Coordenadas do armazem %s fora do limite da simulação!!\n",armazem[i].nome);
            exit(EXIT_FAILURE);
        }
        printf("\n%s xy: %d, %d prod: ",armazem[i].nome,armazem[i].XY.x,armazem[i].XY.y);
        prod = strtok (aux,", ");
        int j =0;
        while (prod != NULL){
            strncpy(armazem[i].Prod.nome[j], prod,SIZENAMES);
            j++;
            if(j>MAXPROD){
                printf("\nNumero de produtos inicial do armazem %s superior ao permitido!!\n",armazem[i].nome);
                exit(EXIT_FAILURE);
            }
            prod = strtok (NULL, ", ");
            armazem[i].Prod.nr[j-1]=atoi(prod);
            prod = strtok (NULL, ", ");
            printf("%s, %d, ",armazem[i].Prod.nome[j-1],armazem[i].Prod.nr[j-1]);
            
        }
        
    }
    printf("\n\n");
    fclose(fp);
    
    //------------------ CRIAR MQ ---------------------------------------------------------------------------------------------------------------------------------------
    
    MQm=msgget(MQ_ID,IPC_CREAT|0777);
    if (MQm < 0){
        perror("ERRO: Ao criar a message queue\n");
        exit(0);
    }
    
    
    //------------------ CRIAR SHARED MEMORY ---------------------------------------------------------------------------------------------------------------------------------------
    
    
    SHM_wm = shmget(1234, (sizeof(struct armazens)*W), 0777|IPC_CREAT);
    shm = shmat(SHM_wm, NULL, 0);
    for(i=0;i<W;i++){
        shm[i] = armazem[i];
    }
    
    
    
    struct stats inic;
    inic.ENC_drones=0;
    inic.ENC_entreg=0;
    inic.PROD_carreg=0;
    inic.PROD_entreg=0;
    inic.tempo_medio=0;
    
    
    SHM_em = shmget(1235, (sizeof(struct stats)), 0777|IPC_CREAT);
    esta=shmat(SHM_em, NULL, 0);
    esta[0]=inic;
    
    
    //------------------ CRIA PROCESSOS --------------------------------------------------------------------------------------------------------------------------------------------
    //Processos armazens
    pid_t pids[W];
    
    for (int i = 1; i < W+1; i++) {
        fflush(log);
        fflush(stdout);
        if (fork() == 0) {
            pids[i]=getpid();
            time ( &rawtime );
            timeinfo = localtime ( &rawtime );
            
            criaArmazens(i,Tempo);
            exit(0);
        }
    }
    
    //Processo central
    fflush(log);
    fflush(stdout);
    if (fork() == 0) {
        centralPID=getpid();
        printf("Processo central criado com sucesso com PID: %i\n",getpid());
        central(max_x,max_y,D,S,Q,Tempo,W);
        exit(0);
    }
    
    signal(SIGUSR1,my_handler);
    
    
    //------------------TERMINA OS PROCESSOS-----------------------------------------------------------------------------------------------------------------------------------
    long robin=1;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    while(!done){
        sleep(S*Tempo);
        
        droneMQ pedido;
        int numero=rand() % (quantidade + 1);
        strncpy(tiposProd[numero], pedido.Prod.nome[0],SIZENAMES);
        pedido.gestor=true;
        if(robin==W+1)
            robin=1;
        pedido.mtype=robin;
        
        
        pedido.Prod.nr[0]=Q;
        msgsnd(MQm, &pedido, sizeof(droneMQ)-sizeof(long), 0);
        msgrcv(MQm, &pedido, sizeof(droneMQ)-sizeof(long),MAXREQ*2, 0);
        esta->PROD_entreg=esta->PROD_entreg+Q;
        robin++;
    }
    kill(centralPID,SIGUSR1);
    
    for(i=0;i<W;i++){
        wait(NULL);
    }
    
    
    shmdt(shm);
    shmctl(SHM_wm, IPC_RMID, NULL);
    shmdt(stat);
    shmctl(SHM_em, IPC_RMID, NULL);
    
    log = fopen("log.txt", "a");
    if (log == NULL){
        printf("Não foi possível criar ficheiro de log");
        exit(EXIT_FAILURE);
    }
    for(i=1;i<=W;i++){
        time ( &rawtime );
        timeinfo = localtime ( &rawtime );
        
        fprintf(log,"%d:%d:%d Processo do Armazém %d terminado\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec,i);
        
        printf("%d:%d:%d Processo do Armazém %d terminado\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec,i);
        kill(pids[i],SIGUSR1);
    }
    
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
//    sem_wait(mutex);
    fprintf(log,"%d:%d:%d Simulação terminada!\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
//    sem_post(mutex);
    fclose(log);
    
    printf("\n%d:%d:%d Simulação terminada!\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
    
}
