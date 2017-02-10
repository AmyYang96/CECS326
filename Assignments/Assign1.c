/**********************QUESTIONS*************************************************************
 1.Why can the sleeping thread print its periodic messages while the main thread is 
 waiting for keyboard input? Explain.
 
 Threads are contained in a process. As long as the process is running, both the sleeping
 and the main thread are also running in parallel with one another, unless one of the threads
 is cancelled. Threads are independent of each other.  That is why the sleeping thread can
 print its periodic messages while the main thread is waiting for keyboard input.
 
 
 
 2.Why can the main thread read input, kill the sleeping (second) thread, and print 
 a message while the sleeping thread is in the early part of one of its three-second sleeps? Explain.
 
 The main and sleeeping threads are independent of each other.
 **********************************************************************************************/


//import necessary header files
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

//second thread
static void *child (void *ignored)
{
    while(1) //loops infinitely
    {
        sleep(3); //waits 3 seconds before printing message
        printf("Child is done sleeping 3 seconds.\n"); //prints message
    }
    
    return NULL; //doesn't return anything
}

//main thread
int main(int argc, char const *argv[])
{
    pthread_t child_thread; //declare thread
    
    
    pthread_create(&child_thread, NULL, child, NULL); //run second thread

    
    while (getchar()) //get input to stop thread
    {
        pthread_cancel(child_thread); //stops the thread
        printf("Second thread is killed.\n");//print message that the thread is stopped
        break;
    }
    
    sleep(5);//waits 5 seconds before printing message
    printf("Parent is done sleeping 5 seconds.\n");//prints message

	return 0;//exiting main
}
