#define PROCESSB

#include "./include/processB_utilities.h"
#include "./include/Common.h"

void GetOrigin();
void GetBMP();

void signaleHandler(int sig){
    munmap(SharedMem, VMLEN);
    endwin();
    bmp_destroy(bmp);
    sem_close(Sem_Shm);
    sem_unlink(SemName);
    if (shm_unlink(ShmName) == 1) {
        PrintLog("Error removing %s\n", ShmName);
    }
    fclose(LogFile);
    exit(EXIT_SUCCESS);

}


int main(int argc, char const *argv[])
{
    CreateLog(ProcessNAme);
    PrintLog("Setting up Signal Handler...\n");
    signal(SIGINT, signaleHandler);
    WritePID(ProcessNAme);// Can be replaced with a database

    //init semaphore
    sleep(2);
    Sem_Shm = sem_open(SemName, 0);
    if (Sem_Shm == NULL) {
        PrintLog("Error in openning semaphore: %s",strerror(errno));
        kill(ReadPID(MASTERF),SIGINT);
    } 
    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize UI
    init_console_ui();
    //BMP
    bmp = bmp_create(WIDTH, HEIGHT, DEPTH);

    //Starting Shared Memory
    ShmFD = shm_open(ShmName, O_RDONLY, 0666);
    if (ShmFD == 1) {
        PrintLog("Shared memory segment failed\n");
        kill(ReadPID(MASTERF),SIGINT);
    }
    SharedMem = (int *) mmap(0, VMLEN, PROT_READ, MAP_SHARED, ShmFD, 0);

    if (SharedMem == MAP_FAILED) {
        PrintLog("Map failed\n");
        kill(ReadPID(MASTERF),SIGINT);
    }
    int (*VideoMemory)[HEIGHT] = (int (*)[HEIGHT]) SharedMem;

    // Infinite loop
    while (TRUE) {
 
        // Get input in non-blocking mode
        int cmd = getch();

        // If user resizes screen, re-draw UI...
        if(cmd == KEY_RESIZE) {
            if(first_resize) {
                first_resize = FALSE;
            }
            else {
                reset_console_ui();
            }
        }

        else {
            GetBMP();
            GetOrigin();
            if (c==1000)  c = 0;
            //delete the positions longer then 1000 step 
            mvaddch(Memory[c][1], Memory[c][0], ' ');
            //Save The Last 1000 Coordinates
            Memory[c][0] = x;
            Memory[c][1] = y;
            mvaddch(y, x, '0');
            
            refresh();
            c++;
        }
        if (cmd == 'q')
        {
            kill(ReadPID(MASTERF),SIGINT);
        }
        usleep(PERIODE);
    }

    endwin();
    return 0;
}

/**
 * @brief Get the Origin of the circle, from the bitmap.
 * 
 */
void GetOrigin(){
    int tmp = 0,count = 0;
    for (int j = 0; j < HEIGHT; j++)
    {
        count = 0;
        for (int i = 0; i < WIDTH; i++)
        {
            if (bmp_get_pixel(bmp,i,j)->blue==255 && tmp==0)
            {
               count = 0;
            }
            
            if (bmp_get_pixel(bmp,i,j)->blue==0 && tmp==255)
            {
                if (count >= 2*(RADUIS-1))
                {
                    x = i/20;
                    y = j/20;
                    return;
                }

            }
            if (bmp_get_pixel(bmp,i,j)->blue==255 && tmp==255)
            {
               count++;
            }
            tmp = bmp_get_pixel(bmp,i,j)->blue;
        }
    }
}

/**
 * @brief This function creats the bitmap from the 2d array representation in the shared memory. 
 * so it is a mapping from a 2d representation to a bitmap.
 * semaphores is used to sync the access to the shared memory.
 */
void GetBMP(){
    sem_wait(Sem_Shm);
    int (*VideoMemory)[HEIGHT] = (int (*)[HEIGHT]) SharedMem;
    for (int j = 0; j < HEIGHT; j++)
    {
        for (int i = 0; i < WIDTH; i++)
        {
           if (VideoMemory[i][j] == 1)
           {
                bmp_set_pixel(bmp,i,j,BluePixel);
           }else
           {
            bmp_set_pixel(bmp,i,j,ZeroPixel);
           }
           
        }
    }
    sem_post(Sem_Shm);
    
}

