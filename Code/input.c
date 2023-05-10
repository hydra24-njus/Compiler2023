#include "input.h"



struct InterCodes_ *input(FILE *fp){
    char buf[1024];
    char arg[4][32];
    int n1,n2,n3;
    while(fgets(buf,sizeof(buf),fp)!=NULL){
        memset(arg,'\0',sizeof(char)*32*4);
        n1=0,n2=0,n3=0;
        if(strncmp(buf,"LABEL",5)==0){
            sscanf(buf,"LABEL label%d :",&n1);
            printf("LABEL label%d : \n",n1);
        }
        else if(strncmp(buf,"FUNCTION",8)==0){
            sscanf(buf,"FUNCTION %s :",arg[0]);
            printf("FUNCTION %s : \n",arg[0]);
        }
        else if(strncmp(buf,"GOTO",4)==0){
            sscanf(buf,"GOTO label%d",&n1);
            printf("GOTO label%d \n",n1);
        }
        else if(strncmp(buf,"IF",2)==0){
            sscanf(buf,"IF %s %s %s GOTO label%d",arg[0],arg[1],arg[2],&n1);
            printf("IF %s %s %s GOTO label%d \n",arg[0],arg[1],arg[2],n1);
        }
        else if(strncmp(buf,"RETURN",6)==0){
            sscanf(buf,"RETURN %s",arg[0]);
            printf("RETURN %s \n",arg[0]);
        }
        else if(strncmp(buf,"DEC",3)==0){
            sscanf(buf,"DEC %s %d",arg[0],&n1);
            printf("DEC %s %d \n",arg[0],n1);
        }
        else if(strncmp(buf,"ARG",3)==0){
            sscanf(buf,"ARG %s",arg[0]);
            printf("ARG %s \n",arg[0]);
        }
        else if(strncmp(buf,"PARAM",5)==0){
            sscanf(buf,"PARAM %s",arg[0]);
            printf("PARAM %s \n",arg[0]);
        }
        else if(strncmp(buf,"READ",4)==0){
            sscanf(buf,"READ %s",arg[0]);
            printf("READ %s \n",arg[0]);
        }
        else if(strncmp(buf,"WRITE",5)==0){
            sscanf(buf,"WRITE %s",arg[0]);
            printf("WRITE %s \n",arg[0]);
        }
        else{
            //这里是第一个元素是变量的情况
            sscanf(buf,"%s := %s %s %s",arg[0],arg[1],arg[2],arg[3]);
            if(strncmp(arg[1],"CALL",4)==0){
                printf("%s := CALL %s \n",arg[0],arg[2]);
            }
            else{
                if(arg[2][0]=='\0'){
                    printf("%s := %s \n",arg[0],arg[1]);
                }
                else{
                    printf("%s := %s %s %s \n",arg[0],arg[1],arg[2],arg[3]);
                }
            }
        }
    }
    return NULL;
}