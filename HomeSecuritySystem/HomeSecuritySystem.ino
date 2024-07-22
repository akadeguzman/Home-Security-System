#include <SPI.h>
#include <Wire.h> 
#include <RFID.h>
#include <Servo.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

Servo lockServo;
int lock = 100;      //Locked position limit
int unlock = 0;     //Unlocked position limit

#define SS_PIN 14
#define RST_PIN 15
RFID rfid(SS_PIN, RST_PIN);
unsigned char status; 
unsigned char str[MAX_LEN];

#define LED_G 8
#define LED_R 9

const int btn = 13;
int buttonState = 0;
const int buzzer = 12;


bool SystemStatus = false;


//Keypad DOOR
char keypressed = 0;
char tempPass[4];
int pwCursor = 0;
const byte rows= 4;
const byte cols= 3;
char keymap[rows][cols]= { // for 4x3
  {'1', '2', '3'},
  {'4', '5', '6'},  
  {'7', '8', '9'},  
  {'*', '0', '#'} };

//pins
byte rowPins[rows] = {34,32,30,28}; //Rows 0 to 3
byte colPins[cols]= {26,24,22}; //Columns 0 to 3
Keypad myKeypad = Keypad(makeKeymap(keymap), rowPins, colPins, rows, cols);


//Keypad SYSTEM
char SYS_keypressed = 0;
char SYS_tempPass[4];
char SYS_initial_password[4];
char SYS_NewPassword[4];
int SYS_pwCursor = 0;
const byte SYS_rows= 4;
const byte SYS_cols= 4;
char SYS_keymap[SYS_rows][SYS_cols]= { // for 4x4
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},  
  {'7', '8', '9', 'C'},  
  {'*', '0', '#', 'D'} };

//pins
byte SYS_rowPins[SYS_rows] = {37,35,33,31}; //Rows 0 to 3
byte SYS_colPins[SYS_cols]= {29,27,25,23}; //Columns 0 to 3
Keypad SYS_Keypad = Keypad(makeKeymap(SYS_keymap), SYS_rowPins, SYS_colPins, SYS_rows, SYS_cols);

//other sensors
int waterSensor = A0; //water sensor
int flameSensor = A7;  //flame sensor
int smokeSensor = A1; //smoke sensor

int PIRroom1 = 7;
int PIRroom1Val1 = 0;
int stateRoom1 = LOW;
int PIRroom2 = 6;
int PIRroom1Val2 = 0;
int stateRoom2 = LOW;
int PIRroom3 = 5;
int PIRroom1Val3 = 0;
int stateRoom3 = LOW;

int reedSW1 = 3;
int reedState1 = 0;
int reedSW2 = 2;
int reedState2 = 0;

bool waterIsDetected;
bool flameIsDetected;
bool smokeIsDetected;

int zone = 0;

int alarmStatus = 0;





//S E T U P
void setup() {
  Serial.begin(9600);
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(3,0);
  lcd.print("Initiating");
  lcd.setCursor(5,1);
  lcd.print("System");
  delay(2000);

  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("Home System:");
  if (SystemStatus == true) { 
    lcd.setCursor(4,1);
    lcd.print("ENABLED");
  }
  else {
    lcd.setCursor(4,1);
    lcd.print("DISABLED");
  }

  
  
  lockServo.attach(10);
  lockServo.write(lock);

  pinMode(PIRroom1, INPUT); //PIR sensor room 1
  pinMode(PIRroom2, INPUT); //PIR sensor room 1
  pinMode(PIRroom3, INPUT); //PIR sensor room 1

  pinMode(waterSensor ,INPUT);
  pinMode(flameSensor,INPUT);
  pinMode(smokeSensor,INPUT);

  pinMode(reedSW1,INPUT);
  pinMode(reedSW2,INPUT);
  
  pinMode(btn, INPUT); // btn for Manual Unlock
  pinMode(buzzer, OUTPUT); //BUZZER
  digitalWrite(buzzer, LOW); //for buzzer

  pinMode(LED_G, OUTPUT); // Confirmed LED
  pinMode(LED_R, OUTPUT); // Denied LED

  myKeypad.addEventListener(KeypadControls);
  SPI.begin();   
  rfid.init();
  initialpassword();
}


