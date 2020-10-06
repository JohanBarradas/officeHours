/*

  Name: Johan Barradas
  ID: 1001354711

*/
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

/*** Constants that define parameters of the simulation ***/

#define MAX_SEATS 3        /* Number of seats in the professor's office */
#define professor_LIMIT 10 /* Number of students the professor can help before he needs a break */
#define MAX_STUDENTS 1000  /* Maximum number of students in the simulation */

#define CLASSA 0
#define CLASSB 1

/* TODO */
static int class_counterA; /*Class currently on the office*/
static int class_counterB;
static int on_break;
// 0 is class a, 1 is class b, -1 is empty class
sem_t mutex; // Mutex to assure only 3 students enter office
sem_t guard; // Mutex to guard segments of the critical sections of class a and b

static int students_in_office;/* Total numbers of students currently in the office */
static int classa_inoffice;   /* Total numbers of students from class A currently in the office */
static int classb_inoffice;   /* Total numbers of students from class B in the office */
static int students_since_break = 0;
static int classa_total;
static int classb_total;

typedef struct
{
  int arrival_time;  // time between the arrival of this student and the previous student
  int question_time; // time the student needs to spend with the professor
  int student_id;
} student_info;

static int initialize(student_info *si, char *filename)
{
  students_in_office = 0;
  classa_inoffice = 0;
  classb_inoffice = 0;
  students_since_break = 0;
  class_counterA = 0; //for 5 class A policy
  class_counterB = 0; //for 5 class B policy
  on_break = 0; //variable for checking if prof is on break
  classb_total = 0; //total of students from class A
  classa_total = 0;  //total of students from class B

  /* Read in the data file and initialize the student array */
  FILE *fp;

  if((fp=fopen(filename, "r")) == NULL)
  {
    printf("Cannot open input file %s for reading.\n", filename);
    exit(1);
  }

  int i = 0;
  while ( (fscanf(fp, "%d%d\n", &(si[i].arrival_time), &(si[i].question_time))!=EOF)
           && i < MAX_STUDENTS )
  {
    i++;
  }

 fclose(fp);
 return i;
}

/* Code executed by professor to simulate taking a break
 * You do not need to add anything here.
 */
static void take_break()
{
  printf("The professor is taking a break now.\n");
  sleep(5);
  assert( students_in_office == 0 );
  students_since_break = 0;
}

int checkBreak() //check if professor can take a break
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
int checkPermission ( int class) //check permission for B to enter the office
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

void classa_enter()
{

  while(!checkPermission(CLASSA) ) // if 1 is returned loop breaks and access granted
  {
  }

  sem_wait(&mutex);

}

void classb_enter()
{
  while(!checkPermission(CLASSB)) //if !0 = 1 and if !1 = 0
  {
  }
  sem_wait(&mutex);//cant enter, have semaphore waiting

}

static void ask_questions(int t)
{
  sleep(t);
}

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

/* Main code for class A student threads.
 * You do not need to change anything here, but you can add
 * debug statements to help you during development/debugging.
 */
void* classa_student(void *si)
{
  student_info *s_info = (student_info*)si;

  /* enter office */
  classa_enter();

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
  classa_leave();
  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);

  pthread_exit(NULL);
}

/* Main code for class B student threads.
 * You do not need to change anything here, but you can add
 * debug statements to help you during development/debugging.
 */
void* classb_student(void *si)
{
  student_info *s_info = (student_info*)si;

  /* enter office */
  classb_enter();

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
  classb_leave();
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
