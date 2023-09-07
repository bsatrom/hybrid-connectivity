// Copyright 2023 Blues Inc.  All rights reserved.
//
// Use of this source code is governed by licenses granted by the
// copyright holder including that found in the LICENSE file.

// App definitions
#define APP_MAIN
#include "app.h"

void setup() {
  // Initialize debug IO
  pinMode(LED_BUILTIN, OUTPUT);
  debug.begin(115200);

  while (!debug);
  debug.println("*** " __DATE__ " " __TIME__ " ***");

  // Initialize the RTOS support (see NoteRTOS.h)
  _setup();

  // Initialize I2C
  Wire.begin();

  // Initialize Notecard library (without doing any I/O on this task)
  notecard.setDebugOutputStream(debug);
	notecard.setFnNoteMutex(_lock_note, _unlock_note);
	notecard.setFnI2cMutex(_lock_wire, _unlock_wire);
  notecard.begin();

  // Initialize the app and create tasks as needed
  appSetup();

	// Start the scheduler
	_setup_completed();
}

// Not used in freertos (and must not block)
void loop() {
}
