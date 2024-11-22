// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "Arduino.h"
#include "I2Cdev.h"
#include "MPU6050.h"
#include "Adafruit_BMP085.h"
#include "SD.h"
#include "SPI.h"
#include "FS.h"
#include <UMS3.h>

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 accelgyro;
//MPU6050 accelgyro(0x69); // <-- use for AD0 high

int16_t ax, ay, az;
int16_t gx, gy, gz;

Adafruit_BMP085 bmp;

File dataFile;
String fileName = "/Rocket";
const int chipSelect = 5;
String dataString = "";

unsigned long previousMillis = millis();
long interval = 1000;

UMS3 ts;

double zeroAlt = 0;
bool arm = false;
bool runOnce = true;
bool deployed = false;
#define transistor 2

void setup() {
    // join I2C bus (I2Cdev library doesn't do this automatically)
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    Wire.begin();
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
    Fastwire::setup(400, true);
    #endif

    pinMode(transistor, OUTPUT);

    // Initialize all board peripherals, call this first
    ts.begin();

    // Brightness is 0-255. We set it to 1/3 brightness here
    ts.setPixelBrightness(255);

    //initialize serial communication
    Serial.begin(115200);
    //Serial2.begin(9600, SERIAL_8N1, 33, 32);
    //while(!Serial); // wait for serial port to connect. Needed for native USB port only

    //initialize bmp
    if (!bmp.begin()) {
        Serial.println("Could not find a valid BMP085 sensor, check wiring!");
        ts.setPixelColor(255, 255, 0);//yellow
        while (1);
    } else {
        Serial.println("BMP180 connection successful");
    }

    // initialize device
    Serial.println("Initializing I2C devices...");
    accelgyro.initialize();
    // verify connection
    Serial.println("Testing device connections...");
    if (!accelgyro.testConnection()) {
        Serial.println("MPU6050 connection failed");
        ts.setPixelColor( 0, 0, 255);//blue
        while (1) {}
    } else {
        accelgyro.CalibrateAccel();
        accelgyro.CalibrateGyro();
        accelgyro.setFullScaleAccelRange(3);
        Serial.println("MPU6050 connection successful");
    }
    

    zeroAlt = 0;

    for(int i = 0; i <= 10; i++){
        zeroAlt = zeroAlt + bmp.readAltitude(101500);
    }
    
    zeroAlt = (zeroAlt/10) - 1;
    digitalWrite(transistor, LOW);

    ts.setPixelColor( 0, 255, 0);//green
    delay(2000);
    ts.setPixelPower(false);//turn off light
}

double ftToM(double feet){
    return(feet * 0.3048);
}

double mToFt(double meters){
    return(meters * 3.281);
}

void loop() {
    //dataString = "";

    unsigned long currentMillis = millis();

    // read raw accel/gyro measurements from device
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    Serial.println(bmp.readAltitude(101500) - zeroAlt);

    
    if(!runOnce && !deployed){
        if((bmp.readAltitude(101500) - zeroAlt) <= 40 && arm){
            deployed = true;
            Serial.println("deployed");
            digitalWrite(transistor, HIGH);
        }
    }
    
    if(runOnce){
        if((bmp.readAltitude(101500) - zeroAlt) > 50){
            arm = true;
            runOnce = false;
            Serial.println("armed");
        }
        else{

        }
    }

}
