#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(){
	int p[2];
	pipe(p);
  	int pid=fork();
  	char byte;
  	if(pid==0){
		char tmp='X';
	  	write(p[1],&tmp,1);
	  	printf("%d: received ping\n", getpid());	
  	}
  	else{
		wait(0);
		read(p[1],&byte,1);
	  	printf("%d: received pong\n", getpid());
  	}
  	exit(0);
}
