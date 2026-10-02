#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

void debugPrint(String msg);
void dumpKeyReport(uint8_t *data, size_t length);
String dumpReportString(uint8_t *data, size_t length);

#include <Adafruit_NeoPixel.h>

#define RGB_PIN      48 // WROOM-1開発ボードの標準ピン
#define NUM_PIXELS   1  


void displayInit();
void dispDoConnectStat();
void dispBootScanStat();
void dispDisconnectStat();
void dispAtMouseStat();
void dispAtKeyStat();
void dispAtKeyKanaStat();
#endif