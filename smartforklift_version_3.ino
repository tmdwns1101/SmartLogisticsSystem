#include <SoftwareSerial.h>
#define rxPin 13
#define txPin 12
SoftwareSerial SwSerial(txPin, rxPin);


int A_1A = 9;

int A_1B = 10;

int B_1A = 3;

int B_1B = 5;

int C_1A = 6;

int C_1B = 11;

/* 적외선 센서*/

int pirSensor = 2;

/*

   TCRT5000 라인센서

   왼쪽 라인센서 DO - D2

   오른쪽 라인센서 DO - D3

*/

int leftLineSensor =  7;

int rightLineSensor = 8;




int speed = 100;

int state = 1; //초기 상태 설정

int fork_height=1;


void setup() {
  state = 1; //초기 상태 설정

  fork_height=1;

  Serial.begin(9600);
  SwSerial.begin(9600);
  Serial.println("START");

  //핀을 초기화합니다.

  //L298 모터드라이버의 핀들을 출력으로 변경합니다.

  pinMode(A_1A, OUTPUT);

  pinMode(A_1B, OUTPUT);

  pinMode(B_1A, OUTPUT);

  pinMode(B_1B, OUTPUT);

  pinMode(C_1A, OUTPUT);

  pinMode(C_1B, OUTPUT);

  digitalWrite(A_1A, LOW);

  digitalWrite(A_1B, LOW);

  digitalWrite(B_1A, LOW);

  digitalWrite(B_1B, LOW);

  digitalWrite(C_1A, LOW);

  digitalWrite(C_1B, LOW);



  //PIR센서를 초기화합니다.

  pinMode(pirSensor, INPUT); 


  //TCRT5000 라인센서를 입력으로 변경합니다.

  pinMode(leftLineSensor, INPUT);

  pinMode(rightLineSensor, INPUT);

  
  //digitalWrite(C_1B, speed);
  //delay(4000);
  //digitalWrite(C_1B, 0);
}



void fCrossPoint(int id);  //첫 번째 분기점 제어 함수
void sCrossPoint(int id);  //두 번째 분기점 제어 함수
void sLift(int height);  // 목적지 도착 후 물류 운반 함수
void eLift();            // 적재 장소 도착 후 물류 적재 제어 함수
void rotate();
void mainDrive(String id, String quantity); //라인 트레이싱 및 지게차 전반 제어 함수
void forkHeight();
void turnLeft();
void turnRight();
bool personChecker();

void loop() {

  SwSerial.println("Ready");
  Serial.println("I'm Ready!");
  String input = "";
  bool flag = true;
  delay(1000);
  long startTime = millis();

  while (1) {
    long endTime = millis();

    if (endTime - startTime > 10000) {
      flag = false;
      break;
    }
    if (SwSerial.available()) {

      input = SwSerial.readStringUntil('\n');
      Serial.println(input);
      if (input.equals("end")) { // 프로그램 종료 후 대기
        while (1) {
          if (SwSerial.available()) {
            String re = SwSerial.readStringUntil('\n');
            if (re.equals("Init"))
              break;
          }
        }
      }
      else if (input.equals("Init")) { //초기화 부분
        break;
      }
      else {  //메인 하드웨어 작동 부분
        int sliceIdx = input.indexOf('/');
        String id = input.substring(0, sliceIdx);
        String quantity = input.substring(sliceIdx + 1, input.length());


        Serial.print("ID : ");
        Serial.println(id);
        Serial.print("Quantity : ");
        Serial.println(quantity);

        mainDrive(id, quantity);  //블루투스 test 주석
        delay(5000);
        break;
      }
    }
  }
  if (flag == false) {
    Serial.println("No input");
  }

}



