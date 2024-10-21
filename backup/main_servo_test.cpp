#include "Arduino.h"
#include "ESP32Servo.h"

#define INSTANT

// Digital I/O used
#define SERVO_IN      13  // Servo pwm signal

// SERVO 
#ifdef INSTANT
#define TIMER_RELOAD_MS     250  // Reduced timer interval for smoother movement
#define START_ANGLE_DEGREES 0
#define MAX_ANGLE_DEGREES   45 //120
#else
#define TIMER_RELOAD_MS     10 // Reduced timer interval for smoother movement
#define START_ANGLE_DEGREES 45
#define MAX_ANGLE_DEGREES   120
// #define DURATION_MS         500  // Duration to move from 0 to 90 degrees
#define SERVO_STEP_SIZE     1// abs((MAX_ANGLE_DEGREES - START_ANGLE_DEGREES) / (DURATION_MS / TIMER_RELOAD_MS))
#endif

TimerHandle_t reloadTimer;
void reloadTimerCallback(TimerHandle_t xTimer);
Servo servo;
static int pos = START_ANGLE_DEGREES;

void reloadTimerCallback(TimerHandle_t xTimer)
{
    static bool increasing = true;

    servo.write(pos);

#ifdef INSTANT
    if (increasing) {
        pos = MAX_ANGLE_DEGREES;
        increasing = false;
    } else {
        pos = START_ANGLE_DEGREES;
        increasing = true;
    }
#else
    if (increasing) {
        pos += SERVO_STEP_SIZE;
        if (pos >= MAX_ANGLE_DEGREES) {
            increasing = false;
        }
    } else {
        pos -= SERVO_STEP_SIZE;
        if (pos <= START_ANGLE_DEGREES) {
            increasing = true;
        }
    }
#endif
}


// put your setup code here, to run once
void setup()
{

    reloadTimer = xTimerCreate("ReloadTimer", pdMS_TO_TICKS(TIMER_RELOAD_MS), true, 0, reloadTimerCallback);

    servo.attach(SERVO_IN);
	servo.write(START_ANGLE_DEGREES);
    delay(3000);
	xTimerStart(reloadTimer, 0);

    Serial.begin(115200);
}



// put your main code here, to run repeatedly
void loop()
{
	// speaker.loop();
}


