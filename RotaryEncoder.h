#ifndef _ROTARY_ENCODER
#define _ROTARY_ENCODER

#if (ARDUINO >= 100)
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif


//template <uint8_t msbPin, uint8_t lsbPin, uint8_t buttonPin>
class RotaryEncoder {
  private:
    uint8_t buttonPin_;
    // Encoder must use pins 2 and 3
    // MSB = most significant bit
    uint8_t msbPin_;
    // LSB = least significant bit
    uint8_t lsbPin_;

    unsigned int multiplier_;

    volatile long valPrev_;
    int maxVal_;
    int minVal_;
    byte state_;
    byte statePrev_;
    boolean isToggled_;


  public:
    volatile long val;
    byte isActive;

    // Constructor
    RotaryEncoder(uint8_t buttonPin = 12) {
      msbPin_ = 2;
      lsbPin_ = 3;
      buttonPin_ = buttonPin;

      // TODO(ken.frederick): If I set the initial val at 0, and then later
      // call setMinVal, it doesn't update the val...
      val = 60 * 4;
      valPrev_ = val;

      state_ = 1; // true
      statePrev_ = state_;
      isToggled_ = true;

      isActive = 0;
    }

    // Deconstructor
    ~RotaryEncoder() {
    }

    /**
     * Init/setup pins and encoder handler.
     */
    void init() {
      // Set default multiplier value.
      setMultiplier(4);

      // Set default minimum and maximum values.
      setMinVal(60);
      setMaxVal(240);

      // Setup pins for reading Rotary Encoder values.
      pinMode(msbPin_, INPUT);
      pinMode(lsbPin_, INPUT);
      digitalWrite(msbPin_, HIGH);
      digitalWrite(lsbPin_, HIGH);

      // Activate pins for Rotary Encoder button
      pinMode(buttonPin_, INPUT);
      digitalWrite(buttonPin_, HIGH);

      // Can't call attachInterrupt from within class :(
      // https://forum.arduino.cc/index.php?topic=480721.0
      // attachInterrupt(0, encoderHandler, CHANGE);
      // attachInterrupt(1, encoderHandler, CHANGE);
    }

    /**
     * Handler for encoder values.
     */
    void encoderHandler() {
      int MSB = digitalRead(msbPin_);
      int LSB = digitalRead(lsbPin_);

      // converting the 2 pin value to single number
      int encoded = (MSB << 1) | LSB;
      int sum = (valPrev_ << 2) | encoded;

      if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) {
        if (val < maxVal_) {
          val++;
        } else {
          val = maxVal_;
        }
      }
      if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
        if (val > minVal_) {
          val--;
        } else {
          val = minVal_;
        }
      }

      // store this value for next time
      valPrev_ = encoded;
    }

    /**
     * Handler for button click.
     */
    void clickHandler() {
      state_ = digitalRead(buttonPin_);

      if ((state_ != statePrev_) && (state_ == 1)) {
        if (isToggled_) {
          isActive = 1;
          isToggled_ = !isToggled_;
        } else {
          isActive = 0;
          isToggled_ = !isToggled_;
        }
      }

      // save the current state_ as the last state_, for next time through the loop
      statePrev_ = state_;
    }

    /**
     * Multiplier to improve accuracy of encoder value
     */
    void setMultiplier(int val) {
      multiplier_ = val;
    }

    /**
     * Maximum value for encoder.
     */
    void setMaxVal(int val) {
      maxVal_ = val * multiplier_;

      if (val >= maxVal_) {
        val = maxVal_;
        valPrev_ = val;
      }
    }

    /**
     * Minimum value for encoder.
     */
    void setMinVal(int val) {
      minVal_ = val * multiplier_;

      if (val <= minVal_) {
        val = minVal_;
        valPrev_ = val;
      }
    }

    /**
     *
     */
    volatile long getVal() {
      return (val / multiplier_);
    }
};



#endif
