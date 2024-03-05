#include <stdio.h>
#include <stdlib.h>
#include<sys/types.h>
#include<unistd.h>
int main(int argc, char **argv) {int pid;int i;
  //printf("parent process pid is %d  \n",getpid());
  
    for (i=0;i<=1;i++) {
		printf("within the loop, i: %d\n", i);
		pid=fork();
	}
    printf("All children created  \n");
    
}
