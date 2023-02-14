//TODO switch to terminator

#define MASTER 
#define PROCESS_MANAGMENT 
#include "./include/Common.h"


/*                                      Signal Handler                                   */

void handler(int sig) { 
  
  for (int i = 0; i < NUM_PROC; i++){
      PrintLog("Killing the process %s\n",CAA[i]);
      kill(ReadPID(CAA[i]), SIGINT);
    }
  
  while ((wpid = wait(&status)) > 0){
      printf("The process %d exited with status: %d\n",wpid,status);
  }
  fclose(LogFile);
  exit(EXIT_SUCCESS);
} 
/*                                      End Signal Handler                               */



int main(){
    CreateLog(ProcessNAme);
    PrintLog("Setting up Signal Handler...\n");
    signal(SIGINT, handler);
    WritePID(ProcessNAme);// Can be replaced with a database
    
    char tmp[50];
    for (int  i = 0; i < NUM_PROC; i++)
    {
      PrintLog("Creating the Process %s...\n",CAA[i]);
      sprintf(tmp,"./bin/%s",CAA[i]);
      if (i > FIRST_BACKGROUND_P)
      {
        args[0] = tmp;
        args[1] = NULL;
        spawn(V tmp,args,&children[i]);
      }else
      {
        args[2] = tmp;
        spawn(V"konsole",args,&children[i]); 
      }   
    }
    
    printf("Finished Creating Processes...\n");fflush(stdout);
    
    while ((wpid = wait(&status)) > 0){
      printf("The process %d exited with status: %d\n",wpid,status);
      kill(getpid(), SIGINT);
      

  }
    return 0;

}

