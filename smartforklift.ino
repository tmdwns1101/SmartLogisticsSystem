#include <SoftwareSerial.h>
#define rxPin 2 
#define txPin 3 
SoftwareSerial SwSerial(txPin, rxPin); 
int val = 0;

void setup() {
 Serial.begin(9600); 
 SwSerial.begin(9600); 
 Serial.println("START");
 


}



void loop() {
  
  SwSerial.println("Ready");
  Serial.println("I'm Ready!");
  String input = "";
  bool flag = true;
  delay(1000);
  long startTime = millis();
  
  while(1){
    long endTime = millis();
    
    if(endTime - startTime > 10000){
      flag = false;
      break;
    }
    if(SwSerial.available()){
     
    input = SwSerial.readStringUntil('\n');
    Serial.println(input);
    if(input.equals("end")){   // 프로그램 종료 후 대기 
      while(1){
         if(SwSerial.available()){
          String re = SwSerial.readStringUntil('\n');
          if(re.equals("Init"))
            break;
         }
      }
    }else if(input.equals("Init")){  //초기화 부분
      break;
    }
    else{   //메인 하드웨어 작동 부분 
      int sliceIdx = input.indexOf('/');
      String id = input.substring(0,sliceIdx);
      String quantity = input.substring(sliceIdx + 1,input.length());
     
      Serial.println(sliceIdx);
      Serial.print("ID : ");
      Serial.println(id);
      Serial.print("Quantity : ");
      Serial.println(quantity);
      Serial.println("10 초 대기");
      delay(10000); //이 자리에 메인 하드웨어 구동 함수를 넣는다. (목적지 가서 물건을 갖고 다시 물건을 내려 놓는 과정)
      break;
      }
    }
  }
  if(flag == false) {
    Serial.println("No input");
  }
  
}
