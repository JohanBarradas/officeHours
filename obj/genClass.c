#include "../include/simConstants.h"
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

//cannot have class_leave here since its a static 
int checkPermission ( int class) //check permission for students to enter the office
{ //class a is 0 and class b is 1
  int ret = 0;
  int statement;
  sem_wait(&guard);
  if(class == CLASSB) //class b is trying to enter
  {
    statement = students_in_office < 3&&classa_inoffice == 0 && !on_break
      &&students_since_break <10; //this must always be checked when a thread B enters
    if(statement&&(class_counterB < 5||classa_total==0)) ret = 1;
    if (ret) //allowed to enter
    {
      class_counterA = 0;
      class_counterB += 1;
      students_in_office += 1;
      students_since_break += 1;
      classb_inoffice += 1;
    }
  }
  else if(class == CLASSA) //class a is trying to enter
  {
    statement = students_in_office < 3&&classb_inoffice == 0 && !on_break
      &&students_since_break <10; //this must always be checked when a thread A enters
    if(statement&&(classb_total ==0||class_counterA < 5)) ret = 1;
    if( ret )
    {
      class_counterB = 0;
      class_counterA += 1; //at least one class A has entered the office
      students_in_office += 1;
      students_since_break += 1;
      classa_inoffice += 1;
    }
  }
  sem_post(&guard);
  return ret; //will return 0 if student lacks permission, 1 if it can enter
}

void class_enter(int classNo){
	while(!checkPermission(classNo)){ //if !0 = 1 and if !1 = 0
	}
	sem_wait(&mutex); //cant enter, have semaphore waiting 
}




