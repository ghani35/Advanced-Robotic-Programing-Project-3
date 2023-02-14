#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <bmpfile.h>
#include <math.h>
#include <wait.h>
#include <fstream>
#include <sstream>
#include <sys/time.h>
#include <signal.h>
#include <chrono>
#include <iostream>
#include <cstring>
#include <stdarg.h>
#include <semaphore.h> 


using namespace std;

/*                                   MACROS                              */
#define V (char*)
#define FLF fflush(LogFile);
#define NUM_PROC 2
#define ProcessAF V"ProcessA"
#define ProcessBF V"ProcessB"
#define MASTERF V"master"


#define WIDTH 1600
#define HEIGHT 600
#define DEPTH  4 
#define RADUIS 20
#define VMLEN WIDTH * HEIGHT * sizeof(int)
 
/*                               End MACROS                              */
/*                                   Function Declarations*/
void WritePID(char* Fname);
int ReadPID(char* Fname);

int spawn(char* comands,char* args[],int *pid);

void CreateLog(char* Fname);
void PrintLog(const char *fmt, ...);
long GetTimeNow();
/*                               End Function Declarations*/

#ifndef UTILS

char Printable[100];
FILE *LogFile;

// For Semaphore
sem_t *Sem_Shm;
const char SemName[] = "/Sem_SharedMem";
 
//For Shared Memory
const char * ShmName = "/SharedMem";
int ShmFD;
int * SharedMem;

//For bit map
bmpfile_t *bmp;
rgb_pixel_t BluePixel = {255, 0, 0, 0},ZeroPixel = {0,0,0,0};



#else

extern char* ProcessNAme;
extern FILE *LogFile;
extern char Printable[100];

#endif

#ifdef MASTER 
#define FIRST_BACKGROUND_P 1

char* ProcessNAme = MASTERF;

#endif


#ifdef PROCESS_MANAGMENT
char *CAA[NUM_PROC] = {ProcessAF,ProcessBF};//Commands And Arguments 
char *args[4] ={V"konsole",V"-e",V"tmp" ,V NULL};
int children[NUM_PROC];
pid_t wpid;
int status;
#endif


#ifdef PROCESSA 

char* ProcessNAme = ProcessAF;
#define PERIODE 5000

enum ExecutionModality {
    Normal,
    Server,
    Client
};

/* to convert the user choice to enum dt*/
std::istream& operator>>(std::istream& modenum, ExecutionModality& mode)
{
    int tmp;
    modenum >> tmp;
    mode = static_cast<ExecutionModality>(tmp);

    return modenum;
}
int sockfd;
ExecutionModality modality;
#endif

#ifdef PROCESSB 

#define PERIODE 2000

int x=0,y=0;
char* ProcessNAme = ProcessBF;
int Memory[1000][2];
int c = 0;
#endif