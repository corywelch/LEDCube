// LED Cube
// By: Cory Welch

#include <SPI.h>

// Pin Variabled
//Latch
const int latchPin = 8;
//Clock
const int clockPin = 13;
//DataIn
const int dataPin = 11;
//Clear
const int dontClearPin = 9;
//Blank
const int blankPin = 10;

//Global Variables
const unsigned int _brightnessmax = 15;

unsigned int _currentIteration = 0; //1-4*_brightnessmax

int cube [16][4] = {0};
// All columns 0 to 15 are columns
// All planes 0 to 3 are planes

// The follow chart is used to map the physical columns on the cube to the number of the shift register
// The shift register must be shifted in in the following order
// [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,P1,P2,P3,P4,X,X,X,X]
//
// Array index for physical Columns note index (i) = x + 4*y 
// (0,0) = 0 which is shift register 8
// (1,3) = 13 which is shift register 4
// (3,1) = 7 which is shift register 15
// [1,5,9,13,12,8,4,0,2,6,10,14,15,11,7,3]

// Shifter Register Output map to Physical Columns
// Note physical plane connectors come down near (3,3) element
// (0,3)   8   1   9  16  (3,3)
//         7   2  10  15
//         6   3  11  14
// (0,0)   5   4  12  13  (3,0)

byte shiftReadyCube [3][60] = {0};
// The data that will be shifted into the cube
// is created via the drawCube Function
// 0 is reg1 -> 1 is reg2 -> 2 is reg3

// These Functions translate the order of the array to the proper order for the shift registers
byte trans1(unsigned int z);//Plane, shifted last
unsigned int trans2(unsigned int data);//Column, 

// This Function is called by the interupt, it determines the byte/int needed for for the array version of the cube
// then calls the translate function and finishes with latching in the data
void drawCube();

// This Function is called during the drawCube function to determine the index of the int to look for for the brightness value
unsigned int brightnessIndex(unsigned int tempBrightness);

// Functions for putting data into the cube array. ie Drawing shaped on the cube
// x is x coord 0-3
// y is y coord 0-3
// z is z coord/plane 0-3
// b is brightness 0-15
// w is width (depends on orgin) 0-3
void CLEARCUBE();
void LED(unsigned int x, unsigned int y, unsigned int z, unsigned int b);
void PLANEZ(unsigned int z, unsigned int b);
void PLANEX(unsigned int x, unsigned int b);
void PLANEY(unsigned int y, unsigned int b);
void COL(unsigned int x, unsigned int y, unsigned int b);
void ROWX(unsigned int x, unsigned int z, unsigned int b);
void ROWY(unsigned int y, unsigned int z, unsigned int b);
void CUBE(unsigned int b);
void BOX(unsigned int x1, unsigned int y1, unsigned int z1, unsigned int x2, unsigned int y2, unsigned int z2, unsigned int b);
void HOLLOWBOX(unsigned int x1, unsigned int y1, unsigned int z1, unsigned int x2, unsigned int y2, unsigned int z2, unsigned int b);

// Animations
void testing();
void MeganAnimation();
void hollowCubeDance();

//Setup Function to init the cube and shift registers
void setup() {
  
  cli();
    //We use Timer 1 to refresh the cube
    TCNT1 = 0;
    TCCR1A = B00000000;//Timer 1 Register A 
    TCCR1B = B00001011;//Timer 1 Register B
    //            ^ Enables CTC (Clear timer on Compare Match)
    //             ^^^ Are Prescalar Bits
    TIMSK1 = B00000010;
    //              ^ Set to call interupt on OCR1A match
    OCR1A = 30; // Counter Max Value
    //This time for interupt value is good to keep note of testing and actual visuals
    //Prescaler Examples
    // 001 = 1
    // 010 = 8
    // 011 = 64
    // 100 = 256
    // 101 = 1024
    
    //Following Equation
    
    //   (Counter Max ie OCR1A Value)         (Clock Frequency Timer 1 = 16MHz = 16,000,000)
    // --------------------------------  =  ---------------------------------------------------
    //      (Resulting Time seconds)                 (Prescalar = TCCR1B last 3 bits)
    
    //     (Counter Max ie OCR1A Value) * (Prescalar = TCCR1B last 3 bits)     
    // -------------------------------- -------------------------------------  =   (Resulting Time seconds) per interupt
    //             (Clock Frequency Timer 1 = 16MHz = 16,000,000)                     
    
    // Currently
    //   29 * 256
    // -----------  =  992 micro seconds per interupt * 60 = 59.52 milli seconds per cube draw 
    //   16000000
  
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(dontClearPin, OUTPUT);
  pinMode(blankPin, OUTPUT);
  
  digitalWrite(blankPin, HIGH);
  digitalWrite(dontClearPin,HIGH);
  
  for(int i=0; i<24; i++){
  digitalWrite(clockPin, LOW);
  digitalWrite(dataPin,LOW);
  digitalWrite(clockPin, HIGH);
  }
  digitalWrite(latchPin, LOW);
  digitalWrite(latchPin, HIGH);
  
  digitalWrite(blankPin, LOW);
  
  for(int i=0;i<3;i++){
    for(int j=0;j<60;j++){
      shiftReadyCube[i][j] = B00000000;
    }
  }

  SPI.begin();

  sei();
}

