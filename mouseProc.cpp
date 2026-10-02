#include "mouseProc.h"
#include "keyDefine.h"

USBHIDMouse Mouse;

int mouseRepeatCount = 0;
bool oldClickL = false;
bool oldClickM = false;
bool oldClickR = false;

void mouseProcInit(){
  Mouse.begin();
}

void restartMouse(){
    oldClickL = false;
    oldClickM = false;
    oldClickR = false;
    lastKeyInfo.accessTime = millis();
    lastKeyInfo.report.modifiers = 0x00;
    lastKeyInfo.report.reserved = 0x00;
    for( int i=0; i < 6; ++i){
        lastKeyInfo.report.keys[i] = 0x00;
    }
}

bool isMouseTimeout(){
  return (millis() - lastKeyInfo.accessTime) > 100 ;
}

bool mouseProc( KeyReport report ){
    int dX = 0;
    int dY = 0;
    int dW = 0;
    bool clickL = false;
    bool clickM = false;
    bool clickR = false;
    bool doMouse = true;
    for (size_t i = 0; i < 6 ; i++) {
        switch( report.keys[i] ){
        case HID_LARW:  dX = dX - 1;     break;
        case HID_DARW:  dY = dY + 1;     break;
        case HID_UARW:  dY = dY - 1;     break;
        case HID_RARW:  dX = dX + 1;     break;
        case HID_NTRN:  clickL = true;    break;
        case HID_SP:  clickM = true;    break;
        case HID_TRNS:  clickR = true;    break;
        case HID_V:  dW = 1;    break;
        case HID_B:  dW = -1;    break;
        default:  doMouse = false;
        }
    }
    if( dX != 0 || dY != 0){
        int bias = 1;
        if( mouseRepeatCount > 4 ){
            bias =  mouseRepeatCount * 2;
        }
        debugPrint(String(mouseRepeatCount++)+" "+String("Mouse.move ")+String(dX*bias)+String(",")+String(dY*bias));
        Mouse.move(dX*bias,dY*bias,0);
    }else{
        mouseRepeatCount = 0;
    }
    if( dW != 0 ){
        Mouse.move(0,0,dW);
    }
    if( oldClickL ){
        if( !clickL ){
            debugPrint("release MOUSE_LEFT");
            Mouse.release(MOUSE_LEFT);
        }
    }else{
        if( clickL ){
            debugPrint("press MOUSE_LEFT");
            Mouse.press(MOUSE_LEFT);
        }
    }
    if( oldClickM ){
        if( !clickM ){
            debugPrint("release MOUSE_MIDDLE ");
            Mouse.release(MOUSE_MIDDLE );
        }
    }else{
        if( clickM ){
            debugPrint("press MOUSE_MIDDLE ");
            Mouse.press(MOUSE_MIDDLE );
        }
    }
    if( oldClickR ){
        if( !clickR ){
            debugPrint("release MOUSE_RIGHT");
            Mouse.release(MOUSE_RIGHT);
        }
    }else{
        if( clickR ){
            debugPrint("press MOUSE_RIGHT");
            Mouse.press(MOUSE_RIGHT);
        }
    }
    oldClickL = clickL;
    oldClickM = clickM;
    oldClickR = clickR;

      
  lastKeyInfo.accessTime = millis();
  lastKeyInfo.report = report;

  return doMouse;
}

void mouseProcTimeout(){
    mouseProc(lastKeyInfo.report);
    lastKeyInfo.accessTime = millis();
}