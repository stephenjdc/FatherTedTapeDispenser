#include <stdio.h>
#include <wiringPi.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#define PIN_A 8
#define PIN_B 9

#define EVENT_END_WAIT_TIME 0.5
#define SLEEP_TIME 500000

int secondsMultiplier = 1000000;

int totalMovement = 0;

int moving = 0;
// clock_t lastMovementTime = 0;

double startTime = 0;
double endTime = 0;

double getTime() {
    // long int start_time;
	struct timespec gettime_now;

	clock_gettime(CLOCK_REALTIME, &gettime_now);
	// start_time = gettime_now.tv_sec;		//Get nS value

    double seconds = gettime_now.tv_sec;
    double billion = 1000000000;

    return seconds + (gettime_now.tv_nsec / billion);
}

double timeSinceEnd() {
    return getTime() - endTime;
}

void setStartTime() {
    startTime = getTime();
}

void update() {
    moving = 1;
    totalMovement++;
    endTime = getTime();

    // double timeSince = 

    // printf("falling edge detected\n");
    // printf("Total Movement: %f\n", (double) totalMovement);
    // printf("Current Time: %f\n", timeSince);
}

void runEvent() {
    printf("This is when it would say the thing\n");
}

void triggerEvent() {
    if (moving == 1) {
        if (timeSinceEnd() >= EVENT_END_WAIT_TIME) {
            moving = 0;
            runEvent();
        } else {
            printf("not long enough since last trigger\n");
        }
    } else {
        printf("wheel hasn't moved since last trigger\n");
    }
}

int main() {
    wiringPiSetup();
    pinMode(PIN_A, INPUT);

    startTime = getTime();

    wiringPiISR (PIN_A, INT_EDGE_FALLING, update) ;

    // check for event every x seconds
    for (;;) {
        // printf("Checking for changes...\n");
        triggerEvent();
        // double timeSinceMovement = (double) (clock() - lastMovementTime)/CLOCKS_PER_SEC;
        // printf("Time Since Movement: %f seconds\n", timeSinceMovement);

        usleep(SLEEP_TIME);
        // sleep(99999999);
    }

    return 0;
}
