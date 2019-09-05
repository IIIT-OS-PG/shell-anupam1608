#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<termios.h>
#include"util.h"
#include<signal.h>

#define max 512
#define maxchar 1024

void signalHandler_child(int p){

	while (waitpid(-1, NULL, WNOHANG) > 0) {
	}
	printf("\n");
}

void signalHandler_int(int p){

	if (kill(pid,SIGTERM) == 0){
		printf("\nProcess %d received a SIGINT signal\n",pid);
		no_reprint_prmpt = 1;
	}else{
		printf("\n");
	}
}

void initialize()
{
        GBSH_PID = getpid();

        GBSH_IS_INTERACTIVE = isatty(STDIN_FILENO);

		if (GBSH_IS_INTERACTIVE)
        {

			while (tcgetpgrp(STDIN_FILENO) != (GBSH_PGID = getpgrp()))
					kill(GBSH_PID, SIGTTIN);



			act_child.sa_handler = signalHandler_child;
			act_int.sa_handler = signalHandler_int;



			sigaction(SIGCHLD, &act_child, 0);
			sigaction(SIGINT, &act_int, 0);


			setpgid(GBSH_PID, GBSH_PID);
			GBSH_PGID = getpgrp();
			if (GBSH_PID != GBSH_PGID)
            {
					printf("Error, the shell is not process group leader");
					exit(EXIT_FAILURE);
			}

			tcsetpgrp(STDIN_FILENO, GBSH_PGID);


			tcgetattr(STDIN_FILENO, &GBSH_TMODES);


			currentDirectory = (char*) calloc(1024, sizeof(char));
        }
        else
        {
                printf("Could not make the shell interactive.\n");
                exit(EXIT_FAILURE);
        }
}


void prompt(){

	char hostname[1204] = "";

	gethostname(hostname, sizeof(hostname));

	printf("%s@%s %s > ", getenv("LOGNAME"), hostname, getcwd(currentDirectory, 1024));
}

int changeDirectory(char* args[]){


	if (args[1] == NULL)
    {
		chdir(getenv("HOME"));
		return 1;
	}

	else if (args[1] == "~")
    {
		chdir(getenv("HOME"));
		return 1;
	}

	else{
		if (chdir(args[1]) == -1) {
			printf(" %s: no such directory\n", args[1]);
            return -1;
		}
	}
	return 0;
}
int manageEnviron(char * args[], int option){
	char **env_aux;
	switch(option){

		case 0:
			for(env_aux = environ; *env_aux != 0; env_aux ++){
				printf("%s\n", *env_aux);
			}
			break;
		case 1:
			if((args[1] == NULL) && args[2] == NULL){
				printf("%s","Not enought input arguments\n");
				return -1;
			}

			if(getenv(args[1]) != NULL){
				printf("%s", "The variable has been overwritten\n");
			}else{
				printf("%s", "The variable has been created\n");
			}

			if (args[2] == NULL){
				setenv(args[1], "", 1);
			}else{
				setenv(args[1], args[2], 1);
			}
			break;
		case 2:
			if(args[1] == NULL){
				printf("%s","Not enought input arguments\n");
				return -1;
			}
			if(getenv(args[1]) != NULL){
				unsetenv(args[1]);
				printf("%s", "The variable has been erased\n");
			}else{
				printf("%s", "The variable does not exist\n");
			}
		break;


	}
	return 0;
}

