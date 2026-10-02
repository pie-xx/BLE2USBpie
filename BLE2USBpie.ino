#include "Arduino.h"
#include "Wire.h"

#include "keyDefine.h"
#include "display.h"
#include "mouseProc.h"
#include "BLEproc.h"

extern enum ACTMODE actMode;

void setup()
{
    Serial.begin(115200);

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

void loop()
{
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
