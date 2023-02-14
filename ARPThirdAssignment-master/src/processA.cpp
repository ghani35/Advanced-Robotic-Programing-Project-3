#define PROCESSA
#include "./include/processA_utilities.h"
#include "./include/Common.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h> 


void DrawCircle(int px,int py);
void ResetBMP();
void SendBMP();
int runNormal();
void ExitNormal();
void ExitServer();
void ExitClient();


/**
 * @brief 
 * A signal Handler when we press ctrl+c, to safely exit the current running mode, and deallocate the
 * taken resources.
 */
void signaleHandler(int sig){
    switch (modality) {
        case Normal:
            ExitNormal();
            break;
        case Server:
            ExitServer();
            break;
        case Client:
            ExitClient();
            break;
    }
    munmap(SharedMem, VMLEN);
    
    bmp_destroy(bmp);
    sem_close(Sem_Shm);
    sem_unlink(SemName);
    if (shm_unlink(ShmName) == 1) {
        PrintLog("Error removing %s\n", ShmName);
    }
    fclose(LogFile);
    exit(EXIT_SUCCESS);
    
}


/**
 * @brief 
 * A function to draw a circle in the bit map, starting from the cordinates of the origin, the 
 * Raduis of the circle is configurable, and can be modified from the macro RADUIS.
 * 
 * @param px the x component of the origin of the circle.
 * @param py the x component of the origin of the circle.
 */
void DrawCircle(int px,int py){
    // this function will draw a circle in the bit map
    for(int y = -RADUIS; y <= RADUIS; y++) {
        for(int x = -RADUIS; x <= RADUIS; x++) {
            if(sqrt(x*x + y*y) < RADUIS) {
                bmp_set_pixel(bmp, px + x, py + y, BluePixel);
            }   
        }
    }
}

/**
 * @brief This function edites the 2d array representation of the bit map in the shared memory. 
 * so it is a mapping from a bitmap to the 2d representation.
 * semaphores is used to sync the access to the shared memory.
 */
void SendBMP(){
    
    sem_wait(Sem_Shm);
    int (*VideoMemory)[HEIGHT] = (int (*)[HEIGHT]) SharedMem;
    for (int j = 0; j < HEIGHT; j++)
    {
        for (int i = 0; i < WIDTH; i++)
        {
           if (bmp_get_pixel(bmp,i,j)->blue == 255)
           {
                VideoMemory[i][j] = 1;
           }
           else
           {
                VideoMemory[i][j] = 0;
           }
           
        }
    }
    sem_post(Sem_Shm);
}

/**
 * @brief This function is used to clear the bitmap.
 * 
 */
void ResetBMP(){
    //to clear the bitmap
    for (int j = 0; j < HEIGHT; j++)
    {
        for (int i = 0; i < WIDTH; i++)
        {
            bmp_set_pixel(bmp,i,j,ZeroPixel);
        }
        
    }
    
}


/**
 * @brief 
 * This is a function that runs when in Normal mode, Process A of preivious assignment.
 * 
 */
int runNormal() {
   // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize UI
    init_console_ui(); 

    
    PrintLog("Select Normal Mode..\n");
    while (TRUE)
    {
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

        // Else, if user presses print button...
        else if(cmd == KEY_MOUSE) {
            if(getmouse(&event) == OK) {
                if(check_button_pressed(print_btn, &event)) {
                    char filename[20];
                    sprintf(filename,"%s.png",ProcessNAme);
                    // save snapshot
                    bmp_save(bmp, filename);
                }
            }
        }

        else if(cmd == KEY_LEFT || cmd == KEY_RIGHT || cmd == KEY_UP || cmd == KEY_DOWN) {
            // If input is an arrow key
            ResetBMP();
            move_circle(cmd);
            draw_circle();
            DrawCircle(circle.x*20,circle.y*20);
            SendBMP();
            
        }

        if (cmd == 'q')
        {
            goto exit;
        }
        usleep(PERIODE);
    }
    exit:
        ExitNormal();
    return 0;
}


/**
 * @brief 
 * This is a function that runs when in Server mode, creates a socket that listens on port 12345 
 * for the client to send commands and then executes them.
*/
int runServer() {
    int newsockfd, portno;
    socklen_t clilen;
    int buff;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    int first_resize = TRUE;

    // Initialize UI
    init_console_ui(); 
    // create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("ERROR opening socket\n");
        return -1;
    }
    PrintLog("Select Server Mode..\n");
    // clear address structure
    bzero((char *) &serv_addr, sizeof(serv_addr));

    // set port number
    portno = 12345;

    // setup the host_addr structure for use in bind call
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    // bind the socket to the address and port
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("ERROR on binding\n");
        return -1;
    }

    // listen for incoming connections
    listen(sockfd,5);
    clilen = sizeof(cli_addr);

    // accept a connection
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) {
        cout << "ERROR on accept" << endl;
        return -1;
    }

    while(true) {
        // Get input in non-blocking mode, this is done in case user resizes the window
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
        // clear the buffer
        buff = 0;
        
        // read data from client
        n = read(newsockfd,&buff,sizeof(int));
        if (n < 0) {
            goto Continue;
        }
        // if the client sends a q means that they want to quit
        if (buff =='q')
        {
            break;
        }
        // else update the window accordingly
        else if(buff == KEY_LEFT || buff == KEY_RIGHT || buff == KEY_UP || buff == KEY_DOWN) {
            // If input is an arrow key
            ResetBMP();
            move_circle(buff);
            draw_circle();
            DrawCircle(circle.x*20,circle.y*20);
            SendBMP();
            
        }
        Continue:
        usleep(PERIODE);
    }
    ExitServer();
    return 0;
}

