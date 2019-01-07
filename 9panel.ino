#include <Keyboard.h>

bool panelstate[9];
int zero_state;
char characters[9] =  {'q', 'w', 'e', 'a', 's', 'd', 'z', 'x', 'c'};
int panelsens[9] =    {400, 400, 400, 400, 400, 400,400, 400, 350};  // panel sensitivity val 0 - 1024
int zero_out_values[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
int resistor_value = 10; //kilo-ohms
int fsrReading[9];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(2, INPUT);
  Keyboard.begin();
}

int normalize(int raw_reading) {
  return pow((double)raw_reading/ 1024, resistor_value) * 1024;
}

int zeroed_out(int sensor_val, int zero_val) {
  int denominator = 1024-zero_val;
  if (denominator == 0) {
    denominator = 1;
  }
  return ((float)abs((sensor_val-zero_val)) / (float)denominator) * 1024;
}


void loop() {
  //fsrReading[0] = analogRead(1); 
  //fsrReading[1] = analogRead(2); 
  //fsrReading[2] = analogRead(3); 
  //fsrReading[3] = analogRead(4); 
  //fsrReading[4] = analogRead(5); 
  fsrReading[5] = analogRead(6); 
  fsrReading[6] = analogRead(7); 
  fsrReading[7] = analogRead(8);
  fsrReading[8] = analogRead(9);
  zero_state = digitalRead(2);

  if(zero_state == HIGH) {
    for (int i = 0; i < 9; i++) {
      zero_out_values[i] = normalize(fsrReading[i]);
    }
  }

  for (int i = 0; i < 9; i++) {
    if (zeroed_out(normalize(fsrReading[i]), zero_out_values[i]) > panelsens[i] && panelstate[i] == 0) {
      panelstate[i] = 1;
      Keyboard.press(characters[i]);
    }
    
    //Serial.print(zeroed_out(normalize(fsrReading[i]), zero_out_values[i]));
    //Serial.print(", ");
    
    if (zeroed_out(normalize(fsrReading[i]), zero_out_values[i]) < panelsens[i] && panelstate[i] == 1) {
      panelstate[i] = 0;
      Keyboard.release(characters[i]);
    }
  }    
  
  //Serial.println();
  //delay(100);
}
