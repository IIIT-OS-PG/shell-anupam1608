#include<bits/stdc++.h>
#include<unistd.h>
#include <pwd.h>
#include<string.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<termios.h>
#include<signal.h>

#define max 512
#define maxchar 1024
#define TRUE 1
#define FALSE !TRUE

static pid_t bash_pid;
static pid_t grbash_pid;
static int is_interact;
static struct termios bash_mode;
static char* Cd;
extern char** environ;
struct sigaction act_child;
struct sigaction act_int;
int prompt_num;
char hostname[1204] = "";
struct passwd *p;
pid_t pid;
char * hist_file;
char hist_data[1024][1024];
char hist_var[2048];
int line_no=0;
char currwd[1000];

void signalHandler_child(int p);
void signalHandler_int(int p);
int ch_dir(char * args[]);
void initialize();
void init_bashrc();
void prompt();
int cmdhandler(char **);
void copy_history();
void write_curr_history(char buffer[]);

using namespace std;

int main(int argc, char *argv[], char ** envp)
{

    char buff[maxchar];
    char buffdup[maxchar];
    char *tokenlist[max];
    prompt_num=0;
    pid=-15;

    int token_cnt;
    initialize();
    init_bashrc();

    environ=envp;

    setenv("SHELL",getcwd(Cd,1024),1);
    //getcwd(currwd,sizeof(currwd));

    while(1)
    {
        if(prompt_num==0)
            prompt();
        prompt_num=0;
        line_no=0;
        memset(buff,'\0',maxchar);

        fgets(buff,maxchar,stdin);
        //strcpy(buffdup,buff);
        //int len=strlen(buffdup);

        //printf("d%s\n",buffdup);
        //copy_history();
        //write_curr_history(buffdup);
        //strcpy(hist_var,buff);

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

void init_bashrc()
{
    char name[]="bashrc";
    FILE *fp;
    fp=fopen(name,"w");
    char s='$';
    gethostname(hostname, sizeof(hostname));


    p=getpwuid(getuid());
    fprintf(fp, "USER : %s \n",getenv("LOGNAME"));
    
    fprintf(fp,"HOSTNAME :%s \n",hostname);
    fprintf(fp,"$PS1 :%c \n",s);

    fclose(fp);

}

void signalHandler_child(int p)
{

	while (waitpid(-1, NULL, WNOHANG) > 0)
    {
	}
	
}

void signalHandler_int(int p)
{

	if (kill(pid,SIGTERM) == 0)
	{
		printf("\nProcess %d received a signal\n",pid);
		prompt_num = 1;
	}
	else
	{
		cout<<endl;
	}
}

void initialize() //source : gnu.org
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
					printf("Error : Shell is not  a process group leader");

					exit(EXIT_FAILURE);
			}


			tcsetpgrp(STDIN_FILENO, grbash_pid);
			tcgetattr(STDIN_FILENO, &bash_mode);


			Cd = (char*) calloc(1024, sizeof(char));
        }
        else
        {
                cout<<"shell not interactive"<<endl;
                exit(EXIT_FAILURE);
        }
}
void copy_history()
{

    int filedescriptor;
    hist_file=(char *)malloc(100*sizeof(char));

    strcat(hist_file,currwd);
    strcat(hist_file,"/");
    strcat(hist_file,"history.txt");
    filedescriptor=open(hist_file, O_RDONLY|O_CREAT,S_IRUSR|S_IWUSR);
    int rbytes=0,i=0,idx=0,x=0;
    char buff[1], temp[1024];

    do {

          rbytes=read(filedescriptor,buff,sizeof(buff));
          for(i=0;i<rbytes;i++)
          {

              temp[idx]=buff[i];
              idx++;
              if(buff[i]=='\n')
              {
                  //cout<<"a"<<endl;
                  temp[idx-1]='\0';
                  idx=0;
                  strcpy(hist_data[x],temp);
                  //printf("%s\n",hist_data[x]);
                  line_no++;
                  x++;
                  temp[0]='\0';

              }
          }

    }while(rbytes==sizeof(buff));
    close(filedescriptor);


}
void write_curr_history(char buffer[])
{
    char curr_input[2048];
    int filedescriptor;
    int res,len=0;
    line_no++;
    char lnum[10];

    sprintf(lnum,"%d",line_no);
    strcpy(curr_input," ");
    strcat(curr_input,lnum);
    strcat(curr_input," ");
    strcat(curr_input,buffer);
    //printf("curr %s\n",curr_input);


    filedescriptor=open(hist_file,O_WRONLY|O_APPEND|O_CREAT,S_IRUSR|S_IWUSR);
    len=strlen(curr_input);
    //cout<<len<<endl;
    res=write(filedescriptor,curr_input,len);

    if(res<0)
    {
        cout<<"error in writing";
        return ;
    }
    close(filedescriptor);


}

void print_history()
{
    for(int i=0;i<line_no-1;i++)
    {
        printf("%s\n", hist_data[i] );
    }
    printf(" %d %s\n", line_no, hist_var);
}

void echo(char *tokenlist[])
{
    if(tokenlist[1][0]=='$')
    {   
    int i=1,j=0;
    char environ_var[1000], *variable;

    while(tokenlist[1][i]!='\0')
    {
        environ_var[j]=tokenlist[1][i];
        j++;
        i++;
    }

    environ_var[j]='\0';
    variable=getenv(environ_var);

    if(!variable)
        cout<<endl;
    printf("%s\n",variable);
}
    else if(tokenlist[1][0]!='\0')
    {
        printf("%s\n",tokenlist[1]);    

    }

}


