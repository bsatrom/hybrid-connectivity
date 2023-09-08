// Copyright 2023 Blues Inc.  All rights reserved.
// Use of this source code is governed by licenses granted by the
// copyright holder including that found in the LICENSE file.

#include "app.h"
#include "config.h"

// Sensor poll interval
#define	QR_READER_SAMPLING_INTERVAL	200

// Worker task for QR reader
void qrReaderTask(void *param)
{
  (void) param;

  while (true)
  {
    tiny_code_reader_results_t results = {};

    // Lock access to I2C bus
    _lock_wire();

    // Perform a read action on the I2C address of the sensor to get the
    // current QR Code information detected.
    if (!tiny_code_reader_read(&results)) {
      debug.println("No person sensor results found on the i2c bus");
    }

    // Unlock access to I2C bus
    _unlock_wire();

    // The Sensor sees a QR Code. Send it to Notehub
    if (results.content_length > 0) {
      debug.print("Found QR Code:");
      debug.print((char*)results.content_bytes);
      debug.println();

      J *req = notecard.newRequest("note.add");
      JAddStringToObject(req, "file", QR_NOTEFILE);
      JAddNumberToObject(req, "port", QR_PORT);
      J *body = JCreateObject();

      JAddStringToObject(body, FIELD_CONTENT, (char*)results.content_bytes);

      JAddItemToObject(req, "body", body);
      JAddBoolToObject(req, "sync", true);
      if (!notecard.sendRequest(req)) {
        debug.println("Could not send QR Code to Notecard");
      }
    }

    // Delay for sensor poll interval
    _delay(QR_READER_SAMPLING_INTERVAL);
  }
}
