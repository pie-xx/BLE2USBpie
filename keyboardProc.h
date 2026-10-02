#ifndef KEYBOARDPROC_H
#define KEYBOARDPROC_H
#include <Arduino.h>
#include "display.h"
////////////////////////////////////////////////////////////////////////////////////
// HID UUID
#include "USB.h"
#include "USBHIDKeyboard.h"
#include "USBHIDMouse.h"

struct KeyInfo {
    unsigned long accessTime;
    KeyReport   report;
};

struct KeyMkBrkInfo {
    unsigned long accessTime;
    uint8_t hidcode;
    bool  make;
};

// -------------------------------------------------------------
// LED受信機能を追加したカスタムキーボードクラス
// -------------------------------------------------------------
class CustomKeyboard : public USBHIDKeyboard {
public:
  typedef void (*LEDCallback)(uint8_t leds);
  
  void onLED(LEDCallback cb) {
    _ledCallback = cb;
  }

protected:
  // PCからHID出力レポート（LEDステータス等）が送信されてきた時に呼ばれる関数
  void _onOutput(uint8_t report_id, const uint8_t* buffer, uint16_t len) override {
    // 標準キーボードの出力レポート（1バイト: LED状態フラグ）を受信
    if (len > 0 && _ledCallback != NULL) {
      _ledCallback(buffer[0]);
    }
  }

private:
  LEDCallback _ledCallback = NULL;
};

void keyboardProcInit();
void restartKeyboard();
void keyboardProc( KeyReport report );
bool isKeyboardTimeout();
void keyboardProcTimeout();
#endif