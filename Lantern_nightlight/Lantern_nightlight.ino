enum MODE{
  FADE,
  POTENTIOMETER,
  PULL,
  NUM_MODES
};

enum RGB{
  RED,
  GREEN,
  BLUE,
  NUM_COLORS
};

const bool COMMON_ANODE = false;
const int BUTTON_SELECT_MODE_PIN = 2;
// const int LED_PIN = 11;

const int PHOTOCELL_INPUT_PIN = A5;
const int MIN_PHOTOCELL_VAL = 100; 
// max darkness level (LED should be fully on for anything 800+)
const int MAX_PHOTOCELL_VAL = 800; 
const boolean PHOTOCELL_IS_R2_IN_VOLTAGE_DIVIDER = true; // set false if photocell is R1
const int SOUND_SENSOR_DIGITAL_PIN = 7;
const int SOUND_SENSOR__ANALOG_PIN = A1;

const int RGB_RED_PIN = 6;
const int RGB_GREEN_PIN  = 5;
const int RGB_BLUE_PIN  = 3;
const int DELAY_MS = 20; // delay in ms between changing colors
const int MAX_COLOR_VALUE = 255;

int _rgbLedValues[] = {255, 0, 0}; // Red, Green, Blue
enum RGB _curFadingUpColor = GREEN;
enum RGB _curFadingDownColor = RED;
const int FADE_STEP = 5;  

const int POTENTIOMETER_PIN = A0;
const int MIN_ANALOG_INPUT = 0;
const int MAX_ANALOG_INPUT = 1023;
enum MODE _mode = FADE;

// const int MAX_LED_VAL = 255;
const int MIN_LED_VAL = 0;
int _fadeVal = 0;
int _fadeStep = 5;
int _prevButtonVal = HIGH; // pull-up resistor config
bool is_on = false;

void setup()
{
  // Turn on serial for debugging
  Serial.begin(9600); 
  
  // Setup pin modes
  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_GREEN_PIN, OUTPUT);
  pinMode(RGB_BLUE_PIN, OUTPUT);
  
  pinMode(BUTTON_SELECT_MODE_PIN, INPUT_PULLUP);
  pinMode(POTENTIOMETER_PIN, INPUT);
  pinMode(PHOTOCELL_INPUT_PIN, INPUT);
  pinMode(SOUND_SENSOR_DIGITAL_PIN, INPUT);
  // pinMode(LED_PIN, OUTPUT);
  printMode();
  
  Serial.println("Red, Green, Blue");

  // Set initial color
  setColor(_rgbLedValues[RED], _rgbLedValues[GREEN], _rgbLedValues[BLUE], 255);
  delay(DELAY_MS);
}

void loop()
{
  // Check mode select button state and increment
  // mode accordingly
  int buttonVal = digitalRead(BUTTON_SELECT_MODE_PIN);
  
  // Turn on built-in LED when button pressed 
  // to give feedback to user
  digitalWrite(LED_BUILTIN, !buttonVal);
  
  // Button is LOW when pressed
  if(buttonVal == LOW && buttonVal != _prevButtonVal){ 
    
    // Increment the modes
  	_mode = (MODE)(_mode + 1);
    if(_mode >= NUM_MODES){
      _mode = FADE; 
    }
    printMode();
    
    delay(50); // simple debounce
  }
  
  // Now set the LED according to the mode
  if(_mode == FADE){ // Fade mode
  	fadeMode();
  }else if(_mode == POTENTIOMETER){ // Potentiometer mode
    int potVal = analogRead(POTENTIOMETER_PIN);
    int ledVal = map(potVal, MIN_ANALOG_INPUT, MAX_ANALOG_INPUT, 255, 0);
    setColor(_rgbLedValues[RED], _rgbLedValues[GREEN], _rgbLedValues[BLUE], ledVal);
  } else if (_mode == PULL) {
  	pullMode();
  }
  
  _prevButtonVal = buttonVal;
  delay(10);
}



