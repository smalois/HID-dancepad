//#include <Keyboard.h>

// Represents a 9 panel dance pad. All 9-length data arrays represent panels in
// left-to-right, top-to-bottom order (normal english reading order). Simulates
// a keyboard input using the leftmost letter group of a standard keyboard
// (qweasdzxc)

typedef struct panel {
	char key_to_send; // What key this panel represents

	bool state; // 1/0 -> pressed/not
	int current_sensor_value; 
	int sensitivity; // Button trigger tolerance

	int base_sensor_value; // Sensor value after being zeroed out.
	int accompanying_resistor_value; // in Kohms
} panel;

bool panelstate[9]; // if the panel is pressed or not
int panel_sensitivities[9] =    {400, 400, 400, 400, 400, 400, 400, 400, 350};  // panel sensitivity val 0 - 1024
char characters[9] =  {'q', 'w', 'e', 'a', 's', 'd', 'z', 'x', 'c'}; // Characters to send on panel pressed
int zero_out_values[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0}; // Base sensor values (to cancel weight of the panels)
int resistor_value = 10; // Resistor value in the circuit (see arduino schematic)
int fsrReading[9]; // Raw reading from FSRs (analog sensors)

int zero_state;	// Button state for zeroing-out the panels
panel dancepad[9];

void setup() {
  Serial.begin(9600);
  //Keyboard.begin();
  pinMode(2, INPUT); // Button to zero-out the panels
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
    if (zeroed_out(normalize(fsrReading[i]), zero_out_values[i]) > panel_sensitivities[i] && panelstate[i] == 0) {
      panelstate[i] = 1;
      //Keyboard.press(characters[i]);
    }
    
    //Serial.print(zeroed_out(normalize(fsrReading[i]), zero_out_values[i]));
    //Serial.print(", ");
    
    if (zeroed_out(normalize(fsrReading[i]), zero_out_values[i]) < panel_sensitivities[i] && panelstate[i] == 1) {
      panelstate[i] = 0;
      //Keyboard.release(characters[i]);
    }
  }    
  
  //Serial.println();
  delay(1);  
}
