#include <stdio.h>
#include <wiringPi.h>
#include <time.h>
#include <stdlib.h>


#define PIN_A 8
#define PIN_B 9

int loopTracker = 0;

int secondsMultiplier = 1000000;

// int AInput = 15;
// int BInput = 16;
 
unsigned char lastState = 0;
unsigned char steps = 0;

// track whether the steps are moving or not
unsigned char lastStep = 0;

// set to 1 on the *first* step after stationary
int stepsJustStarted = 0;
int stepsJustStopped = 0;

// set to 1 on the while wheel is stopped/moving
int stepsStopped = 0;
int stepsStarted = 0;

unsigned char totalMovement = 0;

//
clock_t lastStepTime = 0;

int  cw = 0;
unsigned char AState = 0;
unsigned char BState = 0;
unsigned char State = 0;

clock_t timeA = 0;
clock_t timeB = 0;
  
void loop() {
  // read the input pin:
  AState = digitalRead(PIN_A);
  BState = digitalRead(PIN_B) << 1;
  State = AState | BState;

  // unsigned char lastSteps = steps;
 
  if (lastState != State){
    switch (State) {
      case 0:
        if (lastState == 2){
          steps++;
          cw = 1;
        }
        else if(lastState == 1){
          steps--;
          cw = -1;
        }
        break;
      case 1:
        if (lastState == 0){
          steps++;
          cw = 1;
        }
        else if(lastState == 3){
          steps--;
          cw = -1;
        }
        break;
      case 2:
        if (lastState == 3){
          steps++;
          cw = 1;
        }
        else if(lastState == 0){
          steps--;
          cw = -1;
        }
        break;
      case 3:
        if (lastState == 1){
          steps++;
          cw = 1;
        }
        else if(lastState == 2){
          steps--;
          cw = -1;
        }
        break;
    }
  }
 
  // if the step is the same as before, the wheel isn't moving
  if (steps != lastStep) {
    totalMovement += abs(steps - lastStep);
  }

  // update state
  lastState = State;
  lastStep = steps;

  if (loopTracker > 1000) {
    printf("%u", totalMovement);
    printf("\n");
    loopTracker = 0;
  }
  
  // delayMicroseconds(500);
  delay(1);
  loopTracker++;
}

void printStatus() {
  printf("%u", State);
  printf("\t");
  printf("%u", cw);
  printf("\t");
  printf("%u", steps);
  printf("\n");
}

void timer() {
  timeB = clock();
  clock_t timeDiff = timeB - timeA;

  if (timeDiff > 1 * secondsMultiplier) {
    printf("Elapsed: %f seconds\n", (double) timeDiff / CLOCKS_PER_SEC);
    timeA = clock();
  }
}


int main(void) {

  // timeA = clock();
  // for (;;) {
  //     timer();
  // }

  // return 0;

  wiringPiSetup();
  pinMode(PIN_A, INPUT);
  pinMode(PIN_B, INPUT);

  for (;;) {
      loop();
  }

  return 0;
}
