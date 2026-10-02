#ifndef MOUSEPROC_H
#define MOUSEPROC_H
#include <Arduino.h>
#include "display.h"
#include "keyboardProc.h"
#include "USBHIDKeyboard.h"
#include "USBHIDMouse.h"

#pragma once
extern KeyInfo lastKeyInfo;

void mouseProcInit();
void restartMouse();
bool mouseProc( KeyReport report );
bool isMouseTimeout();
void mouseProcTimeout();
#endif