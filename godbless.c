/*******************************************************
*
* Copyright (C) 2022 Stephen Coyle <stephenjdc@gmail.com>
*
* This file is part of Godbless.
*
* Godbless can not be copied and/or distributed without the express
* permission of Stephen Coyle
*
*******************************************************/

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

int playing = 0;

int pinATriggered = 0;
int pinBTriggered = 1;

int getAmountUsed(int ticks) {
    int ticksPerRotation = 22;
    double rollRadiusInInches = 1.854;
    double circumferenceInInches = M_PI * 2 * rollRadiusInInches;
    double numberOfTurns = (double) ticks / (double) ticksPerRotation; 
    return circumferenceInInches * numberOfTurns;
}

double getTime() {
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
    moving = 1;
    totalMovement++;
    endTime = getTime();
}

void runEvent() {
    double totalDistance = getAmountUsed(totalMovement);
    int distanceInt = (int) totalDistance;

    printf("%.0f inches used \n", (double) totalDistance);
    printf("This is when it would say the thing\n");

    char command[300];
    char rootCmd[] = "mp3wrap output.mp3 audio/yhy.mp3";

    char endCmd[200];
    strcpy(endCmd, " audio/ost.mp3 audio/gby.mp3 && omxplayer");
    if (altOutputDevice == 1) {
        strcat(endCmd, "-o alsa:hw:1,0");
    }
    strcat(endCmd, " output_MP3WRAP.mp3 --vol ");
    strcat(endCmd, volume);
    strcat(endCmd, " && rm output_MP3WRAP.mp3");

    strcpy(command, rootCmd);

    int feet = distanceInt / 12;
    int inches = distanceInt % 12;

    // get foot as string
    char feetString[10];
    sprintf(feetString, "%d", feet);

    char inchString[10];
    sprintf(inchString, "%d", inches);

    if (feet == 0 && inches == 0) {
        strcat(command, " audio/_1.mp3");
        strcat(command, " audio/_inch.mp3");
    } else {
        // add the feet if needed
        if (feet == 0) {
            // do nothing
        } else if (feet == 1) {
            // add one foot
            strcat(command, " audio/_1.mp3");
            strcat(command, " audio/_foot.mp3");
        } else {
            // add multiple feet
            strcat(command, " audio/_");
            strcat(command, feetString);
            strcat(command, ".mp3");
            strcat(command, " audio/_feet.mp3");
        }

        if (inches == 0) {
            // do nothing
        } else if (inches == 1) {
            // add one inch
            strcat(command, " audio/_1.mp3");
            strcat(command, " audio/_inch.mp3");
        } else {
            // add multiple inches
            strcat(command, " audio/_");
            strcat(command, inchString);
            strcat(command, ".mp3");
            strcat(command, " audio/_inches.mp3");
        }
    }

    strcat(command, endCmd);

    printf(command);
    printf("\n");
    playing = 1;
    system(command);
    playing = 0;
}

void startup() {
    char startCmd[200];
    strcpy(startCmd, "omxplayer ");

    if (altOutputDevice == 1) {
        strcat(startCmd, "-o alsa:hw:1,0");
    }

    strcat(startCmd, " audio/gby.mp3 --vol ");
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

void updateTest() {
    printf("Tick\n");
}

void pinA() {
    // ignore interrupt if sound playing (not sure if this is needed?)
    if (playing == 1) {
        return;
    }

    if (pinATriggered == 0 && pinBTriggered == 1) {
        update();

        pinATriggered = 1;
        pinBTriggered = 0;
    }
}

void pinB() {
    // ignore interrupt if sound playing (not sure if this is needed?)
    if (playing == 1) {
        return;
    }

    if (pinATriggered == 1 && pinBTriggered == 0) {
        pinATriggered = 0;
        pinBTriggered = 1;
    }

}

int main(int argc, char* argv[]) {
    printf("Starting GodBless...\n");
    printf("Removing any existing output files...\n");
    system("rm output_MP3WRAP.mp3");

    if (argc > 1) {

        if (strcmp(argv[1], "debug") == 0) {
            totalMovement = atoi(argv[2]);
            runEvent();
            printf("--debug mode, exiting\n");
            return 0;
        }

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

    wiringPiISR (PIN_A, INT_EDGE_FALLING, pinA) ;
    wiringPiISR (PIN_B, INT_EDGE_FALLING, pinB) ;

    // check for event every x seconds
    for (;;) {
        triggerEvent();
        usleep(SLEEP_TIME);
    }

    return 0;
}