void loop() {
  
  //testing();
  //MeganAnimation();
  hollowCubeDance();

}

// Interupt for Timer 1
ISR(TIMER1_COMPA_vect){
  
  SPI.transfer(shiftReadyCube[2][_currentIteration]);
  SPI.transfer(shiftReadyCube[1][_currentIteration]);
  SPI.transfer(shiftReadyCube[0][_currentIteration]);
  
  digitalWrite(latchPin,HIGH);
  digitalWrite(latchPin,LOW); 
  
  if(_currentIteration == 59){
    _currentIteration = 0;
  } else {
    _currentIteration++;
  }
  
}

byte trans1(unsigned int z){
  // Shift Register 1
  // (Plane 1-4)
  if(z == 0){
    return B10000000;
  } else if(z == 1){
    return B01000000;
  } else if(z == 2){
    return B00100000;
  } else if(z == 3){
    return B00010000;
  } else {
    return B00000000;
  }
}

//see notes above for explaination
unsigned int trans2(unsigned int data){
  unsigned int output = 0;
  // Shift Register 3
  // (Col 1-8)
  bitWrite(output,16-1,bitRead(data,15-13));
  bitWrite(output,16-2,bitRead(data,15-9));
  bitWrite(output,16-3,bitRead(data,15-5));
  bitWrite(output,16-4,bitRead(data,15-1));
  bitWrite(output,16-5,bitRead(data,15-0));
  bitWrite(output,16-6,bitRead(data,15-4));
  bitWrite(output,16-7,bitRead(data,15-8));
  bitWrite(output,16-8,bitRead(data,15-12));
  
  // Shift Register 2
  // (Col 9-16)
  bitWrite(output,16-9,bitRead(data,15-14));
  bitWrite(output,16-10,bitRead(data,15-10));
  bitWrite(output,16-11,bitRead(data,15-6));
  bitWrite(output,16-12,bitRead(data,15-2));
  bitWrite(output,16-13,bitRead(data,15-3));
  bitWrite(output,16-14,bitRead(data,15-7));
  bitWrite(output,16-15,bitRead(data,15-11));
  bitWrite(output,16-16,bitRead(data,15-15));
  
  return output;
}

unsigned int brightnessIndex(unsigned int tempBrightness){
  //think of like this
  //in cube we have    0   5
  //                  10  15
  //Which is equal to 
  //            B00000000  B00000101
  //            B00001010  B00001111
  //This index will determine the  ^ (index) for this value in the cube variable
  
  if(tempBrightness < 2 && tempBrightness != 0){
    return 0;
  } else if (tempBrightness < 4){
    return 1;
  } else if (tempBrightness < 8){
    return 2;
  } else if (tempBrightness < 16){
    return 3;
  } else {
    return 5; //Should always cause 0 in next section
  }
}

void drawCube(){
  unsigned int tempBrightness = 1;
  unsigned int plane = 0;
  for(unsigned int l=0; l< 60;l++){
    byte reg1 = trans1(plane); //could be called plane
  
    unsigned int cubeData = 0;
    unsigned int brightIndex = brightnessIndex(tempBrightness);
  
    for(unsigned int i=16; i>=1; i--){
     bitWrite(cubeData, i-1, bitRead(cube[16-i][plane],brightIndex)); 
    }
    unsigned int columns = trans2(cubeData);
    byte reg3 = B00000000; //could be called Col 1-8
    byte reg2 = B00000000; //could be called Col 9-16

    for(unsigned int i=8; i>=1; i--){
      bitWrite(reg3, i-1, bitRead(columns,i-1));
      bitWrite(reg2, i-1, bitRead(columns,i+7));
    }
  
    shiftReadyCube[0][l] = reg1;
    shiftReadyCube[1][l] = reg2;
    shiftReadyCube[2][l] = reg3;
  
    if(plane == 3){
      plane = 0;
    
      if(tempBrightness == 15){
        tempBrightness = 1;
      } else {
        tempBrightness++;
      }
      
    } else {
      plane++;
    }
  }
}
void CLEARCUBE(){
  for(int i=0; i<16; i++){
    for(int k=0; k<4; k++){
      cube[i][k] = 0;
    }
  }
}
void LED(unsigned int x, unsigned int y, unsigned int z, unsigned int b){
  if(b > _brightnessmax){
    b = _brightnessmax;
  }
  unsigned int i = x + (4*y);
  cube[i][z] = b;
}

