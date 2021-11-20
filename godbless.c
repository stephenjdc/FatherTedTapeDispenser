#include <stdio.h>
#include <wiringPi.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#define PIN_A 8
#define PIN_B 9

#define EVENT_END_WAIT_TIME 0.5
#define SLEEP_TIME 500000

// double lastInterruptTime = 0;

int secondsMultiplier = 1000000;

// the total movement for this event
int totalMovement = 0;

// track if an event is in progress
int moving = 0;

// track whether to use an alt output device
int altOutputDevice = 0;
char volume[300] = "-2000";

double startTime = 0;
double endTime = 0;

int getAmountUsed(int ticks) {
    int ticksPerRotation = 22;
    double rollRadiusInInches = 1.5;
    double circumferenceInInches = M_PI * 2 * rollRadiusInInches;

    double numberOfTurns = (double) ticks / (double) ticksPerRotation; 

    return circumferenceInInches * numberOfTurns;
}

double getTime() {
    // long int start_time;
	struct timespec gettime_now;

	clock_gettime(CLOCK_REALTIME, &gettime_now);

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
    // double interruptTime = getTime();

    // if ((interruptTime - lastInterruptTime) < 0.0005) {
    //     printf("debounced!\n");
    // } else {
    moving = 1;
    totalMovement++;
    endTime = getTime();
    // lastInterruptTime = getTime();
    // }
}

void runEvent() {
    double totalDistance = getAmountUsed(totalMovement);
    int distanceInt = (int) totalDistance;

    printf("%.0f inches used \n", (double) totalDistance);
    printf("This is when it would say the thing\n");

    char command[300];
    char rootCmd[] = "mp3wrap output.mp3 audio/yhy.mp3";

    char endCmd[200];
    strcpy(endCmd, " audio/gby.mp3 && omxplayer ");
    if (altOutputDevice == 1) {
        strcat(endCmd, "-o alsa:hw:1,0");
    }
    strcat(endCmd, " output_MP3WRAP.mp3 --vol ");
    strcat(endCmd, volume);
    strcat(endCmd, " && rm output_MP3WRAP.mp3");
    // char endCmd[] = " audio/gby.mp3 && omxplayer -o alsa:hw:1,0 output_MP3WRAP.mp3 --vol -2000 && rm output_MP3WRAP.mp3";

    strcpy(command, rootCmd);

    if (distanceInt == 1) {
        strcat(command, " audio/oneinch.mp3");
        strcat(command, endCmd);
    } else {
        char snum[10];
        sprintf(snum, "%d", distanceInt);

        strcat(command, " audio/_");
        strcat(command, snum);
        strcat(command, ".mp3");
        strcat(command, " audio/inches.mp3");
        strcat(command, endCmd);
    }
    printf(command);
    printf("\n");
    system(command);
}

void startup() {
    char startCmd[200];
    strcpy(startCmd, "omxplayer ");

    if (altOutputDevice == 1) {
        strcat(startCmd, "-o alsa:hw:1,0");
    }

    strcat(startCmd, " audio/inches.mp3 --vol ");
    strcat(startCmd, volume);
    system(startCmd);
}

void resetState() {
    moving = 0;
    totalMovement = 0;
}

void triggerEvent() {
    if (moving == 1) {
        if (timeSinceEnd() >= EVENT_END_WAIT_TIME) {
            // reset state and trigger event
            runEvent();
            resetState();
        } else {
            // printf("not long enough since last trigger\n");
        }
    } else {
        // printf("wheel hasn't moved since last trigger\n");
    }
}

int pinATriggered = 0;
int pinBTriggered = 1;

void updateTest() {
    printf("Tick\n");
}

void pinA() {
    if (pinATriggered == 0 && pinBTriggered == 1) {
        update();

        pinATriggered = 1;
        pinBTriggered = 0;
    }
}

void pinB() {
    if (pinATriggered == 1 && pinBTriggered == 0) {
        pinATriggered = 0;
        pinBTriggered = 1;
    }

}

int main(int argc, char* argv[]) {
    // printf("Starting GodBless...");
    // printf("%i arguments\n", argc);

    if (argc > 1) {
        if (strcmp(argv[1], "--usb") == 0) {
            altOutputDevice = 1;
            printf("Using USB audio...\n");
        } else {
            printf("Using built-in audio\n");
        }
    } else {
        printf("Using built-in audio\n");
    }

    if (argc > 2) {
        strcpy(volume, argv[2]);
        printf("Setting volume to %s\n", volume);
    }

    startup();

    wiringPiSetup();
    pinMode(PIN_A, INPUT);

    startTime = getTime();
    // lastInterruptTime = getTime();

    // wiringPiISR (PIN_A, INT_EDGE_FALLING, update) ;
    wiringPiISR (PIN_A, INT_EDGE_FALLING, pinA) ;
    wiringPiISR (PIN_B, INT_EDGE_FALLING, pinB) ;

    // check for event every x seconds
    for (;;) {
        triggerEvent();
        usleep(SLEEP_TIME);
    }

    return 0;
}
