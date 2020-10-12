#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include "../include/simConstants.h"

//prototyping function, so that we can call the function from a separate c file
void *professorthread(void *junk);
void class_enter(int classNo);
void ask_questions(int t);
void class_leave(int classNo);
int checkPermission(int class); //  may not need prototype for this function


// define variables for the simulation to work across files 
int students_in_office = 0;
int classa_inoffice = 0;
int classb_inoffice = 0;
int students_since_break = 0;
int class_counterA = 0; //for 5 class A policy
int class_counterB = 0; //for 5 class B policy
int on_break = 0; //variable for checking if prof is on break
int classb_total = 0; //total of students from class A
int classa_total = 0;  //total of students from class B

static void classa_leave()
{
  sem_wait(&guard);
  students_in_office -= 1;
  classa_inoffice -= 1;
  classa_total -= 1; //reduce my total students of class a in line
  sem_post(&guard);

  sem_post(&mutex); //up semaphore
}

static void classb_leave()
{
  sem_wait(&guard);
  students_in_office -= 1;
  classb_inoffice -= 1;
  classb_total -= 1;//reduce my total students of class b in line
  sem_post(&guard);
  sem_post(&mutex); //up semaphore
}

/*  main functions for student threads  */

void* classa_student(void *si)
{
  student_info *s_info = (student_info*)si;

  /* enter office */
  class_enter(CLASSA);
  //classa_enter();

  printf("Student %d from class A enters the office\n", s_info->student_id);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classb_inoffice == 0 );

  /* ask questions  --- do not make changes to the 3 lines below*/
  printf("Student %d from class A starts asking questions for %d minutes\n",
   s_info->student_id, s_info->question_time);
  ask_questions(s_info->question_time);
  printf("Student %d from class A finishes asking questions and prepares to leave\n",
   s_info->student_id);

  /* leave office */


  printf("Student %d from class A leaves the office\n", s_info->student_id);
  //classa_leave();
  class_leave(CLASSA);
  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);

  pthread_exit(NULL);
}

void* classb_student(void *si)
{
  student_info *s_info = (student_info*)si;

  /* enter office */
  //classb_enter();
  
  class_enter(CLASSB);
  
  printf("Student %d from class B enters the office\n", s_info->student_id);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);
  assert(classa_inoffice == 0 );

  printf("Student %d from class B starts asking questions for %d minutes\n",
    s_info->student_id, s_info->question_time);
  ask_questions(s_info->question_time);
  printf("Student %d from class B finishes asking questions and prepares to leave\n",
    s_info->student_id);

  /* leave office */


  printf("Student %d from class B leaves the office\n", s_info->student_id);
  //classb_leave();
  class_leave(CLASSB);
  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);

  pthread_exit(NULL);
}

/* Main function sets up simulation and prints report
 * at the end.
 */
int main(int nargs, char **args)
{
  int i;
  int result;
  int student_type;
  int num_students;
  void *status;
  pthread_t professor_tid;
  pthread_t student_tid[MAX_STUDENTS];
  student_info s_info[MAX_STUDENTS];

  if (nargs != 2)
  {
    printf("Usage: officehour <name of inputfile>\n");
    return EINVAL;
  }

  num_students = initialize(s_info, args[1]);
  if (num_students > MAX_STUDENTS || num_students <= 0)
  {
    printf("Error:  Bad number of student threads. "
           "Maybe there was a problem with your input file?\n");
    return 1;
  }

  printf("Starting officehour simulation with %d students ...\n",
    num_students);
  sem_init(&mutex, 0, 3); //initialize quaternary semaphore, it includes everyone in office
  sem_init(&guard, 0, 1);
  result = pthread_create(&professor_tid, NULL, professorthread, NULL); //create prof thread

  if (result) //pthread_create returns 0 if sucessful
  {
    printf("officehour:  pthread_create failed for professor: %s\n", strerror(result));
    exit(1);
  }

  for (i=0; i < num_students; i++)
  {

    s_info[i].student_id = i;
    sleep(s_info[i].arrival_time);

    student_type = random() % 2;

    if (student_type == CLASSA)
    {
      classa_total++;
      result = pthread_create(&student_tid[i], NULL, classa_student, (void *)&s_info[i]);
    }
    else // student_type == CLASSB
    {
      classb_total++;
      result = pthread_create(&student_tid[i], NULL, classb_student, (void *)&s_info[i]);
    }

    if (result)
    {
      printf("officehour: thread_fork failed for student %d: %s\n",
            i, strerror(result));
      exit(1);
    }
  }
  //here is where the magic begins
  /* wait for all student threads to finish */
  for (i = 0; i < num_students; i++)
  {
    pthread_join(student_tid[i], &status);
  }
  sem_destroy(&mutex);
  sem_destroy(&guard);
  /* tell the professor to finish. */
  pthread_cancel(professor_tid);

  printf("Office hour simulation done.\n");

  return 0;
}
