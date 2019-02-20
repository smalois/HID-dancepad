// Written for teensy 3.2. Make sure to select Joystick from the "Tools > USB
// Type" menu. 
// 
// By default, analog pins 0 through (number_of_panels - 1) 
// will be used. Pin x will press joy button x + 1.
// Written for teensy 3.2
// By default, analog pins 0 through (number_of_panels - 1) 
// will be used. Pin x will press joy button x + 1.

#define ARDUINO_VOLTAGE 3300
#define number_of_panels 4

class Panel {
  bool state;         // 1/0 -> pressed/not
  int analog_pin;     // What pin are we using

  int cur_force;      // in a unit somewhat related to newtons
  int sensitivity;    // Button trigger tolerance based on force value. Setting
                      // these properly will likely take some
                      // trial/error based on your sensors and how they're
                      // mounted.

  int base_force;     // Sensor value after being zeroed out.
  int accompanying_resistor_value;  // in Kohms. Useful if you can't find 9
                                    // 10k resistors

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

  // Zero out every panel by setting the base sensor value
  // (Don't step on anything when pressing this button)
  void zero_out() {
    this->base_force = this->cur_force;
  }

  bool is_pressed() {
    int normalized_force = max(0, this->cur_force - this->base_force);
    return normalized_force > this->sensitivity; 
  }

  void listen_for_stomp() {
    this->cur_force = sensor_val_to_force(analogRead(this->analog_pin));

    if (this->is_pressed()){
      Joystick.button(analog_pin + 1, 1);
    } else {
      Joystick.button(analog_pin + 1, 0);
    }
  }

private:
  // Convert panel sensor value to a ""linear"" force value.
  unsigned long sensor_val_to_force(int sensor_val) {
    int voltage = map(sensor_val, 0, 1023, 0, ARDUINO_VOLTAGE);  
    unsigned long conductance = 1000000;
    unsigned long fsr_resistance = ARDUINO_VOLTAGE - voltage;
    fsr_resistance *= 10000;
    fsr_resistance /= voltage;
    conductance /= fsr_resistance;

    // Makes sensor resistance curve roughly more linear.
    return pow(conductance / 80, 1.5);
  }
};  

Panel dancepad[number_of_panels];
int sensitivities[9] = {80, 100, 60, 60, 100, 100, 100, 100, 100};
int resistors[9] = {10, 10, 10, 10, 10, 10, 10, 10, 10};
int zero_state;     // Button state for zeroing-out the panels


void setup() {
  Serial.begin(9600);
  pinMode(2, INPUT); // Button to zero-out the panels
  Joystick.useManualSend(true);
  for (int i = 0; i < number_of_panels; i++) {
    dancepad[i] = {sensitivities[i], resistors[i], i};
  }
}

void loop() {
  zero_state = digitalRead(2);

  // Check the state of the zero-out button
  if(zero_state == HIGH) {
    for (int i = 0; i < number_of_panels; i++) {
      dancepad[i].zero_out();
    }
  }

  for (int i = 0; i < number_of_panels; i++) {
    dancepad[i].listen_for_stomp();
  }
  
  Joystick.send_now();
}
