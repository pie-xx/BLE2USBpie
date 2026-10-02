#ifndef BLEPROC_H
#define BLEPROC_H
enum ACTMODE {AM_BOOT_SCAN, AM_DISCONNECT_SCAN, AM_DO_CONNECT, AM_MOUSE, AM_KEY, AM_KEY_KANA} ;

void BLEprocInit();
bool connectKeyboard();
#endif