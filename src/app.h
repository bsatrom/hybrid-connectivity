// Copyright 2023 Blues Inc.  All rights reserved.
// Use of this source code is governed by licenses granted by the
// copyright holder including that found in the LICENSE file.

#include <Arduino.h>
#include <Wire.h>
#include <Notecard.h>
#include <Adafruit_BME680.h>
#include <Adafruit_LIS3DH.h>
#include <SFE_MicroOLED.h>
#include "NoteRTOS.h"
#include "tiny_code_reader.h"

#pragma once

#define SEALEVELPRESSURE_HPA (1013.25)

// Notehub definitions
#ifdef LOCALDEV
#define	NOTEHUB_HUB			"brandon.blues.tools"
#define	NOTEHUB_PRODUCT_UID "com.blues.testrobot2"
#else
#define	NOTEHUB_HUB			"-"
#define	NOTEHUB_PRODUCT_UID	"com.blues.ttc"
#endif

// This is the unique Product Identifier for your device
#ifndef PRODUCT_UID
#define PRODUCT_UID "" // "com.my-company.my-name:my-project"
#pragma message "PRODUCT_UID is not defined in this example. Please ensure your Notecard has a product identifier set before running this example or define it in code here. More details at https://dev.blues.io/tools-and-sdks/samples/product-uid"
#endif

// Define this if using USB serial, and comment it out if using the ST-Link V3
// #if !defined(ARDUINO_ARCH_STM32)
// #define debug Serial
// #endif

// Define the debug output stream device, as well as a method enabling us
// to determine whether or not the Serial device is available for app usage.
#ifdef debug
#define	serialIsAvailable() false
#else
#define	serialIsAvailable() true
#ifdef APP_MAIN
HardwareSerial debug(PIN_VCP_RX, PIN_VCP_TX);
#else
extern HardwareSerial debug;
#endif
#endif

// Notecard definition
#ifdef APP_MAIN
Notecard notecard;
#else
extern Notecard notecard;
#endif

// app.cpp
bool appSetup(void);
uint32_t appLoop(void);
