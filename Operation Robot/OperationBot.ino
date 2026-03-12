#include <Servo.h>

Servo Baseservo;
Servo Servo2;
Servo Servo3;

int basepos;
int basetarget;
int pos2;
int target2;
int pos3;
int target3;
int targetx;
int targety;
int targetz;


int buttonstate;
bool runtask = false;

void setup() {
  pinMode(12,INPUT);
  initialize();
  delay(3000);  
}
  
void loop() {
  // check for button press
  buttonstate = digitalRead(12);
  if(buttonstate == HIGH && !runtask){
    runtask = true;  
  }
  // TASK TO RUN
  if(runtask){
    //survey();
    targetx = -13;  
    targety = 15;
    targetz = 0;
    calcangles();
    adjustservos();
    delay(3000);
    standby();
    runtask = false;
  }
  buttonstate = LOW;
}



void initialize(){
  Baseservo.write(90);
  Servo2.write(0);
  Servo3.write(0);
  Baseservo.attach(2);
  delay(3000);
  Servo2.attach(3);
  delay(3000);
  Servo3.attach(4);
  pos2 = 0;
  pos3 = 0;
  basepos = 90;
}



void survey(){      //basepos, pos2, pos3
  target2 = 90;
  basetarget = 90;
  adjustservos();
}

void standby(){
  basetarget = 90;
  target2 = 0;
  target3 = 0;
  adjustservos();
}


void adjustservos() {    //based off position move
  while (basepos != basetarget || pos2 != target2 || pos3 != target3) {
    // base
    if (basepos < basetarget) {
      basepos += 1;
    } else if (basepos > basetarget) {
      basepos -= 1;
    }
    // servo 2
    if (pos2 < target2) {
      pos2 += 1;
    } else if (pos2 > target2) {
      pos2 -= 1;
    }
    // servo 3
    if (pos3 < target3) {
      pos3 += 1;
    } else if (pos3 > target3) {
      pos3 -= 1;
    }
    Baseservo.write(basepos);
    Servo2.write(pos2);
    Servo3.write(pos3);
    
    // SPEED CONTROL
    delay(30);   
  }
}

void pan(){
  basetarget = 0;
  adjustservos();
  delay(3000);
  basetarget = 180;
  adjustservos();
  delay(3000);
}

void calcangles() {     // use inverse kinematics equations to calculate desired positions
  // link lengths
  const float h  = 21.5;   // shoulder height above table
  const float l2 = 19.5;
  const float l3 = 36;

  //offset for the x
  targetx = targetx - 2.5;

  // reach and height
  float r = sqrt(targetx * targetx + targety * targety);
  float z = targetz-h;   // world z (table) -> shoulder frame

  // Joint 1 (base yaw)
  float theta1 = atan2(targety, targetx);

  // Joint 3 (elbow)
  float num = r*r + z*z - l2*l2 - l3*l3;
  float den = 2.0 * l2 * l3;
  float c3  = num / den;
  c3 = constrain(c3, -1.0f, 1.0f);
  float theta3 = -acos(c3);  // elbow-down

  // Joint 2 (shoulder)
  float theta2 = atan2(z, r) - atan2(l3 * sin(theta3),l2 + l3 * cos(theta3));

  // convert to degrees
  float th1d = theta1 * 180.0 / PI;
  float th2d = theta2 * 180.0 / PI;
  float th3d = theta3 * 180.0 / PI;

  // servo offsets (adjust sign to match hardware)
  const float offset2 = 65;
  const float offset3 = 160;

  basetarget = th1d;
  target2    = th2d + offset2;
  target3    = th3d + offset3;
}

//void findobjects{
//  // get targetx and targety
//  //targetx =
//  //targety =
//  //put them into list
//}

