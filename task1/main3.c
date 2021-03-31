#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>

// this is the plain version of dot product of two NxN matrices
// compile with:
// gcc -Wall -O2 matmul-simple.c -o matmul-simple -DN=1000

// matrix dims N rows x N columns: use -DN=.. to define on compilation

#define BLOCK 8


void get_walltime(double *wct) {
  struct timeval tp;
  gettimeofday(&tp,NULL);
  *wct = (double)(tp.tv_sec+tp.tv_usec/1000000.0);
}


int main() {
double ts,te;
unsigned int i,j,ii,jj,istart,iend,jstart,jend;
double *a,*b,*c;	// matrices A,B,C C=AxB, B is transposed

  a = (double *)malloc(N*N*sizeof(double));
  if (a==NULL) {
    exit(1);
  }

  b = (double *)malloc(N*N*sizeof(double));
  if (b==NULL) {
    free(a); exit(1);
  }

  c = (double *)malloc(N*N*sizeof(double));
  if (c==NULL) {
    free(a); free(b); exit(1);
  }

  // init input and output matrices
  for (int i=0;i<N;i++) {
    for (int j=0;j<N;j++) {
      a[i*N+j] = i;
      b[i*N+j] = i;
      c[i*N+j] = 0.0;
    }
  }

  // get starting time (double, seconds)
  get_walltime(&ts);
  // load, matrix multiplication
  for (i=0; i<N; i+=BLOCK) {	// for all rows of A,C
    istart = i; iend = i + BLOCK;
    for (j=0;j<N;j+=BLOCK) {	// for all "columns" (rows) of B
      jstart = j; jend = j + BLOCK;
      for(ii = istart; ii < iend; ii++){

        for (jj = jstart; jj < jend; jj++){
          double sum = 0.0;
          for (int k=0;k<N;k+=4) {	// for each element of selected A row and B "column"
            sum += a[ii*N+k]*b[jj*N+k];	// a[i,k]*b[j,k]  note: B is transposed, originally b[k,j]
            sum += a[ii*N+k+1]*b[jj*N+k+1];
            sum += a[ii*N+k+2]*b[jj*N+k+2];
            sum += a[ii*N+k+3]*b[jj*N+k+3];
          }
          c[ii*N+jj] = sum;	// c[i,j]
        }
      }
    }
  }

  // get ending time
  get_walltime(&te);

  // print computation time
  printf("%f\n",(te-ts));

  // test result
  int done = 0;
  for (int i=0;i<N&&done==0;i++) {
    for (int j=0;j<N;j++) {
      if (c[i*N+j]!=(double)i*j*N) { printf("Error! %d,%d (%f)\n",i,j,c[i*N+j]); done=1; break; }
    }
  }

  free(c);
  free(b);
  free(a);

  return 0;
}
