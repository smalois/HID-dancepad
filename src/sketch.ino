//#include <Keyboard.h>

// Represents a 9 panel dance pad. All 9-length data arrays represent panels in
// left-to-right, top-to-bottom order (normal english reading order). Simulates
// a keyboard input using the leftmost letter group of a standard keyboard
// (qweasdzxc)

typedef struct panel {
  bool state; // 1/0 -> pressed/not

  int cur_force; 
  int sensitivity; // Button trigger tolerance as a 
           // percentage of max sensor capacity

  int base_force; // Sensor value after being zeroed out.
  char key_to_send; // What key this panel represents
  int accompanying_resistor_value; // in Kohms
} panel;

int zero_state; // Button state for zeroing-out the panels
panel dancepad[9];

void setup() {
  Serial.begin(9600);
  pinMode(2, INPUT); // Button to zero-out the panels
  //bashKeyboard.begin();
  dancepad[0].key_to_send = 'q';
  dancepad[1].key_to_send = 'w';
  dancepad[2].key_to_send = 'e';
  dancepad[3].key_to_send = 'a';
  dancepad[4].key_to_send = 's';
  dancepad[5].key_to_send = 'd';
  dancepad[6].key_to_send = 'z';
  dancepad[7].key_to_send = 'x';
  dancepad[8].key_to_send = 'c';
}

// panel sensor value to force. 
unsigned long ptof(int sensor_val) {
  unsigned long conductance = 1000000;
  int voltage = map(sensor_val, 0, 1023, 0, 5000);
  unsigned long fsr_resistance = 5000 - voltage;
  fsr_resistance *= 10000;
  fsr_resistance /= voltage;
  conductance /= fsr_resistance;
  
  if (conductance <= 1000) {
    return conductance / 80;
  } 
  return (conductance - 1000) / 30;
}

int zeroed_out(int cur_force, int zero_val) {
  int denominator = ptof(1024) - zero_val;
  if (denominator == 0) {
    denominator = 1;
  }
  return ((float)abs((cur_force - zero_val)) / (float)denominator) *
ptof(1024);
}

bool is_pressed(struct panel cur_panel) {
  unsigned long denominator = ptof(500) - cur_panel.base_force;
  unsigned long nominator = cur_panel.cur_force - cur_panel.base_force;
  double percentage_pressed = (double)nominator / (double)denominator;
  Serial.println(percentage_pressed);
  return percentage_pressed > cur_panel.sensitivity; 
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
      //Keyboard.press(dancepad[i].key_to_send);
    }
    
    //Serial.print(dancepad[i].cur_force);
    //Serial.print(", ");
    
    if (!is_pressed(dancepad[i])) {
      dancepad[i].state = 0;
      //Keyboard.release(dancepad[i].key_to_send);
    }
  }    
  
    //Serial.println();
    delay(1);  
}

