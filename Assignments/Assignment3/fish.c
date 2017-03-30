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
void moveTowardNearestPellet();
void interruptSignal(); //Exits when there is a keyboard interrupt ^C
void exitOnAlarm();

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
    signal(SIGALRM, exitOnAlarm);
    
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
  
    //generate random location
    srand(time(NULL));
    sharedMemory->manager[0]=rand()%10 + 90;//always in the last row

    while (1)
    {
        moveTowardNearestPellet();//move fish right or left
        sleep(1);
    }
}

/*
 Scans the locaton and move toward nearest pellet
 */
void moveTowardNearestPellet()
{
    int fishLoc = sharedMemory->manager[0];//fishh location
    int max =0;//the max location will give closest row
    int nearestInColumn=0;//nearest location horizontally
    int nearestInColumnDist=11;//asssume horizontal distance to be far
    
    for (int i=1; i<20; i++)//iterate through shareed memory
    {
        if(sharedMemory->manager[i]>=0 &&sharedMemory->manager[i]<100)//look for locations that are not out of the grid
        {
            if (sharedMemory->manager[i]/10>=max)
            {
                max =sharedMemory->manager[i];//nearest row
            }
            
            if(abs(sharedMemory->manager[i]%10-fishLoc%10) <=nearestInColumnDist)
            {
                nearestInColumn=sharedMemory->manager[i];//location withnearest col
                nearestInColumnDist = abs(sharedMemory->manager[i]%10-fishLoc%10); //closest col distance
            }
        }
    }
    
    printf("\nNearest row val: %d", max);
    printf("\nNearest col val: %d",nearestInColumn);
    int nearestRowDist = max/10-9;//vertical  distance

    if(abs(nearestRowDist)< nearestInColumnDist) //if vertical distance is less than horizontal distance
    {
        if (max%10-fishLoc>0 && fishLoc!=99) //if pellet is on the right
        {
            sharedMemory->manager[0]=sharedMemory->manager[0]+1; //move right
            
        }
        else if (max%10-fishLoc<0 && fishLoc!=90)//if pellet is to the left
        {
            sharedMemory->manager[0]=sharedMemory->manager[0]-1; //move left
        }
    }
    else
    {
        if(nearestInColumn%10-fishLoc%10>0 && fishLoc!=99)//if pellet is on the right
        {
            sharedMemory->manager[0]=sharedMemory->manager[0]+1; //move right
        }
        else if (nearestInColumn%10-fishLoc%10>0 && fishLoc!=90)//if pellet is to the left
        {
            sharedMemory->manager[0]=sharedMemory->manager[0]-1;//move left
        }
    }
}

//Exits when there is a keyboard interrupt ^C
void interruptSignal()
{
    printf("\nFish died due to interruption. Fish PID:  %d ", getpid());
    
    //Detach shared memory
    shmdt(sharedMemory);
    shmctl(sharedMemoryID, IPC_RMID, NULL);
    exit(0);//exit
    
}

void exitOnAlarm()
{
    printf("\nFish after 30 seconds. Fish PID:  %d ", getpid());
    
    //Detach shared memory
    shmdt(sharedMemory);
    shmctl(sharedMemoryID, IPC_RMID, NULL);
    exit(0);//exit
    
}

