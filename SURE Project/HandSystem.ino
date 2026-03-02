//Sensor Slots
int sensor[7] = {13, 12, 11, 10, 9, 8,7};
int fsr1;
int fsr2;
int fsr3;
int fsr4;
int fsr5;
int fsr6;

int status = false;
//Actuator
#define ENCA 2
#define ENCB 3
#define PWM 9
#define IN1 5
#define IN2 4
#define con A0

int bend1;  //unused
int pos;
int targetpos;
int reading;



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(ENCA, INPUT);
  pinMode(ENCB, INPUT);
  pinMode(con, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENCA),readEncoder,RISING);
 
}

void loop() {
  // put your main code here, to run repeatedly:
  sensors();
  control();
  drive();
}

void sensors(){
  fsr1 = analogRead(sensor[0]);
  fsr2 = analogRead(sensor[1]);
  fsr3 = analogRead(sensor[2]);
  fsr4 = analogRead(sensor[3]);
  fsr5 = analogRead(sensor[4]);
  fsr6 = analogRead(sensor[5]);
  bend1 = analogRead(sensor[6]);

  Serial.print("fsr1 = ");
  Serial.print (fsr1);Serial.print(" ");
  Serial.print("fsr2 = ");
  Serial.print (fsr2);Serial.print(" ");
  Serial.print("fsr3 = ");
  Serial.print (fsr3);Serial.print(" ");
  Serial.print("fsr4 = ");
  Serial.print (fsr4);Serial.print(" ");
  Serial.print("fsr5 = ");
  Serial.print (fsr5);Serial.print(" ");
  Serial.print("fsr6 = ");
  Serial.println (fsr6);
  Serial.println();
  Serial.println();
//  delay(250);
//Serial.print(pos); Serial.print(" "); Serial.println(targetpos);
//  Serial.println(targetpos-pos);

}

void readEncoder(){
  int b = digitalRead(ENCB);
  if(b>0){
    pos++;
  }else{
    pos--;
  }
}

void control(){
  reading = analogRead(con);
  targetpos = map(reading, 0, 1023, -1200, 1200);
  delay(250);
}

void drive(){
  //positive e go counter clockwise
  //negative e go clockwise
   if(targetpos-pos<-100){
    digitalWrite(IN1,HIGH);
    digitalWrite(IN2,LOW);
  }else if(targetpos-pos>100){
    digitalWrite(IN1,LOW);
    digitalWrite(IN2,HIGH);
  }else{
    digitalWrite(IN1,LOW);
    digitalWrite(IN2,LOW);
  }
}

