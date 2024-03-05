/*
* Add NetID and names of all project partners
*
*/
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

// 0x5655624c
// 0xffffc330
// 0xffffc36c

void signal_handle(int signalno) {

    printf("OMG, I was slain!\n");
    /* 
       an increment of 15 is needed according to gdb for reaching the location where  
       the return address is found, we know it's the return address because we can see in gdb
       that the address matches the location of the division function in main
    */   
    int *return_pointer = &signalno;
    // because we find out that the return address is 15 spaces above, thanks to gdb
    return_pointer = return_pointer + 15;
    // because the instruction after z=x/y is 3 spaces ahead, courtesy of gdb
    // adding a piece of test code to test putty here
    *return_pointer = *return_pointer + 6; 
    return;
}

int main(int argc, char *argv[]) {

    int x=5, y = 0, z=4;

    /* Step 1: Register signal handler first*/
	signal(SIGFPE, signal_handle);
    // This will generate floating point exception

    z=x/y;

    printf("LOL, I live again !!!%d\n", z);

    return 0;
}