void PLANEZ(unsigned int z, unsigned int b){
  if(b > _brightnessmax){
    b = _brightnessmax;
  }
  for(unsigned int i=0; i<16; i++){
    cube[i][z] = b;
  }
}
void PLANEX(unsigned int x, unsigned int b){
  if(b > _brightnessmax){
    b = _brightnessmax;
  }
  for(unsigned int k=0; k<4; k++){
    for(unsigned int j=0; j<4; j++){
      cube[x + 4*j][k] = b;
    }
  }
}
void PLANEY(unsigned int y, unsigned int b){
  if(b > _brightnessmax){
    b = _brightnessmax;
  }
  for(unsigned int k=0; k<4; k++){
    for(unsigned int i=0; i<4; i++){
      cube[i + 4*y][k] = b;
    }
  }
}

void COL(unsigned int x, unsigned int y, unsigned int b){
  if(b > _brightnessmax){
    b = _brightnessmax;
  }
  for(unsigned int k=0; k<4; k++){
    cube[x + (4*y)][k] = b;
  }
}
void ROWY(unsigned int x, unsigned int z, unsigned int b){
  if(b > _brightnessmax){
    b = _brightnessmax;
  }
  for(unsigned int i = x; i<16; i+=4){
    cube[i][z] = b;
  }
}
void ROWX(unsigned int y, unsigned int z, unsigned int b){
  if(b > _brightnessmax){
    b = _brightnessmax;
  }
  for(unsigned int j = y*4; j<(y*4)+4; j++){
    cube[j][z] = b;
  }
}

void CUBE(unsigned int b){
  if(b > _brightnessmax){
    b = _brightnessmax;
  }
  for(int i=0; i<16; i++){
    for(int k=0; k<4; k++){
      cube[i][k] = b;
    }
  }
}
void BOX(unsigned int x1, unsigned int y1, unsigned int z1, unsigned int x2, unsigned int y2, unsigned int z2, unsigned int b){
  if(b > _brightnessmax){
    b = _brightnessmax;
  }
  if(x2 > 3){
    x2 = 3;
  }
  if(y2 > 3){
    y2 = 3;
  }
  if(z2 > 3){
    z2 = 3;
  }
  if(x1 > 3){
    x1 = 0;
  }
  if(y1 > 3){
    y1 = 0;
  }
  if(z1 > 3){
    z1 = 0;
  }
  if(x1 > x2){
    unsigned int t = x1;
    x1 = x2;
    x2 = t;
  }
  if(y1 > y2){
    unsigned int t = y1;
    y1 = y2;
    y2 = t;
  }
  if(z1 > z2){
    unsigned int t = z1;
    z1 = z2;
    z2 = t;
  }
  for(unsigned int k = z1; k <= z2; k++){
    for(unsigned int j = y1; j <= y2; j++){
      for(unsigned int i = x1; i <= x2; i++){
        cube[i + 4*j][k] = b;
      }
    }
  }
}
void HOLLOWBOX(unsigned int x1, unsigned int y1, unsigned int z1, unsigned int x2, unsigned int y2, unsigned int z2, unsigned int b){
  if(b > _brightnessmax){
    b = _brightnessmax;
  }
  if(x2 > 3){
    x2 = 3;
  }
  if(y2 > 3){
    y2 = 3;
  }
  if(z2 > 3){
    z2 = 3;
  }
  if(x1 > 3){
    x1 = 0;
  }
  if(y1 > 3){
    y1 = 0;
  }
  if(z1 > 3){
    z1 = 0;
  }
  if(x1 > x2){
    unsigned int t = x1;
    x1 = x2;
    x2 = t;
  }
  if(y1 > y2){
    unsigned int t = y1;
    y1 = y2;
    y2 = t;
  }
  if(z1 > z2){
    unsigned int t = z1;
    z1 = z2;
    z2 = t;
  }
  for(unsigned int k = z1; k <= z2; k++){
    for(unsigned int j = y1; j <= y2; j++){
      for(unsigned int i = x1; i <= x2; i++){
        if((i == x1 || i == x2) && (j == y1 || j == y2)){
          cube[i + 4*j][k] = b;
        } else if((i == x1 || i == x2) && (k == z1 || k == z2)){
          cube[i + 4*j][k] = b;
        } else if((j == y1 || j ==y2) && (k ==z1 || k==z2)){
          cube[i + 4*j][k] = b;
        }
      }
    }
  }
}

