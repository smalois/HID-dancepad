#include <Keyboard.h>

#define ARDUINO_VOLTAGE 3300

// Represents a 9 panel dance pad. All 9-length data arrays represent panels in
// left-to-right, top-to-bottom order (normal english reading order). Simulates
// a keyboard input using the leftmost letter group of a standard keyboard
// (qweasdzxc)

typedef struct panel {
	bool state; 		// 1/0 -> pressed/not

	int cur_force;   	// in a unit somewhat related to newtons
	int sensitivity; 	// Button trigger tolerance based on force value. Setting
						// these properly will likely take some
						// trial/error based on your sensors and how they're
						// mounted.

	int base_force;  	// Sensor value after being zeroed out.
	char key_to_send; 	// What key this panel represents
	int accompanying_resistor_value; // in Kohms. Useful if you can't find 9
									 // 10k resistors.
} panel;

panel dancepad[9];
char keys[9] = {'q', 'w', 'e', 'a', 's', 'd', 'z', 'x', 'c'};
int sensitivities[9] = {10, 80, 10, 100, 10, 60, 10, 60, 10}; //x, L, x, R, x, D, x, U, x
int resistors[9] = {10, 10, 10, 10, 10, 10, 10, 10, 10};
int zero_state; 	// Button state for zeroing-out the panels


void setup() {
	Serial.begin(9600);
	pinMode(2, INPUT); // Button to zero-out the panels
	Keyboard.begin();
	// Initialize all of the panel values
	for (int i = 0; i < 9; i++) {
		dancepad[i].key_to_send = keys[i];
		dancepad[i].sensitivity = sensitivities[i];
		dancepad[i].accompanying_resistor_value = resistors[i];
	}
}

// Convert panel sensor value to a ""linear"" force value.
unsigned long ptof(int sensor_val) {
	int voltage = map(sensor_val, 0, 1023, 0, ARDUINO_VOLTAGE);  
	unsigned long conductance = 1000000;
	unsigned long fsr_resistance = ARDUINO_VOLTAGE - voltage;
	fsr_resistance *= 10000;
	fsr_resistance /= voltage;
	conductance /= fsr_resistance;

	// Makes sensor resistance curve roughly more linear.
	return pow(conductance / 80, 1.5);
}


bool is_pressed(struct panel cur_panel) {
     int normalized_force = max(0, cur_panel.cur_force - cur_panel.base_force);
     return normalized_force > cur_panel.sensitivity; 
}

void loop() {
	dancepad[1].cur_force = ptof(analogRead(6));
	dancepad[3].cur_force = ptof(analogRead(7));
	dancepad[5].cur_force = ptof(analogRead(8));
	dancepad[7].cur_force = ptof(analogRead(9));
	zero_state = digitalRead(2);

    // Check the state of the zero-out button
	if(zero_state == HIGH) {
		// Zero out every panel by setting the base sensor value
		// (Don't step on anything when pressing this button)
		for (int i = 0; i < 9; i++) {
			dancepad[i].base_force = dancepad[i].cur_force;
		}
	}

    // Read the state of each panel
    for (int i = 0; i < 9; i++) {
		if (is_pressed(dancepad[i])) {
		dancepad[i].state = 1;
			Keyboard.press(dancepad[i].key_to_send);
		}

		// Print statements are useful for determining initial sensitivities:
		//Serial.print(max(0, dancepad[i].cur_force - dancepad[i].base_force));
		//Serial.print(", ");

		if (!is_pressed(dancepad[i])) {
		dancepad[i].state = 0;
			Keyboard.release(dancepad[i].key_to_send);
		}
    }    
  
    //Serial.println();
    //delay(1);  
}
