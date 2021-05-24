// Simple (non-threaded) quicksort implementation
// compile with e.g. gcc -O2 -Wall quicksort-simple.c -o quicksort-simple -DN=10000000

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#define QUEUE_SIZE 1000
#define N 1000000
#define THREADS 24
#define PARTITION_LIMIT 10000
#define CUTOFF 10

struct Job {
    double *index;
    int size;
};

struct Job global_buffer[QUEUE_SIZE];
int global_qin = 0;	// insertion index
int global_qout = 0;	// extraction index

int global_availmsg = 0;	// empty
int shutdown = 0;

pthread_cond_t exists = PTHREAD_COND_INITIALIZER; //buffer empty condition


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

int done_counter = 0; // sorted part of the array 


// Function to get the time elapsed
void get_walltime(double *wct) {
	struct timeval tp;
	gettimeofday(&tp, NULL);
	*wct = (double)(tp.tv_sec+tp.tv_usec/1000000.0);
}


void inssort(double *a,int n) {
int i,j;
double t;
  
  for (i=1;i<n;i++) {
    j = i;
    while ((j>0) && (a[j-1]>a[j])) {
      t = a[j-1];  a[j-1] = a[j];  a[j] = t;
      j--;
    }
  }

}


int partition(double *a,int n) {
int first,last,middle;
double t,p;
int i,j;

  // take first, last and middle positions
  first = 0;
  middle = n/2;
  last = n-1;  
  
  // put median-of-3 in the middle
  if (a[middle]<a[first]) { t = a[middle]; a[middle] = a[first]; a[first] = t; }
  if (a[last]<a[middle]) { t = a[last]; a[last] = a[middle]; a[middle] = t; }
  if (a[middle]<a[first]) { t = a[middle]; a[middle] = a[first]; a[first] = t; }
    
  // partition (first and last are already in correct half)
  p = a[middle]; // pivot
  for (i=1,j=n-2;;i++,j--) {
    while (a[i]<p) i++;
    while (p<a[j]) j--;
    if (i>=j) break;

    t = a[i]; a[i] = a[j]; a[j] = t;      
  }
  
  // return position of pivot
  return i;
}


void quicksort(double *a,int n) {
int i;
  // check if below cutoff limit
  if (n<=CUTOFF) {
    inssort(a,n);
    return;
  }
  
  // partition into two halves
  i = partition(a,n);
   
  // recursively sort halves
  quicksort(a,i);
  quicksort(a+i,n-i);
  
}

void send_msg(struct Job msg) {
  pthread_mutex_lock(&mutex);
  // send message
  global_buffer[global_qin] = msg;
  global_qin += 1;
  if (global_qin>=QUEUE_SIZE) global_qin = 0; // wrap around
  global_availmsg += 1;
  
  // signal the receiver that something was put in buffer
  pthread_cond_signal(&exists);
  
  pthread_mutex_unlock(&mutex);

}

struct Job recv_msg() {
  // lock mutex
  pthread_mutex_lock(&mutex);
  while (global_availmsg<1) {	
    pthread_cond_wait(&exists, &mutex);  
  }
  // receive message
  struct Job i = global_buffer[global_qout];
  global_qout += 1;
  if (global_qout>=QUEUE_SIZE) global_qout = 0; // wrap around
  global_availmsg -= 1;
  
  pthread_mutex_unlock(&mutex);

  return(i);
}

void *work_thread(){
  struct Job my_job = recv_msg();
  if (my_job.size <= PARTITION_LIMIT){
    inssort(my_job.index, my_job.size);
    pthread_mutex_lock(&counter_mutex);
    done_counter += my_job.size;
    if(done_counter >= N){
      shutdown = 1;
      struct Job j;
      for(int i = 0; i < THREADS; i++){
        send_msg(j);
      }
      pthread_mutex_unlock(&counter_mutex);
      pthread_exit(NULL);
    }
    pthread_mutex_unlock(&counter_mutex);
    work_thread();
  }
  int _index = partition(my_job.index, my_job.size);
  struct Job left, right;
  left.index = my_job.index;
  left.size = _index;
  right.index = my_job.index + _index;
  right.size = my_job.size - _index;
  send_msg(left);
  send_msg(right);
  work_thread();
  pthread_exit(NULL);

}



int main() {
double *a;
int i;
double ts, te;
  a = (double *)malloc(N*sizeof(double));
  if (a==NULL) {
    printf("error in malloc\n");
    exit(1);
  }
  pthread_t threads[THREADS];
  // fill array with random numbers
  srand(0);
  for (i=0;i<N;i++) {
    a[i] = (double)rand()/RAND_MAX;
  }
  struct Job start;
  start.index = a;
  start.size = N;
  send_msg(start);
  get_walltime(&ts);
  for (i = 0; i < THREADS; i++){
    pthread_create(&threads[i], NULL, work_thread, NULL);
  }
  // then join threads
  for(i = 0; i < THREADS; i++){
    pthread_join(threads[i],NULL);
  }
  get_walltime(&te);
  // check sorting
  for (i=0;i<(N-1);i++) {
    if (a[i]>a[i+1]) {
      printf("Sort failed!\n");
      break;
    }
  }  
  // destroy mutex - should be unlocked
  pthread_mutex_destroy(&mutex);
  pthread_mutex_destroy(&counter_mutex);
  pthread_cond_destroy(&exists);

  free(a);
  printf("Exec Time(sec) = %f\n", te-ts);	//Print the exec time
  printf("All good\n");
  return 0;
}
