#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/wait.h>
#define nb_processes 3

int main(int argc, char const *argv[]) {
  int  i=0;
  int s[nb_processes];
  pid_t pid;
  int m_7_19=0, m_19=0, m_7=0;
  int pipe1[2], pipe2[2], pipe3[2];
  pipe(pipe1);
  pipe(pipe2);
  pipe(pipe3);
  
  do{
		i++;
		write(pipe1[1], &i, sizeof(i));
		write(pipe2[1], &i, sizeof(i));
		write(pipe3[1], &i, sizeof(i));
	}while (i<50);
	i=-1;
	write(pipe1[1], &i, sizeof(i));
	write(pipe2[1], &i, sizeof(i));
	write(pipe3[1], &i, sizeof(i));

  
  if(fork()==0){
	  do{
		read(pipe1[0], &s[0], sizeof(int));
		if(s[0]%7 == 0){
         m_7 ++;
			}  
		}while(s[0]!=-1);
		printf("m 7    %d\n", m_7);
	}
	
	if(fork()==0){
	  do{
		read(pipe2[0], &s[1], sizeof(int));
		if(s[1]%19 == 0){
         m_19 ++;
			}  
		}while(s[1]!=-1);
		printf("m 19    %d\n", m_19);
	}
	  
	if(fork()==0){
	  do{
		read(pipe3[0], &s[2], sizeof(int));
		if((s[2]%7 == 0) && (s[2]%19 == 0)){
         m_7_19 ++;
			}  
		}while(s[2]!=-1);
		printf("m 7_19    %d\n", m_7_19);	
	}
	
	wait(NULL);
	return 0;
	 
  }
  
