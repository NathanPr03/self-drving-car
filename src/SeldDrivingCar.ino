#include <QTRSensors.h>
    
using namespace std;

int motorLeft[] = {A4, A5};
int motorRight[] = {D6, D5};

int onBoardLed = D7;

int rightIndicator = D2;
int leftIndicator = D8;

QTRSensors qtr;

const uint8_t SensorCount = 3;
uint16_t sensorValues[SensorCount];

const int LEFT_SENSOR = 0;
const int MIDDLE_SENSOR = 1;
const int RIGHT_SENSOR = 2;

const int LARGE_DISPARITY_BETWEEN_SENSORS = 30;

void setup() {
    Serial.begin(9600);
    Serial.println("Start of setup real yin");
    qtr.setTypeRC();
    qtr.setSensorPins((const uint8_t[]){A2, A1, A0}, SensorCount);
    
    pinMode(onBoardLed, OUTPUT);
    pinMode(leftIndicator, OUTPUT);
    pinMode(rightIndicator, OUTPUT);

    for(int i = 0; i < 2; i++){
        pinMode(motorLeft[i], OUTPUT);
        pinMode(motorRight[i], OUTPUT);
    }
    
    led_on(onBoardLed);
    // analogRead() takes about 0.1 ms on an AVR.
    // 0.1 ms per sensor * 4 samples per sensor re ad (default) * 6 sensors
    // * 10 reads per calibrate() call = ~24 ms per calibrate() call.
    // Call calibrate() 400 times to make calibration take about 10 seconds.
    for (uint16_t i = 0; i < 400; i++)
    {
        qtr.calibrate();
    }
    led_off(onBoardLed); // turn off Arduino's LED to indicate we are through with calibration

    for (uint8_t i = 0; i < SensorCount; i++)
    {
        Serial.println("Minimum calibrated value");
        Serial.println(String(qtr.calibrationOn.minimum[i]));
    }
    

    // print the calibration maximum values measured when emitters were on
    for (uint8_t i = 0; i < SensorCount; i++)
    {
        Serial.println("Maximum calibrated value");
        Serial.println(String(qtr.calibrationOn.maximum[i]));
    } 
    Serial.println("End of setup");
    delay(5000);

}

enum direction { TURN_LEFT_GRADUAL = 0, STRAIGHT_AHEAD = 1, TURN_RIGHT_GRADUAL = 2, TURN_LEFT_HARD = 3, TURN_RIGHT_HARD = 4 };

void loop() {
    Serial.println("Start of loop");

    useSensors();
    //motor_stop(2);
    //delay(3000);
    //drive_forward(10000);
    
    // drive_backward(10000);
    // motor_stop(25);
    // turn_left(10000);
    // motor_stop(25);
    // turn_right(10000);
    // motor_stop(10000);
}

void drive(int direction)
{
    switch(direction) {
        case(TURN_LEFT_GRADUAL):
            turn_left_gradual(1);
            break;
        case(TURN_RIGHT_GRADUAL):
            turn_right_gradual(1);
            break;
        case(TURN_RIGHT_HARD):
            turn_right_hard(1);
            break;
        case(TURN_LEFT_HARD):
            turn_left_hard(1);
            break;
        case(STRAIGHT_AHEAD):
            drive_forward(1);
            break;
        default:
            Serial.println("-----NO MATCH FOUND-----");
            break;
    }
}

void useSensors(){
    qtr.read(sensorValues);
    Serial.println("Value for left sensor ");
    Serial.println(String(sensorValues[0]));

    Serial.println("Value for middle sensor");
    Serial.println(String(sensorValues[1]));

    Serial.println("Value for right sensor ");
    Serial.println(String(sensorValues[2]));

    int direction = calculate_direction(sensorValues);

    drive(direction);
}

void led_on(int led){
    Serial.println("LED on");
    digitalWrite(led, HIGH);
}

void led_off(int led){
    Serial.println("LED off");
    digitalWrite(led, LOW);
}