void testing(){
  CLEARCUBE();
  drawCube();
  for(int z =0; z< 4; z++){
    for(int y =0; y <4; y++){
      for(int x=0; x < 4; x++){
        LED(x,y,z,15);
        delay(50);
        drawCube();
      }
    }
  }
  drawCube();
  delay(1000);
  
  for(int i=15; i>=0; i--){
    CUBE(i);
    delay(100);
    drawCube();
  }
  
  CLEARCUBE();
  drawCube();
  delay(1000);
  
  for(int p =0; p < 4; p++){
    PLANEZ(p,8);
    drawCube();
    delay(250);
  }
  
  for(int y=0; y< 4; y++){
    for(int z=0; z<4;z++){
      ROWX(y,z,0);
      drawCube();
      delay(400);
    }
  }
}

void MeganAnimation(){
  CLEARCUBE();
  BOX(1,1,1,2,2,2,15);
  LED(0,0,0,15);
  LED(0,0,3,15);
  LED(0,3,0,15);
  LED(0,3,3,15);
  LED(3,0,0,15);
  LED(3,0,3,15);
  LED(3,3,0,15);
  LED(3,3,3,15);
  
  drawCube();
  delay(200);
  
  CLEARCUBE();
  
  PLANEZ(0,15);
  PLANEZ(3,15);
  drawCube();
  
  delay(50);
  
  CLEARCUBE();
  LED(0,3,0,15);
  LED(0,3,3,15);
  LED(3,3,0,15);
  LED(3,3,3,15);
  LED(1,3,1,15);
  LED(1,3,2,15);
  LED(2,3,1,15);
  LED(2,3,2,15);
  
  drawCube();
  
  delay(50);
}

void hollowCubeDance(){
  int time = 50;
  
  //Start 0,0,0
  CLEARCUBE();
  LED(0,0,0,15);
  drawCube();
  delay(5*time);

  CLEARCUBE();
  HOLLOWBOX(0,0,0,1,1,1,15);
  drawCube();
  delay(time);
  
  CLEARCUBE();
  HOLLOWBOX(0,0,0,2,2,2,15);
  drawCube();
  delay(time);
  
  //FULL
  CLEARCUBE();
  HOLLOWBOX(0,0,0,3,3,3,15);
  drawCube();
  delay(15*time);
  
  //To 0,3,3
  CLEARCUBE();
  HOLLOWBOX(0,1,1,2,3,3,15);
  drawCube();
  delay(time);
  
  CLEARCUBE();
  HOLLOWBOX(0,2,2,1,3,3,15);
  drawCube();
  delay(time);
  
  CLEARCUBE();
  LED(0,3,3,15);
  drawCube();
  delay(5*time);
  
  CLEARCUBE();
  HOLLOWBOX(0,2,2,1,3,3,15);
  drawCube();
  delay(time);
  
  CLEARCUBE();
  HOLLOWBOX(0,1,1,2,3,3,15);
  drawCube();
  delay(time);
  
  //FULL
  CLEARCUBE();
  HOLLOWBOX(0,0,0,3,3,3,15);
  drawCube();
  delay(15*time);
  
  //To 3,0,3
  CLEARCUBE();
  HOLLOWBOX(1,0,1,3,2,3,15);
  drawCube();
  delay(time);
  
  CLEARCUBE();
  HOLLOWBOX(2,0,2,3,1,3,15);
  drawCube();
  delay(time);
  
  CLEARCUBE();
  LED(3,0,3,15);
  drawCube();
  delay(5*time);
  
  CLEARCUBE();
  HOLLOWBOX(2,0,2,3,1,3,15);
  drawCube();
  delay(time);
  
  CLEARCUBE();
  HOLLOWBOX(1,0,1,3,2,3,15);
  drawCube();
  delay(time);
  
  //FULL
  CLEARCUBE();
  HOLLOWBOX(0,0,0,3,3,3,15);
  drawCube();
  delay(15*time);
  
  //To 0,3,0
  CLEARCUBE();
  HOLLOWBOX(0,1,0,2,3,2,15);
  drawCube();
  delay(time);
  
  CLEARCUBE();
  HOLLOWBOX(0,2,0,1,3,1,15);
  drawCube();
  delay(time);
  
  CLEARCUBE();
  LED(0,3,0,15);
  drawCube();
  delay(5*time);
  
  CLEARCUBE();
  HOLLOWBOX(0,2,0,1,3,1,15);
  drawCube();
  delay(time);
  
  CLEARCUBE();
  HOLLOWBOX(0,1,0,2,3,2,15);
  drawCube();
  delay(time);
  
  //FULL
  CLEARCUBE();
  HOLLOWBOX(0,0,0,3,3,3,15);
  drawCube();
  delay(15*time);
  
  //To 0,0,0
  CLEARCUBE();
  HOLLOWBOX(0,0,0,2,2,2,15);
  drawCube();
  delay(time);
  
  CLEARCUBE();
  HOLLOWBOX(0,0,0,1,1,1,15);
  drawCube();
  delay(time);
  
}

