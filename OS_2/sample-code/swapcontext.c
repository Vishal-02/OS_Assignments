#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <ucontext.h>

#define STACK_SIZE sysconf(_SC_SIGSTKSZ)

void* f1withparam(ucontext_t *nctx){
	puts("You are in a different context! Going back to the main context..");
	setcontext(nctx);
	/* setcontext sets PC <--value in nctx context*/
}

int main(int argc, char **argv) {
	ucontext_t cctx,nctx;
	
	if (argc != 1) {
		printf(": USAGE Program Name and no Arguments expected\n");
		exit(1);
	}
	
	if (getcontext(&cctx) < 0){
		perror("getcontext");
		exit(1);
	}

	// Allocate space for stack	
	void *stack=malloc(STACK_SIZE);
	
	if (stack == NULL){
		perror("Failed to allocate stack");
		exit(1);
	}
      
	/* Setup context that we are going to use */
	cctx.uc_link=NULL;
	cctx.uc_stack.ss_sp=stack;
	cctx.uc_stack.ss_size=STACK_SIZE;
	cctx.uc_stack.ss_flags=0;
	
	puts("allocate stack, attach func, with 1 parameter");
	
	// Make the context to start running at f1withparam()
	makecontext(&cctx,(void *)&f1withparam,1,&nctx);
	
	puts("Successfully modified context");
	
	/* swap context will activate cctx and store location after swapcontext in nctx */
	swapcontext(&nctx,&cctx);

	/* PC value in nctx will point to here */
	puts("swap context executed correctly \n");
        
	return 0;
}