void mainDrive(String target, String quantity) {   //초기 loop문

  Serial.print("current ID : ");
  Serial.println(target);
  Serial.print("current quantity : ");
  Serial.println(quantity);

  int height = quantity.toInt();
  int id = target.toInt();



  while (1) {

    if(personChecker() == true){
      analogWrite(A_1A, 0);

      analogWrite(A_1B, 0);

      analogWrite(B_1A, 0);

      analogWrite(B_1B, 0);

      continue;
    }
    //양쪽 센서가 모두 선이 닿았을 경우 정지합니다.

    if (!digitalRead(leftLineSensor) && !digitalRead(rightLineSensor)) {

      //1초 정지
      //모터A 정지

      analogWrite(A_1A, 0);

      analogWrite(A_1B, 0);

      //모터B 정지

      analogWrite(B_1A, 0);

      analogWrite(B_1B, 0);

      delay(1000);

      if (state == 1) {
        fCrossPoint(id);

        state = 2;
      }

      else if (state == 2) {
        sLift(height);
        state = 3;
      }
      else if (state == 3) {
        sCrossPoint(id);
        state = 4;
      }
      else if (state == 4) {
         if(fork_height==1){
          analogWrite(A_1A, speed); //15->20
          analogWrite(A_1B, 0);
          analogWrite(B_1A, speed);
           analogWrite(B_1B, 0);
           delay(600);
         }
         else {
          analogWrite(A_1A, 0); //15->20
          analogWrite(A_1B, 0);
          analogWrite(B_1A, 0);
           analogWrite(B_1B, 0);
          //지게모터 많이 상승(물건 올리기)
           analogWrite(C_1A, speed+50);
           analogWrite(C_1B, 0);
           delay(12500); //2층 지게 올리는 딜레이
           analogWrite(C_1A, 0);
          analogWrite(A_1A, speed); //15->20
          analogWrite(A_1B, 0);
          analogWrite(B_1A, speed);
           analogWrite(B_1B, 0);
           delay(400);
          
         }
         state =5;
      }
      else {
        eLift();
        state = 1;
        break; // 주기 완료
      }

    } else if (!digitalRead(leftLineSensor) && digitalRead(rightLineSensor)) {  // 우 회전

      //모터A 역회전

      analogWrite(A_1A, 0);

      analogWrite(A_1B, speed);

      //모터B 정회전

      //-20을 하는 이유는 전진하다 역회전하는 반대 바퀴보다 가속도가 붙기 때문입니다.

      analogWrite(B_1A, speed - 20); //15->20

      analogWrite(B_1B, 0);

      //오른쪽 센서가 선을 감지하지 않았을 경우 오른쪽모터는 역회전, 왼쪽모터는 정회전을 하여 좌회전합니다.

      //이때 속도 한쪽을 상대적으로 느리게 설정하는데, 이유는 한쪽은 정지 후 가야하기 때문입니다.

    } else if (digitalRead(leftLineSensor) && !digitalRead(rightLineSensor)) {

      //모터A 정회전

      //-20을 하는 이유는 전진하다 역회전하는 반대 바퀴보다 가속도가 붙기 때문입니다.

      analogWrite(A_1A, speed - 20); //15->20

      analogWrite(A_1B, 0);

      //모터B 역회전

      analogWrite(B_1A, 0);

      analogWrite(B_1B, speed);

      //양쪽모두 선이 닿지 않았을 경우 전진합니다.

    } else if (digitalRead(leftLineSensor) && digitalRead(rightLineSensor)) {

      //모터A 정회전

      analogWrite(A_1A, speed);

      analogWrite(A_1B, 0);

      //모터B 정회전

      analogWrite(B_1A, speed);

      analogWrite(B_1B, 0);

    }
  }
}
bool personChecker(){
  if(digitalRead(2) == 1){
   Serial.println("사림이 인식되었습니다.");
   return true;
  } 
  else {
    Serial.println("사람이 인식되지 않습니다.");
    return false;
  }
}


void fCrossPoint(int id) {
  //약간 전진
  //모터A 정회전

  analogWrite(A_1A, speed);

  analogWrite(A_1B, 0);

  //모터B 정회전

  analogWrite(B_1A, speed);

  analogWrite(B_1B, 0);

  delay(600); //400 -> 600


  if (id == 1) {              // turn left
    //모터A 역회전
    turnLeft();
  }
  else if (id == 3) { //우회전
    
    turnRight();
  }

  //모터A 정지

  analogWrite(A_1A, 0);

  analogWrite(A_1B, 0);

  //모터B 정지

  analogWrite(B_1A, 0);

  analogWrite(B_1B, 0);


}


void sCrossPoint(int id) {
  //약간 전진
  //모터A 정회전

  analogWrite(A_1A, speed);

  analogWrite(A_1B, 0);

  //모터B 정회전

  analogWrite(B_1A, speed);

  analogWrite(B_1B, 0);

  delay(600); //400->800 -> 600


  if (id == 1) {            //right turn

   turnRight();
  }
  else if (id == 3) {        // left turn
    turnLeft();

  }
  //모터A 정지

  analogWrite(A_1A, 0);

  analogWrite(A_1B, 0);

  //모터B 정지

  analogWrite(B_1A, 0);

  analogWrite(B_1B, 0);


}
void turnLeft(){
  analogWrite(A_1A, speed);

  analogWrite(A_1B, 0);

  //모터B 정회전

  analogWrite(B_1A, 0);

  analogWrite(B_1B, speed);

  delay(400);

  while (1) {
    if (digitalRead(rightLineSensor)) {     //오른쪽 센서 인식 할 때까지 좌회전
      analogWrite(A_1A, speed - 10);

      analogWrite(A_1B, 0);

      //모터B 정회전

      analogWrite(B_1A, 0);

      analogWrite(B_1B, speed - 10);
     
    }
    else {
      break;
    }
  }
  
}


void turnRight(){
  analogWrite(A_1A, 0);

  analogWrite(A_1B, speed);

  //모터B 정회전

  analogWrite(B_1A, speed);

  analogWrite(B_1B, 0);

  delay(800); //400->800

  while (1) {
    if (digitalRead(leftLineSensor)) {     //왼쪽 센서 인식 할 때까지 좌회전
      analogWrite(A_1A, 0);

      analogWrite(A_1B, speed - 10);

      //모터B 정회전

      analogWrite(B_1A, speed - 10);

      analogWrite(B_1B, 0);
     
    }
    else {
      break;
    }
  }
  
}

