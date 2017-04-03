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
#include <signal.h>
#include <time.h>

//prototypes
void down(int locationIndex);
void interruptSignal(); //Exits when there is a keyboard interrupt ^C
void exitOnAlarm(); //Exit after alarm goes off

struct criticalRegion /* Defines "structure" of shared memory */
{
    int manager[20]; //holds the location of fish and pellets
    int pellet_counter; //counts pellets in swim  mill
};

struct criticalRegion *sharedMemory;//pointer to critical region

int sharedMemoryID; //shared memory  id
int locationIndex; //the index of the array in the shared memory that holds the process's location

int main(int argc, char  *argv[])
{
    signal(SIGINT, interruptSignal);
    signal(SIGTERM, exitOnAlarm);
    /*******************Create shared memory segment******************************/
    sharedMemory=malloc(sizeof (struct criticalRegion)); //Allocate memory for struct
    
    key_t key=5678; //key for shared memory
        
    //create shared memory segment. exit on error
    if ((sharedMemoryID = shmget(key, sizeof(sharedMemory), 0666)) <0)
    {
        perror("shmget");
        exit(1);
    }
    
    //Attach segment to data space
    sharedMemory=shmat(sharedMemoryID,NULL,0);
    /************************************************************/
    
    locationIndex=-1; //initialize as index 1
    
    for (int i=1; i<20; i++)//iterate through array to find empty slot
    {
        if(sharedMemory->manager[i]==-1)//empty slot found, break out of loop
        {
            locationIndex=i;
            break;
        }
    }
	if(locationIndex==-1)
	{
		//Detach shared memory
		shmdt(sharedMemory);
		shmctl(sharedMemoryID, IPC_RMID, NULL);
		
		exit(0);
	}
	sharedMemory->pellet_counter+=1;//increment counter
    //generate random location
    srand(time(NULL));
    sharedMemory->manager[locationIndex]=rand()%10 + 10*(rand()%10);
    
    down(locationIndex);//float down
   
    //if pellet and fish coincide,exit
    if (sharedMemory->manager[locationIndex] + 10 == sharedMemory->manager[0] ||
        sharedMemory->manager[locationIndex] + 10 == sharedMemory->manager[0] + 1 ||
        sharedMemory->manager[locationIndex] + 10 == sharedMemory->manager[0]-1)
    {
        sharedMemory->manager[locationIndex]=-1;// "free" slot
        printf("\nPellet was eaten, now exiting grid. Pellet PID:  %d ", getpid());
        sharedMemory->pellet_counter-=1;//decrease counter
        exit(0);//terminate process
    }
    
    //if pellet in last row, exit
    if (sharedMemory->manager[locationIndex]+10>=109)
    {
        sharedMemory->manager[locationIndex]=-1;//"free"slot
        printf("\nPellet was missed, now exiting grid. Pellet PID:  %d ", getpid());
        sharedMemory->pellet_counter-=1;//decrease counter
        exit(0);//terminate process
    }
    
}

/*Moves pellet downstream*/
void down(int locationIndex)
{
    while(sharedMemory->manager[locationIndex] + 10<109) //move un
    {
        if (sharedMemory->manager[locationIndex] + 10 == sharedMemory->manager[0])
        {
            break;//if next move will coincide with fish, stop
        }
        else
        {
            sharedMemory->manager[locationIndex]+=10;//move down
            sleep(2);
        }
    }
}

//Exits when there is a keyboard interrupt ^C
void interruptSignal()
{
    printf("\nPellet died due to interruption. Pellet PID:  %d \n", getpid());
    
    //Detach shared memory
    shmdt(sharedMemory);
    shmctl(sharedMemoryID, IPC_RMID, NULL);
    exit(0);//exit
    
}

//Exit after alarm goes off
void exitOnAlarm()
{
    printf("\nPellet died after 30 second alarm on swim mill. Pellet PID:  %d \n", getpid());
    
    //Detach shared memory
    shmdt(sharedMemory);
    shmctl(sharedMemoryID, IPC_RMID, NULL);
	exit(0);//exit
    
}