// L O O P
void loop() 
{
  myKeypad.getKey();
  SYS_keypressed = SYS_Keypad.getKey();
  if(SystemStatus == true){
    SensorsController();
  }
  
  
   
  buttonState = digitalRead(btn);
  
  //Manual Button OPEN
  if(buttonState == HIGH)
  {
    ManualUnlock();
  }

 
  //K E Y P A D S
  if(SYS_keypressed){

    if(SYS_keypressed == '#') change(); 
    SYS_tempPass[SYS_pwCursor++] = SYS_keypressed;
    SYS_KeypadControls();
  }

  //RFID
  RFIDControls();
}




//----------------------------------------

// M E T H O D S


void SuccessIndicator(){
  digitalWrite(buzzer, HIGH);
  digitalWrite(LED_G, HIGH);
  delay(100);
  digitalWrite(buzzer, LOW);
  digitalWrite(LED_G, LOW);
  delay(100);
  digitalWrite(buzzer, HIGH);
  digitalWrite(LED_G, HIGH);
  delay(100);
  digitalWrite(buzzer, LOW);
  digitalWrite(LED_G, LOW);
  delay(100);
}

void ErrorIndicator(){
  digitalWrite(buzzer, HIGH);
  digitalWrite(LED_R, HIGH);
  delay(300);
  digitalWrite(buzzer, LOW);
  digitalWrite(LED_R, LOW);
  delay(300);
  digitalWrite(buzzer, HIGH);
  digitalWrite(LED_R, HIGH);
  delay(300);
  digitalWrite(buzzer, LOW);
  digitalWrite(LED_R, LOW);
  delay(300);
   digitalWrite(buzzer, HIGH);
  digitalWrite(LED_R, HIGH);
  delay(300);
  digitalWrite(buzzer, LOW);
  digitalWrite(LED_R, LOW);
  delay(300);  
}


//This will display the status of the system if it is enabled or disabled
void SystemStatusDisplay(){
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("Home System:");
  if (SystemStatus == true) { 
    lcd.setCursor(4,1);
    lcd.print("ENABLED");
    Serial.print("Status: ENABLED");
  }
  else {
    lcd.setCursor(4,1);
    lcd.print("DISABLED");
    Serial.print("Status: DISABLED");
  }
}

//Method for RFID
void RFIDControls(){

  if(rfid.findCard(PICC_REQIDL, str) == MI_OK){
    Serial.println("Card found"); 
    String content= "";
    byte letter;
    if (rfid.anticoll(str) == MI_OK)
    {
      Serial.print("The card's ID number is : ");
      for (byte i = 0; i < 4; i++) 
      {
          content = content + (0x0F & (str[i] >> 4)); 
          content = content + (0x0F & str[i]);
      }
      Serial.println (content);
      checkAccess(content);
    }    
    rfid.selectTag(str); 
  }
  rfid.halt();
}


void checkAccess(String temp){
  if (temp == "13311592110") //change this !!!
    {
           
      //LED and BUZZER
      Serial.print("Card Number: ");
      SuccessIndicator();
      
      lockServo.write(unlock);
      delay(5000);
      lockServo.write(lock);
      delay(500);
     
    }
   
   else   {
      //LED and BUZZER
      ErrorIndicator();
    }
}


//For using the PUSH button for manually overriding the lock
void ManualUnlock(){
   Unlock();
}


//LOCK or UNLOCKING the servo motor
void Unlock(){
  //LED and BUZZER
    SuccessIndicator();
    
    lockServo.write(unlock);
    delay(5000);
    lockServo.write(lock);
    delay(500);
}


//-------------------------------------------------------------------------<<

//For the DOOR Keypad Matrix

void KeypadControls(KeypadEvent key)
{
  switch (myKeypad.getState()){
    case PRESSED:
    tempPass[pwCursor++] = key;
    digitalWrite(buzzer, HIGH);
    delay(100);
    digitalWrite(buzzer, LOW);
    delay(100);

    Serial.println(tempPass);
    Serial.println(pwCursor);   
    
  }
  

    if (pwCursor >= 4){
  
    for(int j=0;j<4;j++){
     SYS_initial_password[j]=EEPROM.read(j);
    }
    
    if(!strncmp(tempPass,  SYS_initial_password, 4))
    {
      Unlock();
      pwCursor = 0;
      memset(tempPass, 0, 4);
      keypressed = 0;
    }
    else 
    {
      ErrorIndicator();
      pwCursor = 0;
      memset(tempPass, 0, 4);
      keypressed = 0;
    }
  }    
}


