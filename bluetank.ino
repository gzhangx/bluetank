

/* 
 drive remote control car wit bluetooth
 */

#include "SoftwareSerial.h"
const int BLUEINT = 3;
SoftwareSerial BTSerial(BLUEINT,10); // blue tx, blue rx

const int BT_BUF_LEN=128;
char sendstr[BT_BUF_LEN+16];
int sendstrpos = 0; 
unsigned long lastAvailableTime = millis();

int motorCounter = 0;
int motorCounterMax = 10;
int motorPins[][2] = {
  {5,6},
  {7,8}
};
int motorSpeed[]={0,0};


void serprintln(String s) {
  if (Serial) Serial.println(s);           
}

String blueReportStr = "";
String curWorkingBlueReportStr = "";
int curWorkingBlueReportStrProg=0;
unsigned long lastBlueTime = millis();
void actualStateBlueReport() {
  if (millis() - lastBlueTime < 500) return;
  if (curWorkingBlueReportStr == "") {
    curWorkingBlueReportStr = blueReportStr;
  }
  if (curWorkingBlueReportStr == "") return;
  if (curWorkingBlueReportStrProg < curWorkingBlueReportStr.length()){
     BTSerial.write(curWorkingBlueReportStr[curWorkingBlueReportStrProg++]);
  }else {
    lastBlueTime = millis();
    BTSerial.write('\n');
     curWorkingBlueReportStr = blueReportStr;     
     curWorkingBlueReportStrProg=0;
     blueReportStr = "";
  }       
}

void blueReport(String s) {
  blueReportStr = s;  
  Serial.println(s);
}

void setup() {
    Serial.begin(115200);
    serprintln("serial initialized");
    BTSerial.begin(9600);

    for (int i = 0; i < 2; i++) {
      for(int k = 0; k < 2; k++) {
        int cur = motorPins[i][k];
        pinMode(cur, OUTPUT);
      }
    }
}


void loop() { 
  //loop_bt();;
  //loop_simple();
  //loop_motor();
  loop_run();
  actualStateBlueReport();
}

void btCmdReceived(String cmd, String name, String val) {  
  Serial.println("gotxcmd " + cmd+ " val " + val);
  if (cmd == "l")  {
    motorSpeed[0] = val.toInt();
    blueReport("l="+String(motorSpeed[0]));
  }
  if (cmd == "r")  {
    motorSpeed[1] = val.toInt();
    blueReport("r="+String(motorSpeed[1]));
  }
}
char receiveStr[BT_BUF_LEN+16];
int receivePos = 0;
String curBtCmd = "";
String curBtName = "";
void loop_bt() {  
  while (BTSerial.available()){
        int c = BTSerial.read();
        if (receivePos < BT_BUF_LEN) {
          receiveStr[receivePos++] = (char)c;
          receiveStr[receivePos] = 0;
          if (c == ':') {
            receiveStr[receivePos-1]=0;
            if (curBtCmd == "") {
              curBtCmd = receiveStr;
            } else  {
              curBtName = receiveStr; 
            }
            receivePos = 0;
          }
          if (c == '|') {            
            receiveStr[receivePos-1]=0;            
            receivePos = 0;
            String val = receiveStr;
            btCmdReceived(curBtCmd, curBtName, val);
            curBtCmd = "";
          }
        }else {
          receivePos = 0; //warning, over flow
        }
        //Serial.write(c);
  }
  // Keep reading from Arduino Serial Monitor and send to HC-05
  if (Serial.available()){
    int c = Serial.read();
    sendstr[sendstrpos++] =c;
    if (sendstrpos > 100) sendstrpos = 100;
    sendstr[sendstrpos] =0;    
    lastAvailableTime = millis();    
  }else {
    if (sendstrpos && (millis()-lastAvailableTime)>500  ) {
      for (int i = 0; i < sendstrpos; i++)
        BTSerial.write(sendstr[i]);   
      BTSerial.write('\r');  
      BTSerial.write('\n');
      Serial.println(sendstr);
      sendstrpos = 0;
    }
    
  }
}

void loop_simple() {
  int who = 1;
   if (Serial.available()){
    char dbgstr[128];
    int c = Serial.read();
    if (c == 'F') motorControlDrive(who,'F');
    if (c == 'R') motorControlDrive(who,'R');
    if (c == 'S') motorControlDrive(who,'S');    
  }
}


void loop_run() {  
  loop_bt();
  motorCounter++;
  if (motorCounter>=motorCounterMax)motorCounter=0;  
  motorControlAll();  
}

void motorControlAll() {
  for (int i = 0; i < 2; i++)
    motorControl(i);  
}

void motorControl(int who) {
  int speed = motorSpeed[who];
  char dir = 'S';
  if (speed > 0) dir = 'F';
  if (speed < 0) {
    dir = 'R';
    speed = -speed;
  }  
  if (speed > motorCounter) {
    motorControlDrive(who, dir);
  }else {
    motorControlDrive(who, 'S');
  }
}

void motorControlDrive(int who, char dir){
  int pin1 = motorPins[who][0];
  int pin2 = motorPins[who][1];

  if (dir == 'R') {
    digitalWrite(pin1, 0);
    digitalWrite(pin2, 1);    
  }else if (dir == 'F') {
    digitalWrite(pin1, 1);
    digitalWrite(pin2, 0);    
  } else {
    digitalWrite(pin1, 1);
    digitalWrite(pin2, 1);
  }
}
