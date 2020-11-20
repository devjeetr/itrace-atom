#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>makin another change
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <libgen.h>     // for dirname()/basename()
#include <time.h>

#define MAX 256
#define BLK 1024
#define CATBLKSIZE 4096

struct sockaddr_in saddr;
char *serverIP   = "127.0.0.1";
int   serverPORT = 1234;
int   sock;
struct stat mystat, *sp;
char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";
const char *local_cmd[] = { "lcat","lls","lcd","lpwd", "lmkdir", "lrmdir","lrm"};

char line[MAX]; // user input command line
char command[16], pathname[64]; // command and pathname strings


// Function provided by textbook
int find_local_cmd(char *command)
{
  int i = 0;
  while (local_cmd[i])
  {
    if (!strcmp(command, local_cmd[i]))
      return i; // found command: return index i
    i++;
  }
  return -1; // not found: return -1}
}


//function provided by PRELAB 5
int init()
{
    int n;

    printf("1. create a socket\n");
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("socket creation failed\n");
        exit(0);
    }

    printf("2. fill in server IP=%s, port number=%d\n", serverIP, serverPORT);
    bzero(&saddr, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr(serverIP);
    saddr.sin_port = htons(serverPORT);

    printf("3. connect to server\n");
    if (connect(sock, (struct sockaddr *)&saddr, sizeof(saddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    printf("4. connected to server OK\n");
}

//Built from Example code 8.2
int read_write(char* arg)
{
  int fd, i, m, n;
  char buf[CATBLKSIZE], dummy;
  fd = 0; //default sdin
  if(arg != NULL)
  {
    fd = open(arg,O_RDONLY);
    if(fd < 0)
      return -1;
  }

  while (n = read(fd,buf,CATBLKSIZE))
  {
    m = write(1,buf,n);
  }
}

//lcat implementation
int lcat()
{
  if (strlen(pathname) != 0)
  {
    char *s, *save;
    printf(" our path is %s ",pathname);
    for(s = strtok_r(pathname, " ", &save); s; s = strtok_r(NULL," ", &save))
    {
      read_write(s);
    }
  }
  else
  {
    read_write(NULL);
  }
}

//8.6.7 problem provided code
int ls_file(char *fname)
{
  struct stat fstat, *sp;
  int r, i;
  char ftime[64];
  sp = &fstat;
  if ( (r = lstat(fname, &fstat)) < 0)
  {
    printf("can't stat %s\n", fname);
    return -1;
  }
  if ((sp->st_mode & 0xF000) == 0x8000) // if (S_ISREG())
    printf("%c",'-');
  if ((sp->st_mode & 0xF000) == 0x4000) // if (S_ISDIR())
    printf("%c",'d');
  if ((sp->st_mode & 0xF000) == 0xA000) // if (S_ISLNK())
    printf("%c",'l');
  for (i=8; i >= 0; i--)
  {
    if (sp->st_mode & (1 << i)) // print r|w|x
      printf("%c", t1[i]);
    else
      printf("%c", t2[i]); // or print
  }
  printf("%4d %4d %4d %8d",sp->st_nlink,sp->st_gid,sp->st_uid,sp->st_size);
  // print time
  strcpy(ftime, ctime(&sp->st_ctime)); // print time in calendar form
  ftime[strlen(ftime)-1] = 0; // kill \n at end
  printf("%s ",ftime);
  // print name
  printf("%s", basename(fname)); // print file basename
  // print -> linkname if symbolic file
  if ((sp->st_mode & 0xF000)== 0xA000)
  {
    // use readlink() to read linkname
    char linkname[1024];
    ssize_t len = readlink(fname,linkname,sizeof(linkname) -1);
    if(len != -1)
    {
      linkname[len] = '\0';
      printf(" -> %s", linkname); // print linked name
    }
  }
  printf("\n");

}

//Implementation of 8.6.7 solution
int ls_dir(char *dname)
{
  struct dirent *ep;
  DIR *dp = opendir(dname);
  while(ep = readdir(dp))
    ls_file(ep->d_name);
}

//Implementation of 8.6.7 problem
int lls()
{
  struct stat mystat, *sp = &mystat;
  int r;
  char path[BLK], cwd[MAX];
  char filename[MAX] = "./";

  if (strlen(pathname) != 0)
    strcpy(filename,pathname);
  if (r = lstat(filename, sp) < 0)
  {
    printf("no such file %s\n", filename);
    return -1;
  }
  strcpy(path, filename);
  if (path[0] != '/')
  { // filename is relative : get CWD path
  getcwd(cwd, MAX);
  strcpy(path, cwd);
  strcat(path, "/");
  strcat(path,filename);

  }
  if (S_ISDIR(sp->st_mode))
    ls_dir(path);
  else
    ls_file(path);

  return 0;
}

//local cd implementation
int lcd()
{
  if (strlen(pathname) != 0)
  {
    if (chdir(pathname) != 0)
      printf("chdir to %s failed\n", pathname);
  }
  else
  {
    if (chdir(getenv("HOME")) != 0)
      printf("chdir to HOME failed\n");
  }

  return 0;
}

//local pwd implementation
int lpwd()
{
  char buf[MAX];
  if( getcwd(buf,MAX) == NULL)
  {
    printf("%s\n", ">lpwd error");
  }
  else
    printf("%s\n", buf);
  return 0;
}

//local mkdir implementation
int lmkdir()
{
  if (strlen(pathname) != 0)
  {
    if (mkdir(pathname,0755) != 0)
        printf("lmkdir %s failed\n", pathname);
  }
  else
    printf("lmkdir error, missing args\n");
  return 0;
}

//local rmdir implementation
int lrmdir()
{
  if (strlen(pathname) != 0)
  {
    if (rmdir(pathname) !=0)
      printf("lrmdir %s failed\n", pathname);
  }
  else
    printf("lrmdir error, missing args\n");
  return 0;
}

//local rmdir implementation
int lrm()
{
  if (strlen(pathname) != 0)
  {
    if (unlink(pathname) != 0);
      printf("lrm %s failed\n", pathname);
  }
  else
    printf("lrm error, missing args\n");
  return 0;
}

 //Main is an amalgamation of prelab 5 and Harry's lab 2 work.
int main(int argc, char *argv[], char *env[])
{
    int  n;
    //char line[MAX],
    char ans[MAX];


    init();
    //using the function pointer table
    int(*fptr[])() = {lcat,lls,lcd,lpwd, lmkdir, lrmdir,lrm};

    while (1)
    {

      printf("input a command line : ");
      fgets(line, MAX, stdin);
      line[strlen(line)-1] = 0;        // kill <CR> at end
      if (line[0]==0)                  // exit if NULL line
          exit(0);

      int ret = sscanf(line, "%s %[^\n]", command, pathname);

      const int index = find_local_cmd(command);
      //if we have a local command
      if(index != -1)
      {
        fptr[index]();
      }
      else
      {
        //we send it to the server instead
        // Send ENTIRE line to server
        n = write(sock, line, MAX);
        printf("client: wrote n=%d bytes; line=(%s)\n", n, line);

        // Read a line from sock and show it
        bzero(ans, MAX);
        n = read(sock, ans, MAX);
        printf("client: read  n=%d bytes; echo=(%s)\n",n, ans);
      }

      //clear the contents of pathname to repurpose in cases where a function can have no parameters. (permits functioning of fptr)/
      memset(&pathname[0], 0, sizeof(pathname));
    }
}