void launchProg(char **args, int background){
	 int err = -1;

	 if((pid=fork())==-1){
		 printf("Child process could not be created\n");
		 return;
	 }
	if(pid==0){

		signal(SIGINT, SIG_IGN);


		setenv("parent",getcwd(currentDirectory, 1024),1);


		if (execvp(args[0],args)==err){
			printf("Command not found");
			kill(getpid(),SIGTERM);
		}
	 }


	 if (background == 0){
		 waitpid(pid,NULL,0);
	 }else{

		 printf("Process created with PID: %d\n",pid);
	 }
}
void fileIO(char * args[], char* inputFile, char* outputFile, int option){

	int err = -1;

	int fileDescriptor;
	if((pid=fork())==-1){
		printf("Child process could not be created\n");
		return;
	}
	if(pid==0){

		if (option == 0){

			fileDescriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);
            dup2(fileDescriptor, STDOUT_FILENO);
			close(fileDescriptor);

		}else if (option == 1){

			fileDescriptor = open(inputFile, O_RDONLY, 0600);

			dup2(fileDescriptor, STDIN_FILENO);
			close(fileDescriptor);

			fileDescriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);
			dup2(fileDescriptor, STDOUT_FILENO);
			close(fileDescriptor);
		}

		setenv("parent",getcwd(currentDirectory, 1024),1);

		if (execvp(args[0],args)==err){
			printf("err");
			kill(getpid(),SIGTERM);
		}
	}
	waitpid(pid,NULL,0);
}
void pipeHandler(char * args[]){

	int filedes[2];
	int filedes2[2];

	int num_cmds = 0;

	char *command[256];

	pid_t pid;

	int err = -1;
	int end = 0;


	int i = 0;
	int j = 0;
	int k = 0;
	int l = 0;


	while (args[l] != NULL){
		if (strcmp(args[l],"|") == 0){
			num_cmds++;
		}
		l++;
	}
	num_cmds++;

    //printf("%d\n",num_cmds);

	while (args[j] != NULL && end != 1){
		k = 0;

		while (strcmp(args[j],"|") != 0){
			command[k] = args[j];
			j++;
			if (args[j] == NULL){

				end = 1;
				k++;
				break;
			}
			k++;
		}

		command[k] = NULL;
		j++;


		if (i % 2 != 0){
			pipe(filedes);
		}else{
			pipe(filedes2);
		}

		pid=fork();

		if(pid==-1){
			if (i != num_cmds - 1){
				if (i % 2 != 0){
					close(filedes[1]);
				}else{
					close(filedes2[1]);
				}
			}
			printf("Child process could not be created\n");
			return;
		}
		if(pid==0){

			if (i == 0){
				dup2(filedes2[1], STDOUT_FILENO);
			}

			else if (i == num_cmds - 1){
				if (num_cmds % 2 != 0){
					dup2(filedes[0],STDIN_FILENO);
				}else{
					dup2(filedes2[0],STDIN_FILENO);
				}

			}else{
				if (i % 2 != 0){
					dup2(filedes2[0],STDIN_FILENO);
					dup2(filedes[1],STDOUT_FILENO);
				}else{
					dup2(filedes[0],STDIN_FILENO);
					dup2(filedes2[1],STDOUT_FILENO);
				}
			}

			if (execvp(command[0],command)==err){
				kill(getpid(),SIGTERM);
			}
		}


		if (i == 0){
			close(filedes2[1]);
		}
		else if (i == num_cmds - 1){
			if (num_cmds % 2 != 0){
				close(filedes[0]);
			}else{
				close(filedes2[0]);
			}
		}else{
			if (i % 2 != 0){
				close(filedes2[0]);
				close(filedes[1]);
			}else{
				close(filedes[0]);
				close(filedes2[1]);
			}
		}

		waitpid(pid,NULL,0);

		i++;
	}
}
int cmdhandler(char *tokenlist[])
{
    int x,y;
    x=0;
    y=0;

    int fd;
    int stdOut;

    int a,bg=0;

    char *tokenlist_a[256];

    while(tokenlist[y]!=NULL)
    {
        if((strcmp(tokenlist[y],"<")==0) || (strcmp(tokenlist[y],">")==0) || (strcmp(tokenlist[y],"&")==0) )
         break;

        tokenlist_a[y]=tokenlist[y];
        y++;
    }
    if(strcmp(tokenlist[0],"exit")==0)
        exit(0);
    else if(strcmp(tokenlist[0],"clear")==0)
        system("clear");
    else if(strcmp(tokenlist[0],"cd")==0)
        changeDirectory(tokenlist);
    else if(strcmp(tokenlist[0],"pwd")==0)
    {
        if(tokenlist[y]!=NULL)
        {
            if((strcmp(tokenlist[y],">")==0) && (tokenlist[y+1]!=NULL) )
            {
                fd=open(tokenlist[y+1],O_CREAT | O_TRUNC | O_WRONLY, 0600);
                stdOut=dup(STDOUT_FILENO);
                dup2(fd,STDOUT_FILENO);
                close(fd);

                printf("%s\n",getcwd(currentDirectory,1024));
                dup2(stdOut, STDOUT_FILENO);
            }
        }

        else
        {
            printf("%s\n",getcwd(currentDirectory,1024));
        }
    }
    else if(strcmp(tokenlist[0],"environ")==0)
    {
        if(tokenlist[y]!=NULL)
        {
            if((strcmp(tokenlist[y],">")==0) && tokenlist[y+1]!=NULL)
            {
                 fd=open(tokenlist[y+1],O_CREAT | O_TRUNC | O_WRONLY, 0600);
                stdOut=dup(STDOUT_FILENO);
                dup2(fd,STDOUT_FILENO);
                close(fd);
                manageEnviron(tokenlist,0);
                dup2(stdOut,STDOUT_FILENO);
            }
        }
        else
        {
            manageEnviron(tokenlist,0);
        }
    }
    else if(strcmp(tokenlist[0],"unsetenv")==0)
        manageEnviron(tokenlist,2);
    else if(strcmp(tokenlist[0],"setenv")==0)
        manageEnviron(tokenlist,1);
    else
    {
        while(tokenlist[x]!=NULL && bg==0)
        {
            if(strcmp(tokenlist[x],"&")==0)
            {
                bg=1;
            }
            else if(strcmp(tokenlist[x],"|")==0)
            {
                pipeHandler(tokenlist);
                return 1;
            }
            else if(strcmp(tokenlist[x],"<")==0)
            {
                a=x+1;
                if(tokenlist[a]==NULL || tokenlist[a+1] || tokenlist[a+2]==NULL)
                {
                    printf("require more arguments\n");
                    return -1;
                }
                else
                {
                    if(strcmp(tokenlist[a+1],">")!=0)
                    {
                        printf("usage expected '>' and found %s \n",tokenlist[a+1]);
                        return -2;
                    }

                }
                fileIO(tokenlist_a,tokenlist[x+1],tokenlist[x+3],1);
                return 1;
            }
            else if(strcmp(tokenlist[x],">")==0)
            {
                if(tokenlist[x+1]=NULL)
                {
                    printf("not enough");

                    return -1;
                }
                fileIO(tokenlist_a,NULL,tokenlist[x+1],0);
                return 1;
            }
            x++;

        }
        tokenlist_a[x]=NULL;
        launchProg(tokenlist_a,bg);

    }

    return 1;
}
int main(int argc, char *argv[], char ** envp)
{

    char buff[maxchar];
    char *tokenlist[max];
    no_reprint_prmpt=0;
    pid=-15;

    int token_cnt;
    initialize();

    environ=envp;

    setenv("SHELL",getcwd(currentDirectory,1024),1);

    while(1)
    {
        if(no_reprint_prmpt==0)
            prompt();
        no_reprint_prmpt=0;
        memset(buff,'\0',maxchar);

        fgets(buff,maxchar,stdin);

        if((tokenlist[0] = strtok(buff," \n\t"))== NULL)
            continue;
        token_cnt=1;

        while((tokenlist[token_cnt]= strtok(NULL, " \n\t"))!= NULL)
        {
            token_cnt++;

        }
        cmdhandler(tokenlist);
    }


}
