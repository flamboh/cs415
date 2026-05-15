#include<stdio.h>
#include <sys/types.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>

void script_print (pid_t* pid_ary, int size);

int main(int argc,char*argv[])
{
	if (argc != 2)
	{
		printf ("Wrong number of arguments\n");
		exit (0);
	}

	/*
	*	TODO
	*	#1	declare child process pool
	*/
	pid_t* pid_array = (pid_t*)malloc(sizeof(pid_t) * atoi(argv[1]));
	if (pid_array == NULL)
	{
		perror ("malloc: ");
		exit(1);
	}

	/*	#2 	spawn n new processes
	*		first create the argument needed for the processes
	*		for example "./iobound -seconds 10"
	*/
	int n = atoi(argv[1]);
	if (n < 1)
	{
		printf ("Invalid input\n");
		exit(1);
	}
	for (int i = 0; i < n; i++)
	{
		pid_t pid = fork();
		if (pid == -1)
		{
			perror ("fork: ");
			exit(1);
		}
		pid_array[i] = pid;
		if (pid_array[i] == 0)
		{
			char* iobound_arg[] = {"./iobound", "-seconds", "10", NULL};
			if (execvp("./iobound", iobound_arg) == -1)
			{
				perror ("execvp: ");
				exit(1);
			}
			exit(0);
		}
	}
	/*	#3	call script_print
	*/
	script_print(pid_array, n);
	/*
	*	#4	wait for children processes to finish
	*/
	for (int i = 0; i < n; i++)
	{
		waitpid(pid_array[i], NULL, 0);
	}
	/*	#5	free any dynamic memory
	*/
	free(pid_array);

	return 0;
}


void script_print (pid_t* pid_ary, int size)
{
	FILE* fout;
	fout = fopen ("top_script.sh", "w");
	fprintf(fout, "#!/bin/bash\ntop");
	for (int i = 0; i < size; i++)
	{
		fprintf(fout, " -p %d", (int)(pid_ary[i]));
	}
	fprintf(fout, "\n");
	fclose (fout);

	char* top_arg[] = {"gnome-terminal", "--", "bash", "top_script.sh", NULL};
	pid_t top_pid;

	top_pid = fork();
	{
		if (top_pid == 0)
		{
			if(execvp(top_arg[0], top_arg) == -1)
			{
				perror ("top command: ");
			}
			exit(0);
		}
	}
}
