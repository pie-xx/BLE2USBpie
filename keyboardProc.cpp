#include "keyboardProc.h"
#include "keyDefine.h"
#include <vector>
#include "outkeytable.h"

KeyInfo lastKeyInfo;
CustomKeyboard Keyboard;
int keyRepeatCount = 0;

std::vector<KeyMkBrkInfo> keyQue;
ROMAJISEQ outseq ;

uint8_t oyaRkey = HID_TRNS;
uint8_t oyaLkey = HID_NTRN;

// PCからLEDステータス変更（Output Report）が届いたときに呼ばれるコールバック
void handleLED(uint8_t leds) {
  // leds には各Lock状態がビット単位で格納されています
  // bit 0: Num Lock (0x01)
  // bit 1: Caps Lock (0x02)
  // bit 2: Scroll Lock (0x04)
  debugPrint(String(leds,HEX));
  bool capsLockOn = leds & 0x02; // Caps Lockの状態を取得

  if (capsLockOn) {
    debugPrint("Caps Lock ON"); // Caps Lock ONならLED点灯
  } else {
    debugPrint("Caps Lock Off");  // Caps Lock OFFならLED消灯
  }
}

void keyboardProcInit(){
// LEDのステータス変化を受け取るコールバックを登録
  Keyboard.onLED(handleLED);
  Keyboard.begin();
}

bool isPressed( uint8_t code, uint8_t* keys ){
  for( int i = 0; i < 6; ++i){
    if( code == keys[i]){
      return true;
    }
  }
  return false;
}

void dumpInQue(){
  
  uint8_t data[64];
  for(int i=0; i<keyQue.size(); ++i){
    data[i] = keyQue[i].hidcode;
  }
  debugPrint("InQue <- "+dumpReportString(data,keyQue.size()));


}

bool addQue( KeyReport report ){
  bool isAllRelease = true;
    for( int i=0; i<6; ++i){
      if( report.keys[i] != 0 ){
        isAllRelease = false;
        if( !isPressed(report.keys[i], lastKeyInfo.report.keys) ){
//          debugPrint("make  "+String(report.keys[i],HEX));
          KeyMkBrkInfo kmbi;
          kmbi.accessTime = millis();
          kmbi.hidcode = report.keys[i];
          kmbi.make = true;
          keyQue.push_back( kmbi );
        }
      }
    }
    for( int i=0; i<6; ++i){
      if( lastKeyInfo.report.keys[i] != 0 ){
        if( !isPressed(lastKeyInfo.report.keys[i], report.keys) ){
//          debugPrint("break "+String(lastKeyInfo.report.keys[i],HEX));
          KeyMkBrkInfo kmbi;
          kmbi.accessTime = millis();
          kmbi.hidcode = HID_timeout; //report.keys[i];
          kmbi.make = false;
          keyQue.push_back( kmbi );
        }
      }
    }

    return !isAllRelease;
}

ROMAJISEQ hid2outSeq(ROMAJISEQ* tbl, uint8_t hidcode){
  if(hidcode==oyaRkey){
    outseq.outcode[0] = hidcode;
    outseq.outcode[1] = 0;
    return outseq;
  }
  
  if( hidcode - 4 > RomaoutLen || hidcode < 4) {
    outseq.outcode[0] = hidcode;
    outseq.outcode[1] = 0;
    return outseq;
  }
//  debugPrint("hid2outSeq tbl "+String(hidcode,HEX)+String(":")+dumpReportString(tbl[hidcode - 4].outcode,4));
  return tbl[hidcode - 4];
}

void putReport(uint8_t outhidcode, uint8_t modifiers){
 //   debugPrint("putReport:" + String(modifiers,HEX )+String("/") + String(outhidcode,HEX));
  if(outhidcode==HID_timeout){
    return;
  }
    KeyReport outreport;
    KeyReport releasereport;
    outreport.modifiers = modifiers;
    releasereport.modifiers = modifiers;
    outreport.reserved = 0x00;
    releasereport.reserved = 0x00;
    for( int i=0; i < 6; ++i){
        outreport.keys[i] = 0x00;
        releasereport.keys[i] = 0x00;
    }
    outreport.keys[0] = outhidcode;
//    debugPrint("put:" + String(modifiers,HEX ) + dumpReportString(outreport.keys,6));

      dumpKeyReport(outreport.keys,6, "Keyboard putReport ");
    Keyboard.sendReport(&outreport);
    delay(10);
    Keyboard.sendReport(&releasereport);
    delay(10);
}

