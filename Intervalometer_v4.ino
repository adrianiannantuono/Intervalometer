#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

float battVoltage = 0.0;

const int timeBetweenPhotos = 1000;
//const int timeBetweenPhotos[] = {};
int photosTaken = 0;
int run = 0;
boolean finished = false;
boolean autoShutterspeed = false;
int photoresistor;
boolean selectElemButtonPressed = false;
boolean changeElementButtonPressed = false;
boolean autoShutterSpeedButtonPressed = false;
String currentPage = "home";
int selectedElem = 0;

const double shutterSpeedValues[] = {100, 150, 200, 250, 500, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000, 15000, 20000, 25000, 30000, 35000};
int shutterSpeedIter = 0;
const int numberOfPhotosValues[] = {1, 5, 10, 20, 50, 100, 150, 200, 250, 300, 400, 500, 600, 700, 800, 900, 1000};
int numberOfPhotosIter = 0;

int timer = 0;

static const unsigned char PROGMEM selectSymbol[] =
{ B10000000, B00000000,
  B11100000, B00000000,
  B11111000, B00000000,
  B11111110, B00000000,
  B11111111, B10000000,
  B11111111, B10000000,
  B11111110, B00000000,
  B11111000, B00000000,
  B11100000, B00000000,
  B10000000, B00000000 };

void setup() {
  Serial.begin(9600);
  // Photoresistor analog input
  pinMode(A0, INPUT);
  // Run digital input
  pinMode(4, INPUT_PULLUP );
  // Auto shutterspeed digital input
  pinMode(5, INPUT_PULLUP );
  // Change shutterspeed user digital input
  pinMode(7, INPUT_PULLUP );
  // Change number of photos user digital input
  pinMode(8, INPUT_PULLUP );
  // Shutter digital output
  pinMode(3, OUTPUT);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(22,10);
  display.println("Intervalometer");
  display.setCursor(10,30);
  display.print("Adrian Iannantuono");
  display.display();
  delay(3500);
}

void loop() {
  if (currentPage == "home") {
    battVoltage = analogRead(A2)*5.04/1024*2031300/1026300;
    //0.00949*analogRead(A2) - 0.01548;
    run = !digitalRead(4);
    if (autoShutterSpeedButtonPressed == false && !digitalRead(5)) {
      if (!digitalRead(5) && autoShutterspeed == false) {
        autoShutterspeed = true;
      } else if (!digitalRead(5) && autoShutterspeed == true) {
        autoShutterspeed = false;
      }
      autoShutterSpeedButtonPressed = true;
    } else if (autoShutterSpeedButtonPressed && digitalRead(5)) {
        autoShutterSpeedButtonPressed = false;
    }
    if (autoShutterspeed) {
      photoresistor = analogRead(A0);
    }
    if (run) {
      if (photosTaken < numberOfPhotosValues[numberOfPhotosIter]) {
        digitalWrite(3, HIGH);
        if (autoShutterspeed) {
          delay(shutterSpeedValues[shutterSpeedIter]);
        } else {
          delay(shutterSpeedValues[shutterSpeedIter]);
        }
        digitalWrite(3, LOW);
        photosTaken++;
        delay(timeBetweenPhotos);
      } else if (photosTaken = numberOfPhotosValues[numberOfPhotosIter]) {
        finished = true;
      }
    } else {
      if (photosTaken = numberOfPhotosValues[numberOfPhotosIter]) {
        finished = false;
        photosTaken = 0;
      }
      if (selectElemButtonPressed == false && !digitalRead(7)) {
        if (selectedElem < 2) {
          selectedElem++;
        } else {
          selectedElem = 0;
        }
        selectElemButtonPressed = true;
      } else if (selectElemButtonPressed && digitalRead(7)) {
        selectElemButtonPressed = false;
      }
      if (changeElementButtonPressed == false && !digitalRead(8)) {
        if (selectedElem == 0) {
          if (shutterSpeedIter < (sizeof(shutterSpeedValues)-1)/sizeof(double)) {
            shutterSpeedIter++;
          } else {
            shutterSpeedIter = 0;
          }
        } else if (selectedElem == 1) {
          if (numberOfPhotosIter < (sizeof(numberOfPhotosValues)-1)/sizeof(int)) {
            numberOfPhotosIter++;
          } else {
            numberOfPhotosIter = 0;
          }
        }
        changeElementButtonPressed = true;
      } else if (changeElementButtonPressed && digitalRead(8)) {
        changeElementButtonPressed = false;
      }
    }
    updateDisplay_home();
  }
  //serialUpdate();
}


void serialUpdate() {
  Serial.print("Shutter Speed: ");
  Serial.print(shutterSpeedValues[shutterSpeedIter]);
  Serial.print("  Photos: ");
  Serial.print(photosTaken);
  Serial.print(" / ");
  Serial.print(numberOfPhotosValues[numberOfPhotosIter]);
  Serial.print(" Auto Shutter: ");
  Serial.print(autoShutterspeed);
  Serial.println("");
  Serial.print("Photoresistor: ");
  Serial.print(photoresistor);
  Serial.println("");
}

void updateDisplay_home() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.drawLine(0, 8, 128, 8, SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(20,15);
  if (shutterSpeedValues[shutterSpeedIter] >= 1000) {
    display.print(int(shutterSpeedValues[shutterSpeedIter]/1000));
  } else {
    display.print("1/");
    display.print(int(1000/shutterSpeedValues[shutterSpeedIter]));
  }
  display.setTextSize(1);
  display.print(" sec");
  if (autoShutterspeed) {
    display.drawRoundRect(100, 16, 28, 10, 3, SSD1306_WHITE);
    display.setCursor(103,17);
    display.print("auto");
  }
  display.setCursor(20,35);
  display.setTextSize(2);
  if (!finished) {
    display.print(photosTaken);
    display.print("/");
    display.print(numberOfPhotosValues[numberOfPhotosIter]);
    display.setTextSize(1);
    if (numberOfPhotosValues[numberOfPhotosIter] >= 1000) {
      display.println("photos");
    } else {
      display.println(" photos");
    }
  } else {
    display.println("Finished");
  }
  display.setCursor(20,54);
  display.setTextSize(1);
  display.println("Settings");
  display.print("Photoresistor: ");
  display.print(photoresistor);

  if (!run) {
     timer = timer + 1;
    if (battVoltage < 7.0 && timer < 50) {
      display.setCursor(92, 0);
      display.print(battVoltage);
      display.print(" V");
    } else if (battVoltage < 7.0 && timer < 100) {
      display.setCursor(80, 0);
      display.print("LOW BATT");
    } else {
      timer = 0;
      display.setCursor(92, 0);
      display.print(battVoltage);
      display.print(" V");
    }
  } else {
    display.setCursor(92, 0);
    display.print(battVoltage);
    display.print(" V");
  }


  if (selectedElem == 0) {
    display.drawBitmap(5, 17, selectSymbol, 16, 10, SSD1306_WHITE);
  } else if (selectedElem == 1) {
    display.drawBitmap(5, 37, selectSymbol, 16, 10, SSD1306_WHITE);
  } else if (selectedElem == 2) {
    display.drawBitmap(5, 53, selectSymbol, 16, 10, SSD1306_WHITE);
  }
  display.display();
}