void rotate() {

  analogWrite(A_1A, speed);

  analogWrite(A_1B, 0);

  //모터B 정회전

  analogWrite(B_1A, 0);

  analogWrite(B_1B, speed);

  delay(800);

  while (1) {
    if (digitalRead(rightLineSensor)) {     //왼쪽 센서 인식 할 때까지 좌회전
      analogWrite(A_1A, speed - 10);

      analogWrite(A_1B, 0);

      //모터B 정회전

      analogWrite(B_1A, 0);

      analogWrite(B_1B, speed - 10);
     
    }
    else {
      break;
    }
  }

}

void sLift(int height) {
  //지게 꽂기
  //모터A,모터B 정회전

  analogWrite(A_1A, speed);

  analogWrite(A_1B, 0);

  analogWrite(B_1A, speed);

  analogWrite(B_1B, 0);

  delay(300); //400->800->400 

  analogWrite(A_1A, 0);
  analogWrite(B_1A, 0);

  if (height == 1) {
    //지게모터 약간 상승(물건 올리기)

    analogWrite(C_1A, speed+50);

    analogWrite(C_1B, 0);

    delay(3000); //7000->4000->3000

    analogWrite(C_1A, 0);
  }
  else if (height == 2) {
    //높이가 2 일 때 설정해주세요.
  }
  else {
    //높이가 3 일 때 설정해주세요.
  }


  // 지게차 빼기
  //모터A,모터B 역회전

  analogWrite(A_1A, 0);

  analogWrite(A_1B, speed);

  analogWrite(B_1A, 0);

  analogWrite(B_1B, speed);

  delay(500);

  if (height > 1) {

    if (height == 2) {
      // 높이가 2 일 때 올라간 만큼 지게를 내려주세요.
    }
    else {
      // 높이가 3 일 때 올라간 만큼 지게를 내려주세요.
    }
  }

  //멈추기
  analogWrite(A_1A, 0);

  analogWrite(A_1B, 0);

  analogWrite(B_1A, 0);

  analogWrite(B_1B, 0);

  delay(1000);
  rotate();
}


void eLift() {
 
  forkHeight();
  rotate();
  
  //모터 정지
  analogWrite(A_1A, 0);

  analogWrite(A_1B, 0);

  analogWrite(B_1A, 0);

  analogWrite(B_1B, 0);

}

void forkHeight(){

  
  if (fork_height == 1) {
  
    //전진하기 
  analogWrite(A_1A, speed);
  analogWrite(A_1B, 0);
  analogWrite(B_1A, speed);
  analogWrite(B_1B, 0);
  delay(400); //1500-> 500 -> 400 -> 500
  analogWrite(A_1A, 0);
  analogWrite(B_1A, 0);
  delay(2000);

  //지게차 내리기
  analogWrite(C_1A, 0);
  analogWrite(C_1B, speed+20);
  delay(2700); //1500->2000 -> 2500
  analogWrite(C_1B, 0);
  delay(1000);
  //후진하기
  analogWrite(A_1A, 0);
  analogWrite(A_1B, speed);
  analogWrite(B_1A, 0);
  analogWrite(B_1B, speed);
  delay(1500);  //1000 -> 2000 -> 1500
  analogWrite(A_1A, 0);
  analogWrite(A_1B, 0);
  analogWrite(B_1A, 0);
  analogWrite(B_1B, 0);
  fork_height=fork_height+1;
    
  }


  else { //지게차 2층 
    

  //전진하기
  analogWrite(A_1A, speed);
  analogWrite(A_1B, 0);
  analogWrite(B_1A, speed);
  analogWrite(B_1B, 0);
  delay(500);    // 500 -> 400
  analogWrite(A_1A, 0);
  analogWrite(B_1A, 0);
  delay(1500);
  
  //지게차 내리기
  analogWrite(C_1A, 0);
  analogWrite(C_1B, speed);
  delay(1800); //2층 지게 내리기
  analogWrite(C_1B, 0);
  delay(1000);
  //후진하기
  analogWrite(A_1A, 0);
  analogWrite(A_1B, speed);
  analogWrite(B_1A, 0);
  analogWrite(B_1B, speed);
  delay(1500); // 1000 -> 1500
  analogWrite(A_1A, 0);
  analogWrite(A_1B, 0);
  analogWrite(B_1A, 0);
  analogWrite(B_1B, 0);
  delay(2000);

    //지게모터 많이 하강(지게내리기)
    analogWrite(C_1A, 0);
    analogWrite(C_1B, speed);
    delay(13000); //2층 지게 원상복귀
    analogWrite(C_1B, 0);
    
    fork_height=1;
  }
} 
