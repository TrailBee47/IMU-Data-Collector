#include <BfButton.h>

const unsigned int btnPin = 0;
enum { UNPRESSED=0, SINGLE_PRESSED, DOUBLE_PRESSED, LONG_PRESSED };
int buttonState;
BfButton btn(BfButton::STANDALONE_DIGITAL, btnPin, true, LOW); //
void IRAM_ATTR buttonISR()
{
  btn.read();
}
void pressHandler (BfButton *btn, BfButton::press_pattern_t pattern) {
  Serial.print(btn->getID());
  switch (pattern) {
    case BfButton::SINGLE_PRESS:
      Serial.println(" pressed.");
      buttonState = SINGLE_PRESSED;
      break;
    case BfButton::DOUBLE_PRESS:
      Serial.println(" double pressed.");
      buttonState = DOUBLE_PRESSED;
      break;
    case BfButton::LONG_PRESS:
      Serial.println(" long pressed.");
      buttonState = LONG_PRESSED;
      break;
  }
}

void setupButton()
{
  btn.onPress(pressHandler)
     .onDoublePress(pressHandler) // default timeout
     .onPressFor(pressHandler, 2000); // custom timeout for 1 second
  attachInterrupt(btnPin, buttonISR, CHANGE );  
}

int buttonRead()
{
  int ret=buttonState;
  buttonState=UNPRESSED;
  return ret;
}

void buttonLoop()
{
  btn.read();
}