int ch_dir(char* tokenlist[])
{


	 if (strcmp(tokenlist[1],"~")==0 || strcmp(tokenlist[1],"~/")==0)
    {
        //cout<<"cd"<<endl;
		chdir(getenv("HOME"));
		return 1;
	}
	else if (tokenlist[1] == NULL)
    {
		chdir(getenv("HOME"));
		return 1;
	}

	else if(chdir(tokenlist[1]) == -1)
		 {
			printf(" %s: no such directory\n", tokenlist[1]);
            return -1;
		}
	
	return 0;
}


int Environment_manager(char * tokenlist[], int choice)
{
	char **env_aux;
	
		if(choice==0)
		{
			for(env_aux = environ; *env_aux != 0; env_aux ++)
			{
				printf("%s\n", *env_aux);
			}
		}	
		else if (choice==1)
		{
			if((tokenlist[1] == NULL) && tokenlist[2] == NULL)
			{
				cout<<"Not enough arguments"<<endl;
				return -1;
			}

			if(getenv(tokenlist[1]) != NULL)
			{
				cout<<"variable overwritten "<<endl;
			}
			else
			{
				cout<<"variable created"<<endl;
			}

			if (tokenlist[2] == NULL)
			{
				setenv(tokenlist[1], "", 1);
			}
			else
			{
				setenv(tokenlist[1], tokenlist[2], 1);
			}
		}
		else if(choice==2)
		{
			if(tokenlist[1] == NULL)
			{
				printf("%s","Not enought input arguments\n");
				return -1;
			}
			if(getenv(tokenlist[1]) != NULL)
			{
				unsetenv(tokenlist[1]);

				printf("%s", "The variable has been erased\n");
			}
			else
			{
				printf("%s", "The variable does not exist\n");
			}
		}

		return 0;
	}
	

void prompt()
{
	gethostname(hostname, sizeof(hostname));

	printf("%s@%s %s > ", getenv("LOGNAME"), hostname, getcwd(Cd, 1024));
}

void execcmd(char **tokenlist, int bg)
{
	 int err = -1;

	 if((pid=fork())==-1)
	 {
		 printf("Child could not be created\n");

		 return;
	 }

	if(pid==0)
	{

		signal(SIGINT, SIG_IGN);


		setenv("parent",getcwd(Cd, 1024),1);


		if (execvp(tokenlist[0],tokenlist)==err)
		{
			printf("Command not found");
			kill(getpid(),SIGTERM);
		}
	 }


	 if (bg == 0){
		 waitpid(pid,NULL,0);
	 }
	 else
	 {

		 printf("Process created with PID: %d\n",pid);
	 }
}
void fileIO(char * tokenlist[], char* inp, char* out, int choice)
{

	int err = -1,fd;

    //cout<<"in file io"<<endl;
	if((pid=fork())==-1)
	{
		printf("Child process could not be created\n");
		return;
	}
	if(pid==0)
	{

		if (choice == 0)
         {
            //cout<<"in >"<<endl;
			fd = creat(out, 0644);;
            dup2(fd, STDOUT_FILENO);
			close(fd);

		}

		else if (choice == 1)
		{

			fd = open(inp, O_RDONLY, 0600);

			dup2(fd, STDIN_FILENO);
			close(fd);

			fd = open(out, O_CREAT | O_TRUNC | O_WRONLY, 0600);
			dup2(fd, STDOUT_FILENO);
			close(fd);
		}
		
		else if(choice ==2)
        {
            fd = open(out, O_WRONLY|O_APPEND);
            dup2(fd, STDOUT_FILENO);
			close(fd);

        }

		setenv("parent",getcwd(Cd, 1024),1);

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

	int no_of_cmds = 0,i = 0,j = 0,k = 0,l = 0,err = -1,end = 0;
	char *cmd[256];

	pid_t pid;

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

		if(pid==-1)
		{
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
		if(pid==0)
		{

			 if (i == no_of_cmds - 1)
            {
				if (no_of_cmds % 2 != 0)
				{

					dup2(fd1[0],STDIN_FILENO);
				}
				else
				{

					dup2(fd2[0],STDIN_FILENO);
				}

			}

            else if (i == 0)
            {
                dup2(fd2[1], STDOUT_FILENO);
            }
			else
			{
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

			if (execvp(cmd[0],cmd)==err)
			{
				kill(getpid(),SIGTERM);
			}
		}


		if (i == 0)
		{
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
		else
		{
			if (i % 2 != 0)
			{
				close(fd2[0]);

				close(fd1[1]);
			}
			else
			{
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
    int x,y,fd,stdOut,a,bg;
    x=0;
    y=0;
    bg=0;

    char *tokenlist_a[256];

    while(tokenlist[y]!=NULL)
    {
        if((strcmp(tokenlist[y],"<")==0) || (strcmp(tokenlist[y],">")==0) || (strcmp(tokenlist[y],"&")==0) )
         break;

        tokenlist_a[y]=tokenlist[y];
        y++;
    }

     
    if(strcmp(tokenlist[0],"cd")==0)
        ch_dir(tokenlist);

    else if(strcmp(tokenlist[0],"exit")==0)
        exit(0);
    else if(strcmp(tokenlist[0],"echo")==0)
    		echo(tokenlist);

    else if(strcmp(tokenlist[0],"history")==0)
    {
    	print_history();
    }	
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

                printf("%s\n",getcwd(Cd,1024));
                dup2(stdOut, STDOUT_FILENO);
            }
        }

        else
        {
            printf("%s\n",getcwd(Cd,1024));
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

