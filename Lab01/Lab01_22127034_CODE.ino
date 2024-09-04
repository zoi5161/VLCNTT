int ledGreen = 2;
int ledYellow = 3;
int ledRed = 4;
int ledOrange = 5;
int ledWhite = 6;
int ledBlue = 7;
int button = 8;
int ledIndex = 0;

unsigned long previousMillis = 0; // Thời gian cuối cùng LED được cập nhật
const long interval = 1000; // Khoảng thời gian giữa các lần bật tắt LED (1000 milliseconds = 1 giây)
bool mode = false; // false cho Mode 1, true cho Mode 2
unsigned long buttonPressedTime = 0; // Thời gian khi nút được nhấn
bool buttonState = false; // Trạng thái hiện tại của nút
bool lastButtonState = false; // Trạng thái trước đó của nút

void setAllLeds(bool state){
  digitalWrite(ledGreen, state);
  digitalWrite(ledYellow, state);
  digitalWrite(ledRed, state);
  digitalWrite(ledOrange, state);
  digitalWrite(ledWhite, state);
  digitalWrite(ledBlue, state);
}

void turnOffAllLeds(){
  setAllLeds(false);
}

void turnOnLedByIndex(int index){
  switch (index) {
    case 0: digitalWrite(ledGreen, HIGH); break;
    case 1: digitalWrite(ledYellow, HIGH); break;
    case 2: digitalWrite(ledRed, HIGH); break;
    case 3: digitalWrite(ledOrange, HIGH); break;
    case 4: digitalWrite(ledWhite, HIGH); break;
    case 5: digitalWrite(ledBlue, HIGH); break;
  }
}

void setup(){
  pinMode(ledGreen, OUTPUT);
  pinMode(ledYellow, OUTPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(ledOrange, OUTPUT);
  pinMode(ledWhite, OUTPUT);
  pinMode(ledBlue, OUTPUT);
  pinMode(button, INPUT);
  Serial.begin(9600);
  setAllLeds(true); // Khởi động ở Mode 1
}

void loop(){
  unsigned long currentMillis = millis();
  buttonState = digitalRead(button);

  if(buttonState != lastButtonState){
    if(buttonState == HIGH)
      buttonPressedTime = currentMillis;
    else{
      if((currentMillis - buttonPressedTime) >= 5000){
        if(mode == false)
          ledIndex = 0;
        mode = true; // Chuyển sang Mode 2 nếu giữ nút >= 5 giây
        previousMillis = 0; // Reset thời gian cho Mode 2
      } 
      else if(mode) // Chuyển lại Mode 1 nếu nhấn nút < 5 giây khi đang ở Mode 2
          mode = false;
    }
  }
  lastButtonState = buttonState;

  if(mode){ // Mode 2: Bật tắt LED luân phiên
    if(currentMillis - previousMillis >= interval){
      previousMillis = currentMillis;
      turnOffAllLeds();
      turnOnLedByIndex(ledIndex);
      ledIndex = (ledIndex + 1) % 6; // Chuyển sang LED tiếp theo
    }
  } 
  else // Mode 1: Tất cả các LED đều bật
    setAllLeds(true);
}
