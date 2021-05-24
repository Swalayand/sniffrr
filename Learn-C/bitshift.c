#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define VAL 12

char* write(int val){ // conclusion: convert decimal to binary pulse
  char* c;
  c = malloc(16);
  for (int i = 0; i < 8; i++)  {
		//printf( "%d" , !!(val & (1 << i)));
    sprintf( c+i, "%d " , !!(val & (1 << i)));
  }
  // printf("\n");
  return c;
}

void read(char* pls){ // conclusion: convert binary pulse to decimal
  uint8_t value = 0b0;

  int pulse[] = {1, 1, 0, 0, 0, 0, 0, 0}; // representaion of pulse on array of int
	
  printf("bit shifting: ");
  for (int i = 0; i < 8; ++i) {
		value = value | ((int)pls[i]-48) << i;
    printf( "%d " , value); 
  }
  printf("\n");
  printf( "read binary pulse: %d \n" , value); 
}

int main(){
  printf("data to transmit: %d\n", VAL);
  char* n = write(VAL);
  printf("binary pulse as string: %s\n", n);
  read(n);
  return 0;
}

// EXERCISE

/*
left shift (multiplication by four)
// 1011 << 2 = 101100
int n = 0b1011;
n = n << 2;
printf("%d", n);
*/

/*
inclusive or
int n = 0b1011;
n |= 0b0100;
*/
 
