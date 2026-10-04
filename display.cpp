#include "display.h"

String alignRight(String str, int len){
    while (str.length() < len) {
        str = " " + str;
    }
    return str;
}

unsigned long oldmillis = 0;
void debugPrint(String msg){
  unsigned long nowmillis = millis();
    Serial.println( 
        alignRight(String(nowmillis), 12)+String(" ")+ 
        alignRight(String(nowmillis - oldmillis), 8)+String(" ")+ 
        msg );
    oldmillis = nowmillis;
}

String dumpReportString(uint8_t *data, size_t length){
    String reportText = "";
    for (size_t i = 0; i < length; i++) {
        String hex = String(data[i], HEX);
        if (hex.length() < 2){
            reportText += "0";
        }
        reportText += hex + " ";
    }
    return reportText;
}

void dumpKeyReport(uint8_t *data, size_t length, String msg){

    debugPrint(msg+dumpReportString(data, length));
}

Adafruit_NeoPixel pixels(NUM_PIXELS, RGB_PIN, NEO_GRB + NEO_KHZ800);

void displayInit(){
  
    pixels.begin(); 
    pixels.setBrightness(20); // 眩しすぎるのを防ぐため輝度を下げる(0-255)

}

void dispDoConnectStat(){
  // 青色
        pixels.setPixelColor(0, pixels.Color(0, 0, 255));
        pixels.show();
}

void dispBootScanStat(){
        // 青色
        pixels.setPixelColor(0, pixels.Color(0, 0, 128));
        pixels.show();
        delay(800);
        pixels.setPixelColor(0, pixels.Color(0, 0, 0));
        pixels.show();
        delay(200);
}

void dispDisconnectStat(){
          // 赤色
        pixels.setPixelColor(0, pixels.Color(255, 0, 0));
        pixels.show();
        delay(200);
        pixels.setPixelColor(0, pixels.Color(0, 0, 0));
        pixels.show();
        delay(200);
}

void dispAtMouseStat(){
        pixels.setPixelColor(0, pixels.Color(0, 128, 0));
        pixels.show();
        delay(20);
        pixels.setPixelColor(0, pixels.Color(0, 0, 0));
        pixels.show();
        delay(20);
}

void dispAtKeyStat(){
        pixels.setPixelColor(0, pixels.Color(0, 128, 0));
        pixels.show();
}
void dispAtKeyKanaStat(){
        pixels.setPixelColor(0, pixels.Color(255, 255, 0));
        pixels.show();
}
