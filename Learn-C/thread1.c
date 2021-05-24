#include <pthread.h>  // gcc thread1.c -lpthread -o nonthread-sequence.exe 
#include <stdio.h>
#include <unistd.h>  // sleep()

void * jalankan ( void * arg ) {
	for( int i=0; i<50; i++ ) {
		printf( "." );
		fflush( stdout );
		sleep( 0.3 );
    }
}
int main() {
	pthread_t t;
	pthread_create( &t, NULL, jalankan, NULL ); // gcc thread1.c -lpthread -o thread.exe 
	//jalankan( NULL );
	
	for( int i=0; i<50; i++ ) {
		printf( "o" );
		sleep(0.3);
		fflush( stdout );
	}
	return 0;
}
