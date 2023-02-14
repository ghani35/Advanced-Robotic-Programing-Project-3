#define UTILS 


#include "./include/Common.h"





/**
 * @brief this function is used to create a new process. 
 * 
 * The function will fork() to create new child then the exec() system call to 
 * run either the konsole in case of foreground processes or one of the processes 
 * in the background, it returns the pid of the child to the parent(master in this case)
 * by modifying the variable pid, in case of failiure it writes to the log and sends a a SIGINT
 * it self because we know that this is the master.
 * @param comands commands to execute.
 * @param args the arguments to the newly created process.
 * @param pid pointer to the pid of the created process it might contain the error status in case of failure of the fork sys call.
 * @return int 
 */

int spawn(char* comands,char* args[],int *pid){
    pid_t pd = fork();
    *pid = pd;
    if (pd<0)
    {
        PrintLog("Failed to fork the process: %s\n",strerror(errno));
        kill(getpid(),SIGINT);
    }
    
    if (pd != 0)
    {
        return pd;
    }
    else
    {
        execvp(comands,args);
        PrintLog("Failed to execute the process: %s\n",strerror(errno));
        kill(getpid(),SIGINT);
        return -1;
    }
}

/**
 * @brief //create/modify(if file exist) a file to write the pid of the calling process.
 * 
 * 
 * @param Fname name of the File
 */
void WritePID(char* Fname){ 
     
    PrintLog("Saving the %s Process ID...\n",Fname);
    // if the dolder .data is not already created,create it.
    mkdir("./.data/",0666);
    char ff[20];
    sprintf(ff,"./.data/%s.txt",Fname);
    
    FILE *f;
    if ((f = fopen(ff,"w"))==NULL)
    {
        PrintLog("Error Writing PID: %s\n",strerror(errno));
        /* This is to terminate any process*/
        kill(getpid(),SIGINT);
    }
    

    int pid = getpid();
    fprintf(f,"%d",pid);
    //close file
    fclose(f);
}


/**
 * @brief Read the pid from a specific file 
 * 
 * @NOTE: This could be later interfaced with a database also.
 * @param Fname name of the file from which we get the pid. 
 * @return int the pid of in the requested file.
 */
int ReadPID(char* Fname){
    char ff[30];
    PrintLog("Reading PID of %s From %s...\n",Fname,ProcessNAme);
    sprintf(ff,"./.data/%s.txt",Fname);
    FILE* f;
    if((f = fopen(ff,"r"))== NULL){
        PrintLog("Error opening PID file: %s\n",strerror(errno));
        kill(getpid(),SIGINT);
    }
    int pid;
    fscanf(f,"%d",&pid);
    //close file
    fclose(f);
    return pid;
}


/// @brief This function creates a log file of the process calling it.
///  
/// @param Fname the name of the Log file.
void CreateLog(char* Fname){
    //create/modify(if file exist) a file to write the pid of the main process 
    mkdir("Logs/",0666);
    char ff[20];
    // 
    sprintf(ff,"Logs/%s.log",Fname);

    if((LogFile = fopen(ff,"w") ) == NULL){
        perror("Error in Creating Log File ...");
        kill(ReadPID(MASTERF),SIGINT);
    } 
    PrintLog("Created Log File...\n");
}
/// @brief To get the current time.
/// @return the current time in micro seconds.
long GetTimeNow(){
    auto end = chrono::high_resolution_clock::now();
    return chrono::system_clock::to_time_t(end);
}


/**
 * @brief Print a log message to the log file.
 * 
 * 
 * @param fmt formated message.
 * @param ... parammeters to format the message.
 */
void PrintLog(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsprintf(Printable, fmt ,args);
    string Timestring = string("[%ld] ") + Printable;
    fprintf(LogFile,Timestring.c_str(),GetTimeNow());FLF
    va_end(args);
}




