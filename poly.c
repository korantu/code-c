#include <stdio.h>
#include <string.h>
#include <math.h>

#ifndef abs
#define abs(X) (((X)>0)?(X):(-(X))) 
#endif

typedef int bool;
static int true = 1;
static int false = 0;

/* Prefixes:
t_ - test things
p_ - points
f_ - real values
fn_- functions
i_ - grids (todo)

 */

void t_must( bool cond, char * why){
  if( ! cond){
    printf( "FAILED: %s\n", why);
  } else {
    printf( "%s ... ok.\n", why);
  }
}

// float points
// 6 minutes - untested printing function.

typedef float * point;
float epsilon = 0.0001;

bool f_same( float a, float b){
  return ( abs(a-b) < epsilon); 
}


// Visual feedback.
void pp( point pnt){
  printf("(%.2f %.2f %.2f)\n", pnt[0], pnt[1], pnt[2]);
}

void p( float pnt){
  printf("=%.2f\n", pnt);
}

void say( char * msg){
  printf( "%s\n", msg);
}


bool p_same( point a, point b){
  return f_same(a[0],b[0]) && f_same(a[1],b[1]) && f_same(a[2],b[2]);
}

// Ops:
#define EACH(X) { int i = 0; X; i = 1; X; i = 2; X;  return a;}
point p_add( point a, point b){ EACH( a[i]+=b[i]) }
point p_sub( point a, point b){ EACH( a[i]-=b[i]) }
point p_copy( point a, point b){ EACH( a[i]=b[i]) }
point p_scale( point a, float f){ EACH( a[i]*=f) }
float p_len2( point a){ return a[0]*a[0]+a[1]*a[1]+a[2]*a[2];};
point p_cross( point a, point b){
  float tmp[] = { a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0] };
  return p_copy( a, tmp);
}
float p_dot( point a, point b){ return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]; }
float p_dist2( point a, point b){ float diff[] = {a[0]-b[0], a[1]-b[1], a[2]-b[2]}; return p_len2( diff); };

// Set to specified coordinates.
point p_set( point a, float x, float y, float z){  a[0] = x; a[1] = y; a[2] = z; return a; }

// Point playground.
void do_point(){
  float a[] = {-1.0f,-1.0f,-1.0f};
  float b[] = {1.0f,1.0f/3,1.0f};
  float a1[] = {1.0f,1.0f,1.0f};

  float d[] = {1.0f,0.0f,0.0f};
  float d1[] = {1.0f,0.0f,0.0f};
  float d2[] = {0.0f,1.0f,1.0f};

  float cr[] = { 3, -3, 1};
  float cr1[] = { 4, 9, 2};
  float cr_res[] = { -15, -2, 39};

  pp(a); pp(b);

  t_must( p_same( p_scale( a, -1), a1), "scale");
  p_copy( b, p_copy( a1, a));
  t_must( p_same( a, b), "copy");
  t_must( p_same( a, a1), "copy-2");

  p_scale( b, 2);
  p_sub( b, a);
  t_must( p_same( a, b), "sub");
  t_must( f_same( p_len2( a), 3), "len2");

  t_must( f_same( 1, p_dot( d, d1)), "dot");
  t_must( f_same( 0, p_dot( d, d2)), "dot2");

  t_must( p_same( cr_res, p_cross( cr, cr1)), "cross");

  p_set( a, 2,3,4);
  p_set( b, 2,3,4);
  t_must( p_same( a, b) && f_same(a[0], 2) &&  f_same(a[1], 3) &&  f_same( a[2], 4) , "set");
  p_set( b, 1,2,3);
  t_must( ! p_same( a, b), "set2");

}

// Indexing part.
// Cell
// 0001xyz

typedef unsigned int cell;
typedef unsigned int idx;

#define BITS 10
#define CMASK ((1<<(BITS))-1)
#define MAXIDX CMASK

// Bits packing. (Cannot stand waste.)
#define IX(i) (CMASK & i)
#define IY(i) (CMASK & (i >> BITS))
#define IZ(i) (CMASK & (i >> (BITS << 1)))

#define I(x,y,z) ((z<<(BITS<<1))+(y<<BITS)+(x))

void px( idx i){
  printf("[%d %d %d]\n", IX(i), IY(i), IZ(i));
}

// Reference
float i_zero[] = {-1,-1,-1};
float i_size = 2;

