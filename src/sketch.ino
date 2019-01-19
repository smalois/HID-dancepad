//#include <Keyboard.h>

// Represents a 9 panel dance pad. All 9-length data arrays represent panels in
// left-to-right, top-to-bottom order (normal english reading order). Simulates
// a keyboard input using the leftmost letter group of a standard keyboard
// (qweasdzxc)

typedef struct panel {
	char key_to_send; // What key this panel represents

	bool state; // 1/0 -> pressed/not
	int sensor_value; 
	int sensitivity; // Button trigger tolerance

	int base_sensor_value; // Sensor value after being zeroed out.
	int accompanying_resistor_value; // in Kohms
} panel;

int zero_state;	// Button state for zeroing-out the panels
panel dancepad[9];

void setup() {
  Serial.begin(9600);
  //Keyboard.begin();
  pinMode(2, INPUT); // Button to zero-out the panels
}

int normalize(int raw_reading) {
  return pow((double)raw_reading/ 1024, 10) * 1024;
}

// panel sensor value to force. 
unsigned long ptof(struct panel sensor) {
	unsigned long conductance = 1000000;
	int voltage = map(sensor.sensor_value, 0, 1023, 0, 5000);
	unsigned long fsr_resistance = 5000 - voltage;
	fsr_resistance *= sensor.accompanying_resistor_value;;
	fsr_resistance /= voltage;
	conductance /= fsr_resistance;
	
	if (conductance <= 1000) {
		return conductance / 80;
	} 
	return (conductance - 1000) / 30;
}

int zeroed_out(int sensor_val, int zero_val) {
  int denominator = 1024-zero_val;
  if (denominator == 0) {
    denominator = 1;
  }
  return ((float)abs((sensor_val-zero_val)) / (float)denominator) * 1024;
}

int is_pressed(struct panel cur_panel) {
	int voltage = map(cur_panel.sensor_value, 0, 1023, 0, 5000);
	return cur_panel.sensor_value < cur_panel.sensitivity; 
}

void loop() {
	dancepad[1].sensor_value = analogRead(6);
	dancepad[3].sensor_value = analogRead(7);
	dancepad[5].sensor_value = analogRead(8);
	dancepad[7].sensor_value = analogRead(9);
  	zero_state = digitalRead(2);

	// Check the state of the zero-out button
  	if(zero_state == HIGH) {
		// Zero out every panel by setting the base sensor value
		// (Don't step on anything when pressing this button)
    	for (int i = 0; i < 9; i++) {
      		dancepad[i].base_sensor_value = normalize(dancepad[i].sensor_value);
    	}
  	}

	// Read the state of each panel
  	for (int i = 0; i < 9; i++) {
		if (is_pressed(dancepad[i])) {
			dancepad[i].state = 1;
			//Keyboard.press(characters[i]);
		}
		
		//Serial.print(zeroed_out(normalize(fsrReading[i]), zero_out_values[i]));
		//Serial.print(", ");
		
		if (!is_pressed(dancepad[i])) {
			dancepad[i].state = 0;
			//Keyboard.release(characters[i]);
		}
	}    
  
  	//Serial.println();
  	delay(1);  
}
