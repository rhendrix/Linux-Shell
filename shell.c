#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/wait.h>

#define MAXLENGTH 256
//split string into array by delimiter
char** split(char* str, char delim)
{
	char* tmp = str;
	size_t count = 0;

	//count tokens
	while(*tmp)
	{
		if(*tmp == delim)
		{
			count++;
		}
		tmp++;
	}
	count += 2;

	//allocate space for char**
	char** result = (char**)malloc(sizeof(char*) * count);

	//duplicate substrings
	int start = 0;
	int stop = 0;
	int i, j = 0;
	while(1)
	{
		start = stop;
		while(str[stop] != 0 && str[stop] != delim && str[stop] != '\n')
			stop++;
		tmp = (char*)malloc(sizeof(char) * (stop-start+1));
		for(i=0;i<stop-start+1;i++)
			tmp[i] = str[i+start];
		tmp[i-1] = 0;
		result[j++] = tmp;
		if(str[stop] == 0 || str[stop] == '\n')
			break;
		stop++;
	}

	//end char** with null
	result[j] = 0;

	return result;
}

//print history array
void printHistory(char** hist, int comNum)
{
	int i;
	for(i=0;i<10;i++)
	{
		if(hist[i] == NULL)
			break;
		printf("%d %s\n", comNum-i, hist[i]);
	}
}

//frees history
void freeHistory(char** hist)
{
	int i;
	for(i=0;i<10;i++)
	{
		if(hist[i] == NULL)
			break;
		free(hist[i]);
	}
}

//frees args array
void freeArgs(char** args)
{
	int i =0;
	char* cur = args[i++];
	while(cur)
	{
		free(cur);
		cur = args[i++];
	}
	free(args);
}

//add entry to history array
void addHistory(char** hist, char* entry)
{
	int i;
	if(hist[9] != NULL)
		free(hist[9]);
	for(i=9;i>0;i--)
		hist[i] = hist[i-1];
	hist[0] = strdup(entry);
}

//locate argument in history
char* getHistory(char** hist, int comNum, char* histarg)
{
	int entryNum = atoi(histarg+1);
	free(histarg);
	return strdup(hist[comNum-entryNum]);
}

int main(void)
{
	char* argstr;
	char** args;
	int childstatus, shouldwait, shouldRun = 1;
	int command = 0;
	pid_t cpid, tpid;
	char* history[10];
	char cwd[1024];

	int i;
	for(i=0;i<10;i++)
		history[i] = NULL;
	
	while(shouldRun)
	{
		getcwd(cwd, sizeof(cwd));
		printf("%s$ ", cwd);
		fflush(stdout);
		argstr = malloc(sizeof(char) * MAXLENGTH);

		//get args
		if(!fgets(argstr, MAXLENGTH, stdin))
		{
			break;
		}
		
		//check for !! and ![num]
		if(argstr[0] == '!' && argstr[1] == '!')
		{
			free(argstr);
			argstr = strdup(history[0]);
		}
		else if(argstr[0] == '!')
		{
			argstr = getHistory(history, command, argstr);
		}

		//split args
		args = split(argstr, ' ');
		shouldwait = 1;

		//check for builtin commands
		if(!strcmp(args[0], "exit"))
		{
			shouldRun = 0;
			free(argstr);
			freeArgs(args);
			break;
		}
		else if(!strcmp(args[0], "history"))
		{
			printHistory(history, command);
			free(argstr);
			freeArgs(args);
			continue;
		}
		else if(!strcmp(args[0], "cd"))
		{
			chdir(args[1]);
			addHistory(history, argstr);
			command++;
			free(argstr);
			freeArgs(args);
			continue;
		}

		//execute argument
		else
		{
			//invoke a child process using fork()
			cpid = fork();
			if(!cpid)
			{
				//the child process will invoke execvp()
				execvp(args[0], args);
				_exit(EXIT_FAILURE);
			}
			addHistory(history, argstr);
			command++;
			free(argstr);
			freeArgs(args);
		}

		//wait for child process to finish
		while(shouldwait)
		{
			tpid = wait(&childstatus);
			if(tpid == cpid) shouldwait = 0;
		}
	}
	freeHistory(history);
	return 0;
}