//-------------------------------------------------------------------------<<

//For the security system keypad matrix

void SYS_KeypadControls()
{
  if(SYS_keypressed)
  {
    digitalWrite(buzzer, HIGH);
    delay(100);
    digitalWrite(buzzer, LOW);
    delay(100);
    Serial.println(SYS_keypressed);

  }
  
  if (SYS_pwCursor == 4){
    
     for(int j=0;j<4;j++){
      SYS_initial_password[j]=EEPROM.read(j);
      }

      if(!strncmp(SYS_tempPass, SYS_initial_password, 4))
      {
        // SET THE SENSORS HERE!!! -----------------------------------------------------<<
         lcd.clear();
        SystemStatus = !SystemStatus;
        
        SystemStatusDisplay();
        SuccessIndicator();

        if(alarmStatus == 1){
          alarmStatus = !alarmStatus;
          AlarmNoise();
        }

        //END
        SYS_pwCursor = 0;
        memset(SYS_tempPass, 0, 4);
        SYS_keypressed = 0;
      }
      else 
      {
        ErrorIndicator();
        SYS_pwCursor = 0;
        memset(SYS_tempPass, 0, 4);
        SYS_keypressed = 0;
      }
    }
  
}


//------------------------------------------------------------------------------------

//Setting the default password
void initialpassword(){

  for(int j=0;j<4;j++)

    EEPROM.write(j, j+49);

  for(int j=0;j<4;j++)

     SYS_initial_password[j]=EEPROM.read(j);
}



//For CHANGING the password
void change()
{
  int j=0;
  lcd.clear();
  lcd.print("Current Password:");
  lcd.setCursor(6,1);

  while(j<4)
  {
    char key = SYS_Keypad.getKey();
    
    if(key)
    {
      digitalWrite(buzzer, HIGH);
      delay(100);
      digitalWrite(buzzer, LOW);
      delay(100);

      SYS_NewPassword[j++]=key;
      lcd.print("*");   
    }
    key = 0;
  }
  delay(500);


  if((strncmp(SYS_NewPassword, SYS_initial_password, 4)))
  {
    lcd.clear();
    ErrorIndicator();
    lcd.print("Wrong Password");
    lcd.setCursor(0,1);
    lcd.print("Try Again");
    delay(1000);
  }

  else
  {
    j=0;
    lcd.clear();
    lcd.print("New Password:");
    lcd.setCursor(0,1);
    
    while(j<4)
    {
      char key = SYS_Keypad.getKey();
      
      if(key)
      {
        digitalWrite(buzzer, HIGH);
        delay(100);
        digitalWrite(buzzer, LOW);
        delay(100);

        SYS_initial_password[j] = key;
        lcd.print("*");
        EEPROM.write(j,key);
        j++;
      }
    }
    lcd.clear();
    lcd.setCursor(2,0);
    SuccessIndicator();
    lcd.print("PASS CHANGED");
    delay(1000);
  }

  SystemStatusDisplay();
  SYS_keypressed = 0;

}


// O T H E R  _  SENSORS -----------------------------------<<

//WATER sensor
void waterSensorModule(){
  int sensorValue = analogRead(A0);
  Serial.print("Water Sensor = ");
  Serial.print(sensorValue);
  if(sensorValue <= 128)
  waterIsDetected = false;
  else{
    zone = 8;
    AlarmTriggered();
  }
}


//FLAME sensor
void flameSensorModule(){
  int sensorValue = analogRead(A2);
  Serial.print("Flame Sensor = ");
  Serial.print(sensorValue);
   if(sensorValue <=512) {
    flameIsDetected = false;
   }
   else {
   
    zone = 6;
    AlarmTriggered();
   }
}


//SMOKE Sensor
void smokeSensorModule(){
   int sensorValue = analogRead(A1);
  Serial.print("Smoke Sensor = ");
  Serial.print(sensorValue);
   if(sensorValue > 400) {
    zone = 7;
    AlarmTriggered();
   }
   else{
    
    smokeIsDetected = false;
   }
}


