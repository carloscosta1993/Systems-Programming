#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char const *argv[]) {
	int  s[3], i =0, pid;
	int pipe1[2], pipe2[2], pipe3[2];
	int m_7_19, m_19, m_7;

	m_7_19= m_19= m_7=0;
	pipe(pipe1);
	pipe(pipe2);
	pipe(pipe3);
	
	do{
		i++;
		write(pipe1[1], &i, sizeof(i));
		write(pipe2[1], &i, sizeof(i));
		write(pipe3[1], &i, sizeof(i));
	}while (i<30);
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
		}while (s[0]!=-1);
		printf("m 7    %d\n", m_7);
		exit(1);
	}
	if(fork()==0){
		do{
			read(pipe2[0], &s[1], sizeof(int));
			if(s[1]%19 == 0){
				m_19 ++;
	 		}
		}while (s[1]!=-1);
		printf("m 19    %d\n", m_19);
		exit(1);
	}

	if(fork()==0){
		do{
			read(pipe3[0], &s[2], sizeof(int));
			if((s[2]%19  == 0) && (s[2]%7  == 0)){
				m_7_19 ++;
	 		}
		}while (s[2]!=-1);
		printf("m 7 19    %d\n", m_7_19);
		exit(1);
	}

	wait(NULL);


	return 0;
	}
