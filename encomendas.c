#include "central.h"

#include <stdio.h>

int pipefd1;

void pipeencomenda(){
    if ((mkfifo(NAMED_PIPE, O_CREAT|O_EXCL|0600)<0) && (errno!= EEXIST)) {
        perror("Não foi possivel criar pipe: ");
        exit(0);
    }
    char msg[MAXLENGTH],msg2[MAXLENGTH];
    while(1){
        if ((pipefd1 = open(NAMED_PIPE, O_RDONLY)) < 0) {
            perror("Não foi possível abrir pipe para leitura: ");
            exit(0);
        }
        else{
            printf("\n");
        }
        fgets(msg,80,stdin);
        
        write(pipefd1,msg,strlen(msg)+1);
        
        close(pipefd1);
        
        if ((pipefd1 = open(NAMED_PIPE,O_WRONLY))<0){
            perror("error opening pipe");
        }
        else{
            printf("\n");
        }
        read(pipefd1,msg2,sizeof(msg2));
        printf("server : %s\n",msg2);
        close(pipefd1);
    }
}



//int main(int argc, char const *argv[]) {
//
//    pipeencomenda();
//
//    return 0;
//}