//PIR sernsor
void PIRSensor() {
  PIRroom1Val1 = digitalRead(PIRroom1);
  PIRroom1Val2 = digitalRead(PIRroom2);
  PIRroom1Val3 = digitalRead(PIRroom3);

  Serial.print("PIR 1 = ");
  Serial.println(PIRroom1Val1);
  Serial.print("PIR 2 = ");
  Serial.println(PIRroom1Val2);
  Serial.print("PIR 3 = ");
  Serial.println(PIRroom1Val3);
 // R O O M - 1
  if (PIRroom1Val1 == HIGH) { // check if the sensor is HIGH
    zone = 1; 
    AlarmTriggered();

    
    if (stateRoom1 == LOW) {
      stateRoom1 = HIGH;       // update variable state to HIGH
     }
  } 
  else {
      if (stateRoom1 == HIGH){
        stateRoom1 = LOW;      // update variable state to LOW
    }
  }


  // R O O M - 2
  if (PIRroom1Val2 == HIGH) {  // check if the sensor is HIGH
    zone = 2; 
    AlarmTriggered();
    
    if (stateRoom2 == LOW) { 
      stateRoom2 = HIGH;       // update variable state to HIGH
     }
  } 
  else {
      if (stateRoom2 == HIGH){
        stateRoom2 = LOW;       // update variable state to LOW
    }
  }


  // R O O M - 3
  if (PIRroom1Val3 == HIGH) {   // check if the sensor is HIGH
    zone = 3;
    AlarmTriggered();
    
    if (stateRoom3 == LOW) {
      stateRoom3 = HIGH;       // update variable state to HIGH
     }
  } 
  else {
      if (stateRoom3 == HIGH){
        stateRoom3 = LOW;       // update variable state to LOW
    }
  }
}



//For the Windows and Door alarm
void ReedSW(){
  
  //WINDOWS switch
  reedState1 = digitalRead(reedSW1);

  Serial.print("Windows = ");
  Serial.println(reedState1);
  
  if (reedState1 == HIGH) //the windows/door is OPEN
  {
    zone = 4;
    AlarmTriggered();
  } 
  

  //BACK DOOR switch
  reedState2 = digitalRead(reedSW2);

  Serial.print("Doors = ");
  Serial.println(reedState2);
    
  if (reedState2 == HIGH) //the windows/door is OPEN
  {
    zone = 5;
    AlarmTriggered();
  } 
}

//method for calling the sensors if the system is enabled
void SensorsController(){
  //If the SYSTEM is ENABLED, the sensors will be activated
  ReedSW();
  PIRSensor();
  smokeSensorModule();
  flameSensorModule();
  waterSensorModule();
  
}


//Method for identifying the triggered sensor and acticvating the noise.
void AlarmTriggered(){
  alarmStatus = 1;
  AlarmNoise();
  //reset keypad
  SYS_pwCursor = 0;
  memset(SYS_tempPass, 0, 4);
  SYS_keypressed = 0;

  //reset system keypad
  pwCursor = 0;
  memset(tempPass, 0, 4);
  keypressed = 0;

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SYSTEM TRIGGERED");
  lcd.setCursor(0,1);
  Serial.println("SYSTEM TRIGGERED");
  
  
  if (zone == 1)
  { 
    lcd.print("Motion in Room 1");
    delay(1000);
  }
  if (zone == 2)
  { 
    lcd.print("Motion in Room 2");
    delay(1000);
  }
  if (zone == 3)
  { 
    lcd.print("Motion in Room 3");
    delay(1000);
  }
  if (zone == 4)
  { 
    lcd.print("Windows OPEN");
    delay(1000);
  }
  if (zone == 5)
  { 
    lcd.print("Backdoor OPEN");
    delay(1000);
  }


  //other sensors
  if (zone == 6)
  { 
    lcd.print("Fire Detected!");
    delay(1000);
  }

  if (zone == 7)
  { 
    lcd.print("Smoke Detected!");
    delay(1000);
  }
  
  if (zone == 8)
  { 
    lcd.print("Water Detected!");
    delay(1000);
  }
}

//Buzzer Noise
void AlarmNoise(){
  if(alarmStatus == 1){
    digitalWrite(buzzer, HIGH);
    digitalWrite(LED_R, HIGH);
  }
  else
  {
     digitalWrite(buzzer, LOW);
    digitalWrite(LED_R, LOW);
  }
}
