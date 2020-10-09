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
extern int class_counterA; /*Class currently on the office*/
extern int class_counterB;
extern int on_break;
// 0 is class a, 1 is class b, -1 is empty class
sem_t mutex; // Mutex to assure only 3 students enter office
//probably will remove guard or mutex and test if that still works
sem_t guard; // Mutex to guard segments of the critical sections of class a and b

extern int students_in_office;/* Total numbers of students currently in the office */
extern int classa_inoffice;   /* Total numbers of students from class A currently in the office */
extern int classb_inoffice;   /* Total numbers of students from class B in the office */
extern int students_since_break;
extern int classa_total;
extern int classb_total;

typedef struct
{
  int arrival_time;  // time between the arrival of this student and the previous student
  int question_time; // time the student needs to spend with the professor
  int student_id;
} student_info;
// initialize has to be on global scope for all files, hence we use the keyword extern 
extern int initialize(student_info *si, char *filename);
