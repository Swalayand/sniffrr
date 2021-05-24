#include <stdio.h>
#include <unistd.h>

int main() {
  for( int i=0; i<5; i++ ) printf( "%d ", i );	
  for( int i=0; i<5; i++ ) printf( "%d\n", i );	
  for( int i=0; i<5; i++ ) { printf( "%d\r", i );	fflush(stdout ); sleep(1); }
  return 0;
}
