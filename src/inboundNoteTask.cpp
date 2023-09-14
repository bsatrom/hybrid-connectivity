// Copyright 2023 Blues Inc.  All rights reserved.
// Use of this source code is governed by licenses granted by the
// copyright holder including that found in the LICENSE file.

#include "app.h"
#include "config.h"

// Sensor poll interval
#define	INBOUND_NOTE_POLL_INTERVAL	5000

extern MicroOLED oled;

// Worker task for Inbound Note reader
void inboundNoteTask(void *param)
{
  (void) param;

  while (true)
  {
    J *req = notecard.newRequest("note.get");
    JAddStringToObject(req, "file", INBOUND_NOTEFILE);
    JAddBoolToObject(req, "delete", true);
    J *rsp = notecard.requestAndResponse(req);
    if (rsp != NULL)
    {
        if (!notecard.responseError(rsp))
        {
          J *body = JGetObject(rsp, "body");
          if (body != NULL)
          {
              bool notify = JGetBool(body, FIELD_NOTIFY);

              if (notify)
              {
                debug.println("Inbound Note: Notify");

                // Write a message to the OLED
                oled.clear(PAGE);
                oled.clear(ALL);
                oled.setCursor(0, 0);
                oled.println("Got an");
                oled.println("inbound");
                oled.println("note!");
                oled.display();
              }
          }
        }
    }
    notecard.deleteResponse(rsp);

    // Delay for sensor poll interval
    _delay(INBOUND_NOTE_POLL_INTERVAL);
  }
}
