// Written for teensy 3.2

//** ---------- USER CONFIGURABLE VALUES ---------- **//

// The number of panels on your pad. ITG = 4, PIU = 5, RH = 9, etc...
// By default, analog pins 0 through (NUMBER_OF_PANELS - 1) 
// will be used. Pin x will press joy button x + 1.
#define NUMBER_OF_PANELS 9

// The sensitivity value needed to release a pressed panel is 
// "FORCE_RELEASE_WINDOW" less than the panel's sensitivity. 
// Prevents jittery inputs for analog signals (without debounce delays).
#define FORCE_RELEASE_WINDOW 10

// Value should match your board's Vin. Teensy is 3300mv by default.
#define ARDUINO_VOLTAGE 3300

// The sensitivity of each panel. Sensitivity is a force value that 
// the sensor must exceed to trigger the panel. The number of values 
// between the curly brackets should match the NUMBER_OF_PANELS. These 
// values will need to be set through some trial and error, depending 
// on your hardware. The order of the values will also depend on how
// the inputs are wired to the panels.

// d,l,u,r,ur,c,ul,dl,dr on my hardware
int sensitivities[NUMBER_OF_PANELS] = {80, 240, 75, 100, 95, 180, 90, 130,
160};

// Stores the values of the accompanying resistors. Because stomping on FSRs
// introduces
// a lot of force, using lower value resistors can improve the resolution of
// the sensor
// and lead to better results.
int resistors[NUMBER_OF_PANELS] = {505, 675, 675, 1000, 1000, 1000, 1000, 1000,
1000};

//** ---------- END USER CONFIGURABLE VALUES ---------- **//



class Panel {
  int pressed; // 1/0 pressed / not pressed
  int analog_pin;     // What pin are we using  
  int accompanying_resistor_value;  // ohms

  // Force values
  int cur_force;      // in a unit somewhat related to newtons
  int sensitivity;    // Button trigger tolerance based on force value
  int base_force;     // Sensor value after being zeroed out

  public:
    Panel(int sensitivity, int resistor_value, int pin) {
      this->sensitivity = sensitivity;
      this->accompanying_resistor_value = resistor_value;
      this->analog_pin = pin;
    }
    
    Panel() {
      this->sensitivity = 100;
      this->accompanying_resistor_value = 10;
      this->analog_pin = 0;
    }
  
    // Tare every panel by setting the base sensor value. This
    // negates the force of brackets/panels and makes it much
    // easier to set meaningful force values.
    // (Don't step on anything when pressing this button)
    void zero_out() {
      this->base_force = this->cur_force;
    }
  
    bool is_pressed() {
      int normalized_force = max(0, this->cur_force - this->base_force);
      //Serial.print(normalized_force);
      //Serial.print(", ");
      return normalized_force > this->sensitivity; 
    }
  
    void listen_for_stomp() {
      this->cur_force = sensor_val_to_force(analogRead(this->analog_pin));
      
      if (this->is_pressed()){
        Joystick.button(analog_pin + 1, 1);

        // "debounce" by changing the sensitivity to a lower value once in the
        // pressed state.
        if (!pressed) {
          this->sensitivity = this->sensitivity - FORCE_RELEASE_WINDOW; // This
only runs once during the transition to a pressed state
          pressed = 1;
        }
      } else { // is released.
        Joystick.button(analog_pin + 1, 0);

        // Return to original sensitivity after "debounce" changed the release
        // sensitivity.
        if (pressed) {
          this->sensitivity = this->sensitivity + FORCE_RELEASE_WINDOW; // This
only runs once during the transition out of a pressed state
          pressed = 0;
        }
      }
    }
  
  private:
    // Convert panel sensor value to a normalized ""linear"" force value. 
    // This is necessary because of the ability to "zero-out" the panels 
    // without changing the performance characteristics of the panel.
    unsigned long sensor_val_to_force(int sensor_val) {
      int voltage = map(sensor_val, 0, 1023, 0, ARDUINO_VOLTAGE);  
      unsigned long conductance = 1000000;
      unsigned long fsr_resistance = ARDUINO_VOLTAGE - voltage;
      fsr_resistance *= this->accompanying_resistor_value;
      fsr_resistance /= voltage;
      conductance /= fsr_resistance;
  
      // Makes sensor resistance curve roughly more linear.
      return pow(conductance / 80, 1.5);
    }
};  

Panel dancepad[NUMBER_OF_PANELS];
int zero_state;     // Button state for zeroing-out the panels

void setup() {
  //Serial.begin(9600);
  pinMode(2, INPUT); // Button to zero-out the panels
  Joystick.useManualSend(true);
  
  for (int i = 0; i < NUMBER_OF_PANELS; i++) {
    dancepad[i] = {sensitivities[i], resistors[i], i};
  }
}

void loop() {
  zero_state = digitalRead(2);
  // Check the state of the zero-out button
  if(zero_state == HIGH) {
    for (int i = 0; i < NUMBER_OF_PANELS; i++) {
      dancepad[i].zero_out();
    }
  }

  for (int i = 0; i < NUMBER_OF_PANELS; i++) {
    dancepad[i].listen_for_stomp();
  }
  //Serial.println();
  
  Joystick.send_now();
}
