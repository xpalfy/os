#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(){
  int pid=fork();
  int p1[2];
  int p2[2];
  pipe(p1);
  pipe(p2);
  if(pid==0){
    close(p1[0]);
    close(p1[1]);
  }
  else if(pid==0){
    close(p2[0]);
    close(p2[1]);
  }
  else{
    exit(1);
  }
  exit(0);
}
