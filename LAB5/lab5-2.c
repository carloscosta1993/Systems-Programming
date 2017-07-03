#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
 

int verify_prime(int n){
  int i;
  for(i=2; i <n; i++){
    if(n%i==0)
      return 0;
  }
  return 1;
}
int main(int argc, char const *argv[]) {
  int n_child;
  if (argc == 1){
    n_child = 0;
  }
  if(argc == 2){
    if (sscanf(argv[1], "%d", &n_child) == 0)
		  n_child = 0;  
  }
 printf("number of children %d\n", n_child);


  if(n_child==0){
    int i, n;
    for(i = 2; i < 99999; i++){
      n = random()%999999;
      if (verify_prime(n))
       printf("%d is prime\n", n);
    }
  }else{
	  int new_pipe[2*n_child];
	  int z=0;
	  int n;
	  int x;
	  for(int i=0;i<n_child;i++){
		  pipe(&new_pipe[2*i]);
	  }
	  for(int j=0;j<n_child;j++){
		  if(fork()==0){
			  int prime=0;
			  do{
				read(new_pipe[2*j], &z, sizeof(int));
				if (verify_prime(z))
					printf("IM CHILD %d ---- %d is prime\n", getpid(), z);
				}while(z!=-1);
				printf("%d ACABEI\n",getpid());
		}else{
			break;
			}	    
	  }
	  
	  for(int i = 2; i < 100; i++){
      //n = random()%999999;
	  x = random()%(n_child);
	  //printf("%d %d\n",n,x);
	  write(new_pipe[2*x+1], &i, sizeof(int));
    }
    n=-1;
    for(int f=0;f<n_child;f++){
		write(new_pipe[2*f+1], &n, sizeof(int));
	}
	wait(NULL);    
  }
  
  
  return 0;
}
