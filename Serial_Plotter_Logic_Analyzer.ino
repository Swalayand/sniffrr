#include <SPI.h>

byte x[140];
byte y;
void setup (void)
{
  PORTC = B00001111; //enable pullups on A0-A3
  SPI.setDataMode(SPI_MODE3);
  SPI.setClockDivider(SPI_CLOCK_DIV128);
  Serial.begin(115200);
  SPI.begin();
}
void loop (void)
{
  SPDR = 0xAB; //spi transfer

  for (int i = 0; i < 140; i++) {
    x[i] = PINC & 0x0F;
  }
  for (int i = 0; i < 140; i++) {
    plot(i);
  }
  while (1); //wait
}

void plot(int i)
{
  Serial.print(((x[i] >> 0) & 1) + 12);  //A0
  Serial.print(" ");
  Serial.print(((x[i] >> 1) & 1) + 8);   //A1
  Serial.print(" ");
  Serial.print(((x[i] >> 2) & 1) + 4);   //A2
  Serial.print(" ");
  Serial.println(((x[i] >> 3) & 1) + 0); //A3
}
