#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

typedef int bool;
const bool true = 1;
const bool false = 0;
const int OK = 0; // Return code.

const int END = 0;


//Work, actually.
int * task = 0;
int * result = 0;
bool * started = 0;

pthread_mutex_t work_pool_lock;

int number_of_tasks = 100; 

void error(char * msg){
  printf( "Problem: %s.\n", msg);
  exit(1);
}

int pick_work(){
  int i;
  for( i = 0; task[i]; i++){
    if( !started[i]){
      started[i] = true;
      return i;
    }
  }
  return i;
}

int locked_pick_work(){
  int picked;

  if( pthread_mutex_lock( &work_pool_lock) != OK ){ error( "unable to lock the mutex"); }
  picked = pick_work(); 
  if( pthread_mutex_unlock( &work_pool_lock) != OK ){ error( "unable to unlock the mutex");}

  return picked;
}

int init( int times){
  int i, rc;
  number_of_tasks = times+1;

  task = malloc( number_of_tasks * sizeof(int) );
  result = malloc( number_of_tasks * sizeof(int) );
  started = malloc( number_of_tasks * sizeof(bool) );

  if ( (rc = pthread_mutex_init( &work_pool_lock, NULL))){
    printf("code: %d\n", rc);
    error( "unable to init mutex");
  };

  for( i = 0; i < number_of_tasks; i++){
    task[i] = 2*i+1;
    result[i] = task[i];
    started[i] = false;
  }
  
  task[number_of_tasks-1] = END;
}

int release(){
  int rc;

  free(task);
  free(result);
  free(started);

  if ( (rc = pthread_mutex_destroy( &work_pool_lock))){
    printf("code: %d\n", rc);
    error( "unable to init mutex");
  };

}

int len(){
  return number_of_tasks;/*
  int is_of = 0;
  for( ; task[is_of]; is_of++){
  }
  return is_of;*/
}

int do_work(int n){
  printf( "Doing task %d\n", n);
  int a, b, c;
  int done = 0;
  int done_more = 1;
  /*
  for( a=0; a < 100000; a++){
    for( b=0; b < 20; b++){
      for( c=0; c < 2; c++){
	done += a;
	done = (done%15);
	done_more = (done+1);
      }
    }
  }
  */
  sleep(1);
  return result[n] = task[n]*2; //(done_more - done);
}

bool done(){
  int i;
  for( i = 0; task[i]; i++){
    if( task[i]*2 != result[i] ){
      return false;
    }
  }
  return true;
}

void * pthreads_worker( void * in){
  //  do_work(*((int*)in));
  int todo; 
  for(todo = locked_pick_work(); task[todo]; todo = locked_pick_work()) {
    do_work(todo);
  }
  return NULL;
}

void do_pthreads(int threads_num){
  int i, rc;
  pthread_t * threads = (pthread_t *) malloc( len() * sizeof(pthread_t));
  int * task_ids = (int *)malloc( len() * sizeof(int));
  
  for( i = 0; i < threads_num; i++){
    printf("Starting worker %d.\n", i);
    task_ids[i] = i;
    rc = pthread_create( &(threads[i]), NULL, pthreads_worker, &task_ids[i]);
    if( rc != 0){
      printf( "Code: %d\n", rc);
      if ( 11 == rc ){
	sleep(1); // Try again.
	rc = pthread_create( &(threads[i]), NULL, pthreads_worker, &task_ids[i]);
	if(rc!=0){
	  error("really failed to start the thread");
	}
      }
      error("failed to start task");
    }
  }
  
  for( i = 0; i < len(); i++){
    pthread_join( threads[i], NULL);
  }
  
  free( threads);
  free( task_ids);
}

int do_simply(){
  int i;
  for( i=0; i < len(); i++ ){
    do_work(i);
  }
}

int main(int argc, char ** argv){

  // Options.
  enum {
    SIMPLE,
    PTHREADS
  } mode;

  int times = 20;
  int threads_num = 3;
  int opt;
  char * usage = "s: Simple mode.\np: Pthreads mode.\n";
  
  while( (opt = getopt( argc, argv, "spn:t:")) != -1 ) { 
    switch( opt ){
    case 'n':
      times = atoi(optarg);
      break;
    case 't':
      threads_num = atoi(optarg);
      break;
    case 's':
      printf("Simple.\n");
      mode = SIMPLE;
      break;
    case 'p':
      printf("Pthreads.\n");
      mode = PTHREADS;
      break;
    default:
      printf( "%s\n", usage );
    }
  }

  // Do stuff:
  
  printf("%d tasks.\n", times);
  init( times);

  switch(mode) {
  case SIMPLE:
    do_simply();
    break;
  case PTHREADS:
    do_pthreads( threads_num);
    break;
  default:
    printf("Unknown mode, do nothing: \n %s.\n", usage);
  }

  printf( "Task %s completed\n", done()?"is":"is not");

  release();
}
