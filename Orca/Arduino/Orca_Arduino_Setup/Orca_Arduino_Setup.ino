// set pin numbers for switch, joystick axes, and LED:
const int xAxis = A0;       // joystick X axis
const int yAxis = A1;       // joystick Y axis
const int button = 6;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(button, INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:

  // read and scale the two axes:
  int xValue = analogRead(xAxis);
  int yValue = analogRead(yAxis);
  int buttonValue = digitalRead(button);

  Serial.print(xValue);
  Serial.print(",");
  Serial.print(yValue);
  Serial.print(",");
  Serial.println(buttonValue);

  delay(100);
}
