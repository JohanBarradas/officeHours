#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "../include/simConstants.h"

/* Code executed by professor to simulate taking a break*/
static void take_break()
{
  printf("The professor is taking a break now.\n");
  sleep(5);
  assert( students_in_office == 0 );
  students_since_break = 0;
}

/* Check if professor can take a break*/
int checkBreak() 
{
  int ret = 0;
  sem_wait(&guard);
  if(students_since_break == 10 && students_in_office == 0)
    ret = 1;
  if ( ret )
  {
    on_break = 1;
    take_break();
    on_break = 0;
  }
  sem_post(&guard);

  return ret;
}

/*Professor Thread "Main" function assigned on OfficeHoursSim.c*/
void *professorthread(void *junk) //EDIT HERE
{

  printf("The professor arrived and is starting his office hours\n");
  /* Loop while waiting for students to arrive. */
  while (1)
  {

    while(!checkBreak())
      {
      }

  }
  pthread_exit(NULL);
}
