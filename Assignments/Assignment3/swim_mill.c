//
//  swim_mill.c
//  
//
//  Created by Amy Yang on 3/9/17.
//
//
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <signal.h> // For signal
#include <stdbool.h> // For bool
#include <time.h>

bool end = false;
struct criticalRegion /* Defines "structure" of shared memory */
{
    int manager[20]; //holds the location of fish and pellets
    int pellet_counter; //counts pellets in swim  mill
};

struct criticalRegion *sharedMemory;//pointer to critical region

int sharedMemoryID; //shared memory  id

#define length 10 //grid length

//prototypes
void interruptSignal();
void exitOnAlarm();
void raiseAlarm();
void printGrid(char A[length][length]);
void updateGrid(char A[length][length]);
void convertNumtoCoord(int position, int index, char swimMill_grid[length][length]);

int main(int argc, char  *argv[])
{
	
   
    //alarm(10);

    // declare 2D array of size 10 by 10
    char swimMill_grid[10][10];
    
    
/*******************Create shared memory segment******************************/
    sharedMemory=malloc(sizeof (struct criticalRegion)); //Allocate memory for struct
    
    key_t key=5678; //key for shared memory
  
    //create shared memory segment. exit on error
    if ((sharedMemoryID = shmget(key, sizeof(sharedMemory), IPC_CREAT | 0666)) <0)
    {
        perror("shmget");
        exit(1);
    }
    
    //Attach segment to data space
    sharedMemory=shmat(sharedMemoryID,NULL,0);
    
/************************************************************/

    for (int i = 0; i < 20; i++)//initialize locations to be "empty"
    {
        sharedMemory->manager[i]=-1;
    }
    
    sharedMemory->pellet_counter=0;//initialize pellet counter
	signal(SIGINT, interruptSignal);//watch for interrupt signal
	signal(SIGALRM, exitOnAlarm);//watch for alarm signa;
	alarm(30);
	
    pid_t fishPID = fork();//fork fish process
    if(fishPID==0)
    {
		execv("fish", argv);
    }
	
	pid_t printSwimMill = fork(); //prints grid
	if (printSwimMill==0)
	{
		while (1)
		{
			updateGrid(swimMill_grid);//update and print grid
			printf("\n");
			sleep(1);
		}
	}
	
	while(1)
	{
		pid_t pellet_pid=fork(); //fork pellets
		if(pellet_pid==0)
		{
			execv("pellet",argv);
		}
		
		while (sharedMemory->pellet_counter>19)//limits number of pellets
		{
			sleep(2);
		}
		//drop pellet at different times
		srand(time(NULL));
	   sleep(rand()%10);
	}
}



//coonvert integer location to grid coordinate
void convertNumtoCoord(int position, int index, char swimMill_grid[length][length])
{
    if (position >0)
    {
        int x = position%10; //one's digit is x coordinate
        int y  = (position - x)/10;//ten's is y coordinate
        
        if (index == 0)
        {
            swimMill_grid [y][x]='f'; //label fish
        }
        else
        {
            swimMill_grid [y][x]='p';//label pellet
        }
    }
}

//print grid
void printGrid(char swimMill_grid[length][length])
{
    for (int i=0; i < length; i++)
    {
        printf("\n%d|",i);
        for (int j=0; j <length; j++)
        {
            printf("   %c",swimMill_grid[i][j]);
            
        }
    }
    printf("\n     -------------------------------------");
    printf("\n     0   1   2   3   4   5   6   7   8   9\n\n");
    printf("Number of pellets: %d",sharedMemory->pellet_counter);
}


//update grid and print it
void updateGrid(char swimMill_grid[length][length])
{
    
    for (int i = 0; i < 10; i++) //clear fish and pellet locations to avoid repeats
	{
        for (int j = 0; j < 10; j++)
		{
            swimMill_grid[i][j] = '~';
        }
    }
    
    for (int i=0; i<20; i++)//put fish and pellets in grid
    {
        convertNumtoCoord(sharedMemory->manager[i], i, swimMill_grid);
    }
    printGrid(swimMill_grid); //print grid
    
}


//Exits when there is a keyboard interrupt ^C
void interruptSignal()
{
    printf("\nSwim mill died due to interruption. Swim mill PID:  %d ", getpid());
	
	//Detach shared memory
	shmdt(sharedMemory);
    shmctl(sharedMemoryID, IPC_RMID, NULL);
	
	exit(0);//exit swim mill

}

//Exit after alarm goes off
void exitOnAlarm()
{
    printf("\nSwim mill died after 30 seconds. Swim mill PID:  %d \n", getpid());
	
	//Detach shared memory
	shmdt(sharedMemory);
    shmctl(sharedMemoryID, IPC_RMID, NULL);
	
	kill(0,SIGTERM);//kill fish and pellets
	exit(0);//exit swim mill

}