void dumpOutSeq(){
  uint8_t data[64];
  for(int i=0; i<8; ++i){
    if(outseq.outcode[i]==0){
      break;
    }
    data[i] = outseq.outcode[i];
  }
//  debugPrint("outseq -> "+dumpReportString(data,keyQue.size()));

}

void putSeq(uint8_t modifiers){

  bool addshift = false;
  for( int j=0; j<8; ++j){
    if(outseq.outcode[j]==0){
      break;
    }
    if( outseq.outcode[j]==HID_addshift){
      addshift = true;
    }else{
      if( addshift ){
        putReport(outseq.outcode[j], modifiers | 0x02 );
        addshift = false;
      }else{
        putReport(outseq.outcode[j], modifiers);
      }
    }
  }
}

void putQue(uint8_t modifiers){
// dumpInQue();

//先頭のHID_timeoutを除去
  while(keyQue.size()>0 && keyQue[0].hidcode == HID_timeout){
    keyQue.erase(keyQue.begin());
  }
  
  if(keyQue.size()<2)
    return;


  if( keyQue[0].hidcode == oyaRkey) {
    if( keyQue[1].hidcode == HID_timeout){
      outseq = hid2outSeq(hid2romajiNS, oyaRkey);
    }else{
      outseq = hid2outSeq(hid2romajiOR, keyQue[1].hidcode);
    }
    keyQue.erase(keyQue.begin(), keyQue.begin() + 2);
  } else
  if( keyQue[0].hidcode == oyaLkey) {
    if( keyQue[1].hidcode == HID_timeout){
      outseq = hid2outSeq(hid2romajiNS, oyaLkey);
    }else{
      outseq = hid2outSeq(hid2romajiOL, keyQue[1].hidcode);
    }
    keyQue.erase(keyQue.begin(), keyQue.begin() + 2);
  } else {
    if( keyQue[1].hidcode == oyaRkey) {
      //todo: 前後どちらの文字が近いか判定ロジックを入れる
      outseq = hid2outSeq(hid2romajiOR, keyQue[0].hidcode);
      keyQue.erase(keyQue.begin(), keyQue.begin() + 2);
    } else
    if( keyQue[1].hidcode == oyaLkey) {
      //todo: 前後どちらの文字が近いか判定ロジックを入れる
      outseq = hid2outSeq(hid2romajiOL, keyQue[0].hidcode);    
      keyQue.erase(keyQue.begin(), keyQue.begin() + 2);
    } else {
      outseq = hid2outSeq(hid2romajiNS, keyQue[0].hidcode);  
      keyQue.erase(keyQue.begin());
    }
  } 

  putSeq(modifiers);

}

void keyboardProc( KeyReport report ){
    addQue(report);
    putQue(report.modifiers);

    lastKeyInfo.accessTime = millis();
    lastKeyInfo.report = report;
}

bool isKeyboardTimeout(){
  return (millis() - lastKeyInfo.accessTime) > 100 ;
}

bool isAllKeyReleased(){
  for( int i=0; i<6; ++i){
    if( lastKeyInfo.report.keys[i] != 0 ){
      if( keyRepeatCount < 5){
        ++keyRepeatCount;
      }
      return false;
    }
  }
  keyRepeatCount = 0;
  return true;
}

void keyboardProcTimeout(){
  if(!isAllKeyReleased()){
    /*
    String lastseq = "lastseq = ";
    for( int i=0; i<8; ++i){      
      if(outseq.outcode[i]==0){
        break;
      }
      lastseq = lastseq + String(outseq.outcode[i],HEX)+" ";
    }
    debugPrint( lastseq );
    */
    if( keyRepeatCount > 2 ){
      putSeq(lastKeyInfo.report.modifiers);
    }
  }
  lastKeyInfo.accessTime = millis();

  KeyMkBrkInfo kmbi;
  kmbi.accessTime = millis();
  kmbi.hidcode = HID_timeout; //report.keys[i];
  kmbi.make = false;
  keyQue.push_back( kmbi );

  putQue(lastKeyInfo.report.modifiers);
}
