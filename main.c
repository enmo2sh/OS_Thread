#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#define MAXCHAR 1000
#define MAXValues 1000

struct matrix {                         // contains number of rows,columns and the values stored in matrix
    int values[MAXValues][MAXValues];
    int row;
    int col;
}m1,m2,output;

struct threadParam {                    // contains the current row from matrix 1 and col from matrix 2 to multiply
    int r;
    int c;
};

struct matrix readFromFiles(char *filename){
    struct matrix mtx;
    mtx.row=0;     //initialize them to check the wrong input
    mtx.col=0;
    FILE *fp;
    char str[MAXCHAR];
    fp = fopen(filename, "r");
    if (fp == NULL){
        printf("Could not open file %s\n",filename);
        return mtx;       // should handle it
    }
    if(fgets(str,MAXCHAR,fp)!= NULL){   // empty file
        int value,i;
        if(str[0]=='r' && str[1]=='o' && str[2]=='w' && str[3]=='='){  //to get the number of rows
            value =0;
            i=4;
            while(str[i]!=' '){
                value *=10;
                value =value +(str[i]-'0');
                i++;
            }
            mtx.row=value;
            if(str[i+1]=='c' && str[i+2]=='o' && str[i+3]=='l' && str[i+4]=='='){  //to get the number of cols
                value=0;
                i=i+5;
                while(i<(strlen(str)-1)){
                    value*=10;
                    value =value +(str[i]-'0');
                    i++;
                }
                mtx.col=value;
                for (int r=0;r<mtx.row;r++){   //start reading the values
                    if(fgets(str, MAXCHAR, fp) != NULL){
                        int negative=0;    // 0-> positive int   1->negative int
                        int c=0;
                        value=0;
                        i=0;
                        while(i<(strlen(str)-1)){
                            if(str[i]!='\t'){
                                if(str[i]=='-')  // to handle negative numbers
                                    negative=1;
                                else{
                                    value*=10;
                                    value=value+(str[i]-'0');
                                }

                            }
                            else{
                                if(negative){
                                    mtx.values[r][c]=value*(-1);
                                    negative=0;
                                }
                                else
                                    mtx.values[r][c]=value;
                                c++;
                                value=0;
                            }
                            i++;
                        }
                        if(value!=0) {    // to set the last element
                            if(negative)
                                mtx.values[r][c]=value*(-1);
                            else
                                mtx.values[r][c]=value;
                        }
                    }
                }
            }
            else{
                printf("wrong input in column number\n");
                return mtx;
            }
        }
        else{
            printf("wrong input in row number\n");
            return mtx;
        }

    }
    else
        printf(" the %s is an Empty file\n",filename);
    fclose(fp);
    return mtx;
}

void printMatrix(struct matrix m){
    for(int i=0;i<m.row;i++){
        for(int j=0;j<m.col;j++)
            printf("%d      ",m.values[i][j]);
        printf("\n");
    }
}

void * multiply1(void* threadarg){
    int r,c;
    struct threadParam *my_data;
    my_data = (struct threadParam *) threadarg;
    r=my_data->r;
    c=my_data->c;
    for(int k=0;k<m2.col;k++){    // to calculate row  output.values[r][k]
        int value=0;
        for(int j=0;j<m1.col;j++)
          value+=((m1.values[r][j])*(m2.values[j][k]));
        output.values[r][k]=value;
        //printf("r=%d     c=%d      value=%d\n",r,c,output.values[r][c]);
    }
}

void MulMethod1(){
    struct timeval stop, start;
    gettimeofday(&start, NULL); //start checking time
    int NumOfThreads=m1.row;
    printf("Number of Threads in method 1 : %d\n",NumOfThreads);
    pthread_t threads[NumOfThreads];
    struct threadParam temp[NumOfThreads];
    int rc;
    // thread to calculate rows
    for(int t=0;t<NumOfThreads;t++){
        temp[t].r=t;
        temp[t].c=m2.col;
        rc=pthread_create(&threads[t],NULL,multiply1,(void *)&temp[t]);
        if (rc) {
            printf("ERROR; return code from thread create() is %d\n", rc);
            exit(-1);
        }
    }
    for(int t=0; t<NumOfThreads; t++)
       pthread_join(threads[t], NULL);
    gettimeofday(&stop, NULL); //end checking time
    printf("Seconds taken in method 1 : %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken in method 1 : %lu\n", stop.tv_usec - start.tv_usec);
}

void * multiply2(void* threadarg){
    int r,c;
    struct threadParam *my_data;
    my_data = (struct threadParam *) threadarg;
    r=my_data->r;
    c=my_data->c;
    int value=0;
    for(int k=0;k<m1.col;k++)
        value+=((m1.values[r][k])*(m2.values[k][c]));
    output.values[r][c]=value;
}

void MulMethod2(){
    struct timeval stop, start;
    gettimeofday(&start, NULL); //start checking time
    int NumOfThreads=m1.row*m2.col;
    printf("Number of Threads in method 2 : %d\n",NumOfThreads);
    pthread_t threads[NumOfThreads];
    struct threadParam temp[NumOfThreads];
    int t=0;int rc;
    // thread to calculate elements
    for(int i=0;i<m1.row;i++){
        for(int j=0;j<m2.col;j++){
            temp[t].r=i;
            temp[t].c=j;
            rc=pthread_create(&threads[t],NULL,multiply2,(void *)&temp[t]);
            if (rc) {
                printf("ERROR; return code from pthread_create() is %d\n", rc);
                exit(-1);
            }
            t++;
        }
    }
    for(t=0; t<NumOfThreads; t++)
        pthread_join(threads[t], NULL);
    gettimeofday(&stop, NULL); //end checking time
    printf("Seconds taken in method 2 : %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken in method 2: %lu\n", stop.tv_usec - start.tv_usec);
}

void writeToFile(char *filename){
    FILE *fp;
    fp = fopen(filename, "w");
    if (fp == NULL){
        printf("Could not write to file %s",filename);
        return ;
    }
    for(int i=0;i<output.row;i++){
        for(int j=0;j<output.col;j++)
            fprintf(fp,"%d  ",output.values[i][j]);
        fprintf(fp,"\n");
    }
    fclose(fp);
}

int main(int args,char* argv[]){
    char *firstName;
    char *secondName;
    char *outputName;
    if(args==1){        //default files
        firstName="a";
        secondName="b";
        outputName="c";
    }
    else{
        firstName=argv[1];
        secondName=argv[2];
        outputName=argv[3];
    }
    m1=readFromFiles(firstName);
    if(m1.row==0 || m1.col==0)     // wrong in first matrix input
        return 0;
    m2=readFromFiles(secondName);
    if(m2.row==0 || m2.col==0)     // wrong in second matrix input
        return 0;
    printMatrix(m1);
    printf("************************************************\n");
    printMatrix(m2);
    printf("************************************************\n");
    if(m1.col!=m2.row){
        printf("\n the no of columns of 1st one should equal the number of rows in the 2nd one\n\n");
        return 0;
    }
    output.row=m1.row;
    output.col=m2.col;
    MulMethod1();
    printMatrix(output);
    printf("************************************************\n");
    MulMethod2();
    printMatrix(output);
    writeToFile(outputName);
    return 0;
}