void motor_stop(int duration){
    Serial.println("Stopping");

    digitalWrite(motorLeft[0], LOW);
    digitalWrite(motorLeft[1], LOW);

    digitalWrite(motorRight[0], LOW);
    digitalWrite(motorRight[1], LOW);

    delay(duration);
}

void turn_right_hard(int duration){
    Serial.println("Turning right hard");

    digitalWrite(motorLeft[0], HIGH);
    digitalWrite(motorLeft[1], LOW);

    digitalWrite(motorRight[0], HIGH);
    digitalWrite(motorRight[1], LOW);

    delay(duration);
}

void turn_right_gradual(int duration){
    Serial.println("Turning right gradually");

    digitalWrite(motorLeft[0], LOW);
    digitalWrite(motorLeft[1], LOW);

    digitalWrite(motorRight[0], HIGH);
    digitalWrite(motorRight[1], LOW);

    delay(duration);
}

void turn_left_hard(int duration){
    Serial.println("Turning left hard");

    digitalWrite(motorLeft[0], LOW);
    digitalWrite(motorLeft[1], HIGH);

    digitalWrite(motorRight[0], LOW);
    digitalWrite(motorRight[1], HIGH);

    delay(duration);
}

void turn_left_gradual(int duration){
    Serial.println("Turning left gradually");

    digitalWrite(motorLeft[0], LOW);
    digitalWrite(motorLeft[1], HIGH);

    digitalWrite(motorRight[0], LOW);
    digitalWrite(motorRight[1], LOW);

    delay(duration);
}

void drive_forward(int duration){
    Serial.println("Driving forward");

    digitalWrite(motorLeft[0], LOW);
    digitalWrite(motorLeft[1], HIGH);

    digitalWrite(motorRight[0], HIGH);
    digitalWrite(motorRight[1], LOW);
    
    delay(duration);
}

void drive_backward(int duration){
    Serial.println("Driving backward");

    digitalWrite(motorLeft[0], HIGH);
    digitalWrite(motorLeft[1], LOW);

    digitalWrite(motorRight[0], LOW);
    digitalWrite(motorRight[1], HIGH);

    delay(duration);
}

int calculate_direction(uint16_t sensorValues[]){
    if(black_line_in_middle()){
        return STRAIGHT_AHEAD;
    }else if (right_hard_turn()) {
        return TURN_RIGHT_HARD;
    }else if (left_hard_turn()){
        return TURN_LEFT_HARD;
    }else if(black_line_on_left()) {
        return TURN_LEFT_GRADUAL;
    }else if(black_line_on_right()){
        return TURN_RIGHT_GRADUAL;
    }
    return STRAIGHT_AHEAD;
}

bool black_line_in_middle() {
    return sensorValues[MIDDLE_SENSOR] > sensorValues[LEFT_SENSOR] && 
    sensorValues[MIDDLE_SENSOR] > sensorValues[RIGHT_SENSOR];
}

bool right_hard_turn() {
    int disparityBetweenSensors = sensorValues[RIGHT_SENSOR] - sensorValues[LEFT_SENSOR];    

    return disparityBetweenSensors >= LARGE_DISPARITY_BETWEEN_SENSORS;
}

bool left_hard_turn() {
    int disparityBetweenSensors = sensorValues[LEFT_SENSOR] - sensorValues[RIGHT_SENSOR];

    return disparityBetweenSensors >= LARGE_DISPARITY_BETWEEN_SENSORS;
}

bool black_line_on_left() {
    return sensorValues[LEFT_SENSOR] > sensorValues[MIDDLE_SENSOR] && sensorValues[LEFT_SENSOR] > sensorValues[RIGHT_SENSOR];
}

bool black_line_on_right() {
    return sensorValues[RIGHT_SENSOR] > sensorValues[LEFT_SENSOR] && sensorValues[RIGHT_SENSOR] > sensorValues[MIDDLE_SENSOR];
}
