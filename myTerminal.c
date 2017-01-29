#define _XOPEN_SOURCE 600
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<string.h>
#include<errno.h>
#include<termios.h>
#include<sys/ioctl.h>
#include<sys/types.h>
#include<sys/select.h>
#include<unistd.h>
#define BUF_SIZE 2000
#define MAX_SNAME 1000
pid_t ptyFork(int *masterFd, char *slaveName,struct termios *slaveTermios, const struct winsize *slaveWS)
{
int master,error;
master=posix_openpt(O_RDWR|O_NOCTTY);
*masterFd=master;
if(master==-1)
{
printf("master not opened\n");
return -1;
}
if(grantpt(master)==-1)
{
error=errno;
printf("grantpt error\n");
close(master);
}
if(unlockpt(master)==-1)
{
error=errno;
printf("unlockpt error\n");
close(master);
}
char *sname;
sname=ptsname(master);
slaveName=sname;

if(sname==NULL)
{
error=errno;
printf("ptsname\n");
close(master);
}
int k=fork();
if (k == -1) {
error = errno;
close(master);
return -1;
}
if(k!=0)
{
return k;
}
if(setsid()==-1)
{
printf("in setsid\n");
}
close(master);
int slavefd=open(sname,O_RDWR);
if(slavefd==-1)
{
printf("slavefd_open\n");
}
if(slaveTermios!=NULL)
{
if(tcsetattr(slavefd,TCSANOW,slaveTermios)==-1)
{
printf("tcsetattr\n");
}
}
if (slaveWS != NULL)
/* Set slave tty window size */
if (ioctl(slavefd, TIOCSWINSZ, slaveWS) == -1)
printf("ptyFork:ioctl-TIOCSWINSZ\n");
if (dup2(slavefd, STDIN_FILENO) != STDIN_FILENO)
printf("ptyFork:dup2-STDIN_FILENO\n");
if (dup2(slavefd, STDOUT_FILENO) != STDOUT_FILENO)
printf("ptyFork:dup2-STDOUT_FILENO\n");
if (dup2(slavefd, STDERR_FILENO) != STDERR_FILENO)
printf("ptyFork:dup2-STDERR_FILENO\n");
if (slavefd > STDERR_FILENO)
close(slavefd);
return 0;
}
int main(int argc,char *argv[])
{
char *shell;
ssize_t numRead;
pid_t childPid;
char buf[BUF_SIZE];
struct termios ttyOrig;
struct winsize ws;
char slaveName[MAX_SNAME];
int masterFd;
 tcgetattr(STDIN_FILENO, &ttyOrig);    
    ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
    childPid = ptyFork(&masterFd, slaveName, &ttyOrig, &ws);    
    if (childPid == 0) 
    {        
        shell = "/bin/bash";
        execlp(shell, shell, (char *) NULL);        
    }
char text[100];
while(1)
{
printf("enter command\n");
gets(text);
if(strcmp(text,"exit")==0)
exit(0);
printf("SENDING TO PTY\n");
int len=strlen(text);
text[len]='\n';
text[len+1]='\0';
    write(masterFd,text,strlen(text));
sleep(1);
  memset(&buf,0,sizeof(buf));
    numRead = read(masterFd, buf, BUF_SIZE);
    buf[numRead] = '\0';
    printf("RECIEVING FROM PTY %s\n",buf);
}
}
