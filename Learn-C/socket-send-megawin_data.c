#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>

int ar[18] = {3, 64, 192, 0, 0, 134, 0, 185, 0, 243, 0, 115, 0, 2, 0, 0, 0, 138};
uint8_t full_digits[] = { 223, 134, 189, 183, 230, 119, 127, 199, 255, 247, 219, 130, 185, 179, 226, 115, 123, 131, 251, 243 };

char res[20];

void searchNum(){
  int c = 0;
  for (int i =0; i < 18; i++){
    for (int j = 0; j < 20; j++){
      if ((ar[i] == full_digits[j] && ar[13] == 2) || (ar[i] == full_digits[j] && ar[13] == 3)) {
        
        //printf("%d = %d \n", i, j%10);
        sprintf(res+(c++), "%d", j%10);
      }
    }
  }
}

int main(){
  searchNum();
  printf("%s", res);
  printf("\n");
}
