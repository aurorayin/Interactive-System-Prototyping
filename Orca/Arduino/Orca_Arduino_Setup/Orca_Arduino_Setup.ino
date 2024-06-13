// set pin numbers for switch, joystick axes, and LED:
const int xAxis = A0;       // joystick X axis
const int yAxis = A1;       // joystick Y axis
const int joystick_button = 6;
const int button1 = 2;
const int button2 = 3;
const int button3 = 4;
const int button4 = 5;
const int button5 =>? 7;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(joystick_button, INPUT_PULLUP);
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(button3, INPUT_PULLUP);
  pinMode(button4, INPUT_PULLUP);
  pinMode(button5, INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:

  // read and scale the two axes:
  int xValue = analogRead(xAxis);
  int yValue = analogRead(yAxis);
  int buttonValue = digitalRead(joystick_button);
  int button1Val = digitalRead(button1);
  int button2Val = digitalRead(button2);
  int button3Val = digitalRead(button3);
  int button4Val = digitalRead(button4);
  int button5Val = digitalRead(button5);

  Serial.print(xValue);
  Serial.print(",");
  Serial.print(yValue);
  Serial.print(",");
  Serial.print(buttonValue);
  Serial.print(",");
  Serial.print(button1Val);
  Serial.print(",");
  Serial.print(button2Val);
  Serial.print(",");
  Serial.print(button3Val);
  Serial.print(",");
  Serial.print(button4Val);
  Serial.print(",");
  Serial.println(button5Val);

  delay(100);
}
