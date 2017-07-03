#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

int i = 0;

void handle_alarm(int sig){
	i = 0;
	signal(SIGALRM, handle_alarm);
}

int main(){
	int randomnumber;
	signal(SIGALRM, handle_alarm);
	srand(getpid());
	randomnumber = rand() % 10;

	while(1){
		if(randomnumber == (i-1)){
		srand(getpid());
		randomnumber = rand() % 10;
		alarm(1);
	}
		printf("%d\n",i);
		sleep(1);
		i++;
	}
	return 0;
}
