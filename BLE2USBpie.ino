#include "Arduino.h"
#include "Wire.h"

#include "keyDefine.h"
#include "display.h"
#include "mouseProc.h"
#include "BLEproc.h"

extern enum ACTMODE actMode;
QueueHandle_t bleKeyQueue;

extern CustomKeyboard Keyboard;

void setup()
{
    Serial.begin(115200);
    bleKeyQueue = xQueueCreate(16, sizeof(KeyReport));

    displayInit();
    mouseProcInit();
    keyboardProcInit();

    USB.begin();

    // -----------------------------------
    // BLE
    // -----------------------------------
    BLEprocInit();
  debugPrint("BLEUSBpie --------------------------------------");
}

void processKeyReport(KeyReport report){
  
  bool isMousekey = false;
  bool isKana = false;
  bool isEng = false;
  for (size_t i = 0; i < 6 ; i++) {
      if( report.keys[i]==HID_PRTSC){
        isMousekey = true;
      }
      if( report.keys[i]==HID_CAPS){
        isEng = true;
      }
      if( report.keys[i]==HID_KANA){
        isKana = true;
      }
  }

  if(isMousekey){
    if( actMode == AM_KEY ){
        actMode = AM_MOUSE;
        debugPrint("AM_MOUSE ---------------------------------");
        restartMouse();
    }else{
        actMode = AM_KEY;
        debugPrint("AM_KEY   ---------------------------------");
    }
    return;
  }

  if(isKana){
    actMode = AM_KEY_KANA;
  }
  if(isEng){
    actMode = AM_KEY;
  }

  switch( actMode ){
  case AM_KEY:
      dumpKeyReport(report.keys, 6, "AM_KEY ");
    Keyboard.sendReport(&report);
    break;
  case AM_KEY_KANA:
    if(report.modifiers!=0){
      dumpKeyReport(report.keys, 6, "AM_KEY_KANA ");
        Keyboard.sendReport(&report);
    }else{
        keyboardProc(report);
    }
    break;
  case AM_MOUSE:
      if(mouseProc(report)){
          for( int i= 0; i<6; ++i){
              report.keys[i] = 0;
          }
      }
      dumpKeyReport(report.keys, 6, "AM_MOUSE ");
      Keyboard.sendReport(&report);
  }

}

void loop()
{
    KeyReport report;

    while (xQueueReceive(bleKeyQueue, &report, 0) == pdTRUE) {
        processKeyReport(report);
    }

    switch (actMode) {
    case AM_DO_CONNECT:
        dispDoConnectStat();
        actMode = AM_BOOT_SCAN;
        connectKeyboard();
        break;
    case AM_BOOT_SCAN:
        dispBootScanStat();
        break;
    case AM_DISCONNECT_SCAN:
        dispDisconnectStat();
        break;
    case AM_MOUSE:
        if( isMouseTimeout()  ){
            mouseProcTimeout();
        }
        dispAtMouseStat();
        break;
    case AM_KEY:
        dispAtKeyStat();
        break;
    case AM_KEY_KANA:
        if( isKeyboardTimeout()  ){
            keyboardProcTimeout();
        }
        dispAtKeyKanaStat();
    }
}
