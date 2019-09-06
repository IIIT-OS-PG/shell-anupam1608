#include<bits/stdc++.h>
#include<unistd.h>
#include <pwd.h>
#include<string.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<termios.h>
#include"util.h"
#include<signal.h>

#define max 512
#define maxchar 1024
using namespace std;

void signalHandler_child(int p){

	while (waitpid(-1, NULL, WNOHANG) > 0)
        {
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
        bash_pid = getpid();

        is_interact = isatty(STDIN_FILENO);

		if (is_interact)
        {

			while (tcgetpgrp(STDIN_FILENO) != (grbash_pid = getpgrp()))
					kill(bash_pid, SIGTTIN);



			act_child.sa_handler = signalHandler_child;

			act_int.sa_handler = signalHandler_int;



			sigaction(SIGCHLD, &act_child, 0);

			sigaction(SIGINT, &act_int, 0);


			setpgid(bash_pid, bash_pid);
			grbash_pid = getpgrp();

			if (bash_pid != grbash_pid)
            {
					printf("Error,shell is not process group leader");

					exit(EXIT_FAILURE);
			}

			tcsetpgrp(STDIN_FILENO, grbash_pid);


			tcgetattr(STDIN_FILENO, &bash_mode);


			currentDirectory = (char*) calloc(1024, sizeof(char));
        }
        else
        {
                printf("Could not make the shell interactive.\n");
                exit(EXIT_FAILURE);
        }
}




int changeDirectory(char* tokenlist[])
{


	if (tokenlist[1] == NULL)
    {
		chdir(getenv("HOME"));
		return 1;
	}

	else if (tokenlist[1] == "~")
    {
        cout<<"cd"<<endl;
		chdir(getenv("HOME"));
		return 1;
	}

	else{
		if (chdir(tokenlist[1]) == -1) {
			printf(" %s: no such directory\n", tokenlist[1]);
            return -1;
		}
	}
	return 0;
}
int Environment_manager(char * tokenlist[], int choice){
	char **env_aux;
	switch(choice){

		case 0:
			for(env_aux = environ; *env_aux != 0; env_aux ++){
				printf("%s\n", *env_aux);
			}
			break;
		case 1:
			if((tokenlist[1] == NULL) && tokenlist[2] == NULL){
				printf("%s","Not enought input arguments\n");
				return -1;
			}

			if(getenv(tokenlist[1]) != NULL){
				printf("%s", "The variable has been overwritten\n");
			}else{
				printf("%s", "The variable has been created\n");
			}

			if (tokenlist[2] == NULL){
				setenv(tokenlist[1], "", 1);
			}else{
				setenv(tokenlist[1], tokenlist[2], 1);
			}
			break;
		case 2:
			if(tokenlist[1] == NULL){
				printf("%s","Not enought input arguments\n");
				return -1;
			}
			if(getenv(tokenlist[1]) != NULL){
				unsetenv(tokenlist[1]);
				printf("%s", "The variable has been erased\n");
			}else{
				printf("%s", "The variable does not exist\n");
			}
		break;


	}
	return 0;
}
void prompt()
{

	char hostname[1204] = "";

	gethostname(hostname, sizeof(hostname));

	printf("%s@%s %s > ", getenv("LOGNAME"), hostname, getcwd(currentDirectory, 1024));
}

void execcmd(char **tokenlist, int background){
	 int err = -1;

	 if((pid=fork())==-1){
		 printf("Child process could not be created\n");
		 return;
	 }
	if(pid==0){

		signal(SIGINT, SIG_IGN);


		setenv("parent",getcwd(currentDirectory, 1024),1);


		if (execvp(tokenlist[0],tokenlist)==err){
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
void fileIO(char * tokenlist[], char* inputFile, char* outputFile, int choice)
{

	int err = -1,fd;

    //cout<<"in file io"<<endl;
	if((pid=fork())==-1){
		printf("Child process could not be created\n");
		return;
	}
	if(pid==0){

		if (choice == 0)
            {
            //cout<<"in >"<<endl;
			fd = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);
            dup2(fd, STDOUT_FILENO);
			close(fd);

		}
		else if (choice == 1)
		{

			fd = open(inputFile, O_RDONLY, 0600);

			dup2(fd, STDIN_FILENO);
			close(fd);

			fd = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);
			dup2(fd, STDOUT_FILENO);
			close(fd);
		}
		else if(choice ==2)
        {
            fd = open(outputFile, O_WRONLY|O_APPEND);
            dup2(fd, STDOUT_FILENO);
			close(fd);

        }

		setenv("parent",getcwd(currentDirectory, 1024),1);

		if (execvp(tokenlist[0],tokenlist)==err)
        {
			printf("err");
			kill(getpid(),SIGTERM);
		}
	}
	waitpid(pid,NULL,0);
}
void pipe_manager(char * tokenlist[])
{

	int fd1[2];
	int fd2[2];

	int no_of_cmds = 0;

	char *cmd[256];

	pid_t pid;

	int err = -1;
	int end = 0;


	int i = 0,j = 0,k = 0,l = 0;

	while (tokenlist[l] != NULL)
        {
		if (strcmp(tokenlist[l],"|") == 0)
		{
			no_of_cmds++;
		}

		l++;
	}
	no_of_cmds++;

    //printf("%d\n",num_cmds);

	while (tokenlist[j] != NULL && end != 1)
    {
		k = 0;

		while (strcmp(tokenlist[j],"|") != 0)
        {
			cmd[k] = tokenlist[j];
			j++;
			if (tokenlist[j] == NULL)
			{

				end = 1;
				k++;
				break;
			}
			k++;
		}

		cmd[k] = NULL;
		j++;


		if (i % 2 != 0)
        {
			pipe(fd1);
		}
		else
		{
			pipe(fd2);
		}

		pid=fork();

		if(pid==-1){
			if (i != no_of_cmds - 1)
            {
				if (i % 2 != 0)
				{
					close(fd1[1]);
				}
				else
				{
					close(fd2[1]);
				}
			}
			printf("Child could not be created \n");

			return;
		}
		if(pid==0){

			if (i == 0){
				dup2(fd2[1], STDOUT_FILENO);
			}

			else if (i == no_of_cmds - 1)
            {
				if (no_of_cmds % 2 != 0){

					dup2(fd1[0],STDIN_FILENO);
				}
				else{

					dup2(fd2[0],STDIN_FILENO);
				}

			}
			else{
				if (i % 2 != 0)
                {
					dup2(fd2[0],STDIN_FILENO);

					dup2(fd1[1],STDOUT_FILENO);
				}
				else
                {
					dup2(fd1[0],STDIN_FILENO);

					dup2(fd2[1],STDOUT_FILENO);
				}
			}

			if (execvp(cmd[0],cmd)==err){
				kill(getpid(),SIGTERM);
			}
		}


		if (i == 0){
			close(fd2[1]);
		}
		else if (i == no_of_cmds - 1)
        {
			if (no_of_cmds % 2 != 0)
			{
				close(fd1[0]);
			}
        else
        {
				close(fd2[0]);
			}
		}
		else{
			if (i % 2 != 0){
				close(fd2[0]);

				close(fd1[1]);
			}
			else{
				close(fd1[0]);

				close(fd2[1]);
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
                Environment_manager(tokenlist,0);
                dup2(stdOut,STDOUT_FILENO);
            }
        }
        else
        {
            Environment_manager(tokenlist,0);
        }
    }

    else if(strcmp(tokenlist[0],"unsetenv")==0)
        Environment_manager(tokenlist,2);

    else if(strcmp(tokenlist[0],"setenv")==0)
        Environment_manager(tokenlist,1);

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
                pipe_manager(tokenlist);

                return 1;
            }
            else if(strcmp(tokenlist[x],"<")==0)
            {
                a=x+1;
                if(tokenlist[a]==NULL || tokenlist[a+1] || tokenlist[a+2]==NULL)
                {
                    printf("more arguments required\n");

                    return -1;
                }
                else
                {
                    if(strcmp(tokenlist[a+1],">")!=0)
                    {
                        cout<<"usage expected '>' and found "<<tokenlist[a+1]<<endl;

                        return -2;
                    }

                }
                fileIO(tokenlist_a,tokenlist[x+1],tokenlist[x+3],1);
                return 1;
            }
            else if(strcmp(tokenlist[x],">")==0)
            {
                if(tokenlist[x+1]==NULL)
                {
                    cout<<"not enough";
                    cout<<endl;

                    return -1;
                }
                fileIO(tokenlist_a,NULL,tokenlist[x+1],0);
                return 1;
            }
          /* else if(strcmp(tokenlist[x],">>")==0)
            {
                if(tokenlist[x+1]==NULL)
                {
                    cout<<"not enough";
                    cout<<endl;

                    return -1;
                }
                fileIO(tokenlist_a,NULL,tokenlist[x+1],2);
                return 1;
            }*/
            x++;

        }
        tokenlist_a[x]=NULL;
        execcmd(tokenlist_a,bg);

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