void printMode(){
  Serial.print("Now in mode: ");
  //Serial.print(_mode);
  switch(_mode){
  	case FADE:
      Serial.print("FADE automatically fades the LED with step size: ");
      Serial.println(_fadeStep);
      break;
    case POTENTIOMETER:
      Serial.print("POTENTIOMETER sets the LED based on the pot value: ");
      Serial.println(analogRead(POTENTIOMETER_PIN));
      break;
    case PULL:
      Serial.print("PULL mode on");
      break;
    default:
      Serial.println("In an unknown mode, we should not be here! :)");
      break;
  }
}

void pullMode()
{ 
  int val = digitalRead(SOUND_SENSOR_DIGITAL_PIN);
  Serial.println(val);
  if (val == 1) {
    if (is_on == true) {
      setColor(0, 0, 0, 0);
      is_on = false;
    } else {
      setColor(255, 20, 0, random(120)+135);
      is_on = true;
    }
  }
}

void fadeMode()
{	
  int photocellVal = analogRead(PHOTOCELL_INPUT_PIN);
  Serial.print("photocell: ");
  Serial.println(photocellVal);
  int brightnessVal = map(photocellVal, MIN_PHOTOCELL_VAL, MAX_PHOTOCELL_VAL, 0, 255);
  brightnessVal = constrain(brightnessVal, 0, 255);
  if(PHOTOCELL_IS_R2_IN_VOLTAGE_DIVIDER == false){
    // We need to invert the LED (it should be brighter when environment is darker)
    // This assumes the photocell is Rtop in the voltage divider
    brightnessVal = 255 - brightnessVal;
  }
  Serial.print(photocellVal);
  Serial.print(",");
  Serial.println(brightnessVal);
  
  // Increment and decrement the RGB LED values for the current
  // fade up color and the current fade down color
  _rgbLedValues[_curFadingUpColor] += FADE_STEP;
  _rgbLedValues[_curFadingDownColor] -= FADE_STEP;

  // Check to see if we've reached our maximum color value for fading up
  // If so, go to the next fade up color (we go from RED to GREEN to BLUE
  // as specified by the RGB enum)
  // This fade code partially based on: https://gist.github.com/jamesotron/766994
  if(_rgbLedValues[_curFadingUpColor] > MAX_COLOR_VALUE){
    _rgbLedValues[_curFadingUpColor] = MAX_COLOR_VALUE;
    _curFadingUpColor = (RGB)((int)_curFadingUpColor + 1);

    if(_curFadingUpColor > (int)BLUE){
      _curFadingUpColor = RED;
    }
  }

  // Check to see if the current LED we are fading down has gotten to zero
  // If so, select the next LED to start fading down (again, we go from RED to 
  // GREEN to BLUE as specified by the RGB enum)
  if(_rgbLedValues[_curFadingDownColor] < 0){
    _rgbLedValues[_curFadingDownColor] = 0;
    _curFadingDownColor = (RGB)((int)_curFadingDownColor + 1);

    if(_curFadingDownColor > (int)BLUE){
      _curFadingDownColor = RED;
    }
  }

  // Set the color and then delay
  setColor(_rgbLedValues[RED], _rgbLedValues[GREEN], _rgbLedValues[BLUE], brightnessVal);
  delay(DELAY_MS);
}

/**
 * setColor takes in values between 0 - 255 for the amount of red, green, and blue, respectively
 * where 255 is the maximum amount of that color and 0 is none of that color. You can illuminate
 * all colors by intermixing different combinations of red, green, and blue
 * 
 * This function is based on https://gist.github.com/jamesotron/766994
 */
void setColor(int red, int green, int blue, int brightnessVal)
{
  // If a common anode LED, invert values
  if(COMMON_ANODE == true){
    red = MAX_COLOR_VALUE - red;
    green = MAX_COLOR_VALUE - green;
    blue = MAX_COLOR_VALUE - blue;
  }
  analogWrite(RGB_RED_PIN, red *brightnessVal/MAX_COLOR_VALUE);
  analogWrite(RGB_GREEN_PIN, green*brightnessVal/MAX_COLOR_VALUE);
  analogWrite(RGB_BLUE_PIN, blue*brightnessVal/MAX_COLOR_VALUE);  
}