void do_index(){
  px(I(9,34,7));
}

// 20 minutes - incomplete tests for spherical function
// Function playground

typedef float (*field)( point);

//Example fields:

// Unit phere centered at 0
float a_sphere( point where){
  float center[] = { 0.0f, 0.0f, 0.0f};
  float radius2 = 1.0f;
  p_sub( center, where);
  return radius2 - p_len2( center);
}

// Gradient in direction (1,1,1)
float a_gradient( point where){
  return where[0]+where[1]+where[2];
}

// Check if the cell intersects the field.
// checker if the cell intersects the field. 10 mins.
bool f_intersects( field f, point c, float size){
  float reference = (*f)(c);
  float corners[8][3] = {
    {c[0],c[1],c[2]}, {c[0],c[1],c[2]+size}, {c[0],c[1]+size,c[2]}, {c[0],c[1]+size,c[2]+size},
    {c[0]+size,c[1],c[2]}, {c[0]+size,c[1],c[2]+size}, {c[0]+size,c[1]+size,c[2]}, {c[0]+size,c[1]+size,c[2]+size},
  };
  int i;
  
  for( i = 1; i < 8; i++){
    if( reference * (*f)(corners[i]) < 0) return true;
  }

  return false;
}

// Find surface - 20 min.
bool f_find( field f, point a, point b){
  float c[] = {0,0,0};
  float fa = (*f)(a);
  float fb = (*f)(b);
  float fc;
  if (  fa * fb  > 0 ) return false; // Nothing to search
  if ( p_dist2( a, b) < epsilon*epsilon ) return true;
  p_scale( p_add( p_add( c, a), b), 0.5);
  fc = (*f)(c);
  p_copy( (fa * fc > 0)?a:b, c); 
  return f_find( f, a, b);
}

// Function-related excercizes
void do_function(){
  float zero[] = { 0,0,0};
  float center[] = { -1, -1, -1};
  float corners[8][3] = {
    {1,1,1}, {1,1,-1}, {1,-1,1}, {1,-1,-1},
    {-1,1,1}, {-1,1,-1}, {-1,-1,1}, {-1,-1,-1},
  };
  int i;

  pp( zero);
  say("Points");
  t_must( 0 < a_sphere( zero), "center");
  for( i = 0; i < 8; i++){
    pp( corners[i]);
    t_must( 0 > a_sphere( corners[i]), "corner");
  }

  t_must( ! f_intersects( &a_sphere, center, 2), "whole");
  t_must( f_intersects( &a_sphere, center, 1), "part");

  float a[] = {-1,-1,-1};
  float b[] = {1,1,1};
  f_find( &a_gradient, a, b);
  t_must( f_same( (*a_gradient)(a), 0), "find-gradient");

  f_find( &a_sphere, p_set(a, 0,0,0), p_set(b, 200,30,-100));
  t_must( f_same( (*a_sphere)(a), 0), "find-sphere");
  

}

// Stupid samples.

void do_A(){
  printf("I am sort-of A.\n");
}


void do_B(){
  printf("I am maybe B...\n");
}

// Container for the workers.
typedef void deed();

typedef struct {
  char * name;
  deed * the_thing;
} act;

// List of things this code can do.
act todo[]  = { 
  {"A", do_A }, 
  {"B", do_B}, 
  {"point", do_point},
  {"function", do_function},
  {"index", do_index},
  {NULL, NULL} }; // Must be NULL-terminated.

// Print all the things.
void list_actions(){
  act * cur = todo;
  while( cur->name){
    printf( "[%s]\n", cur->name);
    cur++;
  }
}

// Do all things named as name.
void do_action( char * name){
  act * cur = todo;
  while( cur->name){
    if( strstr( cur->name, name)){
      cur->the_thing();
    }
    cur++;
  }
}


// Print input args.
void report_args( int argc, char ** argv) {
  int i;

  for ( i = 0; i < argc; i++){
    if( i > 0){
      printf("|");
    }
    printf("%s", argv[i]);
  }
  printf("\n");
}

// Commandline args tell what functions to run.
// The idea is to build more and more tests untill it makes sense.
int main( int argc, char ** argv){ 
  int i = 1;

  printf("Started...\n");
  report_args( argc, argv);
  if( argc == 1){
    list_actions();
  } else {
    for(; i < argc; i++){
      do_action( argv[i]);
    }
  }
  printf("Done.\n");
}