/**
 * @brief 
 * This is a function that runs when in Client mode, listens to the key presses, 
 * displays the ball, and sends the commands to the server.

*/
int runClient(){
    int newsockfd, portno;
    socklen_t clilen;
    int buff;
    char addr[20];
    struct hostent *server;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    int first_resize = TRUE;

    
    PrintLog("Select Client Mode..\n");
   // create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("ERROR opening socket\n");
        return 0;
    }

    // clear address structure
    bzero((char *) &serv_addr, sizeof(serv_addr));

    // get server IP and port number from user
    printf("Enter server IP: ");
    cin >> addr;
    printf("Enter port number: ");
    cin >> portno;
    server = gethostbyname(addr);

    // setup the server address structure
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);

    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    
    // connect to the server
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("ERROR connecting\n");
        return 0;
    }
    // Initialize UI
    init_console_ui(); 

    while(true) {
        // clear the buffer
        buff = 0;
        
        buff = getch();
        
        if(buff == KEY_RESIZE) {
                    if(first_resize) {
                        first_resize = FALSE;
                    }
                    else {
                        reset_console_ui();
                    }
                }

        // send message to server
        int n = write(sockfd, &buff, sizeof(int));
        if (n < 0) {
            PrintLog("ERROR writing to socket\n");
            break;
        }
        else if(buff == KEY_LEFT || buff == KEY_RIGHT || buff == KEY_UP || buff == KEY_DOWN) {
            // If input is an arrow key
            move_circle(buff);
            draw_circle();
            
        }
        if(buff=='q'){
            break;
        }
        usleep(PERIODE);  
    }
    ExitClient();
    return 0;
}

/**
 * @brief 
 * a function to exit Normal mode.
 */
void ExitNormal(){
    reset_console_ui();
    endwin();
    PrintLog("Exiting Normal Mode..\n");
}

/**
 * @brief 
 * a function to exit Server mode.
 */
void ExitServer(){
    close(sockfd);
    reset_console_ui();
    endwin();
    PrintLog("Exiting Server Mode..\n");
}
/**
 * @brief 
 * a function to exit Client mode.
 */
void ExitClient(){
    close(sockfd);
    reset_console_ui();
    endwin();
    PrintLog("Exiting Client Mode..\n");
}


int main(int argc, char *argv[])
{
    CreateLog(ProcessNAme);
    PrintLog("Setting up Signal Handler...\n");
    signal(SIGINT, signaleHandler);
    WritePID(ProcessNAme);// Can be replaced with a database
    
    //init semaphor 
    Sem_Shm = sem_open(SemName, O_CREAT, S_IRUSR | S_IWUSR, 1);
    if (Sem_Shm == NULL) {
        PrintLog("Error in openning semaphore: %s",strerror(errno));
        kill(ReadPID(MASTERF),SIGINT);
    }

    // Starting The shared memory 

    ShmFD = shm_open(ShmName, O_CREAT | O_RDWR, 0666);

    if (ShmFD == 1) {
        PrintLog("Shared memory segment failed\n");
        kill(ReadPID(MASTERF),SIGINT);
    }
    // resizing the memory 
    ftruncate(ShmFD, VMLEN);
    if (SharedMem == MAP_FAILED) {
        PrintLog("Map failed\n");
        kill(ReadPID(MASTERF),SIGINT);
    }
    //maping the shared memory, this will give us the pointer to the shared memory
    SharedMem = (int*) mmap(0, VMLEN, PROT_READ | PROT_WRITE, MAP_SHARED, ShmFD, 0);
    // clearing wny garbage data, and setting it to 0, as our background is always black
    memset(SharedMem,0,VMLEN);
    //create the Bit Map
    bmp = bmp_create(WIDTH, HEIGHT, DEPTH);

    while (true){
        // get execution modality from user
        cout << "Enter execution modality (0 for normal, 1 for server, 2 for client).\nctrl+c to exit. \nyour choice: ";
        cin >> modality;

        // run the program based on the selected modality
        switch (modality) {
            case Normal:
                runNormal();
                break;
            case Server:
                runServer();
                break;
            case Client:
                runClient();
                break;
            default:
                cout << "Invalid execution modality" << endl;
        }
    }
    
    return 0;
}


