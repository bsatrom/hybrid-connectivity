// Copyright 2023 Blues Inc.  All rights reserved.
// Use of this source code is governed by licenses granted by the
// copyright holder including that found in the LICENSE file.

#include "app.h"
#include "config.h"

// Sensor poll interval
#define	SENSOR_SAMPLING_INTERVAL_MS	60000

typedef struct {
  float x;
  float y;
  float z;
} accelContext;

extern Adafruit_LIS3DH accel;

accelContext *accelSensor;

// Last time heartbeat was sent
uint32_t lastEnvAccelHeartbeatMins = 0;
uint32_t heartbeatAccelDueMs = 0;

// Forward
void accelSensorSample(accelContext *s);

// Worker task that monitors connected sensors
void accelTask(void *param)
{
	(void) param;

  digitalWrite(LED_BUILTIN, HIGH);

  accelSensor = (accelContext *) _malloc(sizeof(accelContext));
  if (accelSensor == NULL) {
    debug.printf("monitor: can't allocate accel sensor context\n");
    return;
  }
  memset(accelSensor, 0, sizeof(accelContext));

	// Loop indefinitely, polling sensors
	for (;;) {

		// See if a heartbeat is due
		if (envHeartbeatMins != lastEnvAccelHeartbeatMins) {
			heartbeatAccelDueMs = 0;
			lastEnvAccelHeartbeatMins = envHeartbeatMins;
		}
		uint32_t nowMs = _millis();
		bool heartbeatDue = (nowMs >= heartbeatAccelDueMs);
		if (heartbeatDue) {
			heartbeatAccelDueMs = nowMs + (envHeartbeatMins * 60L * 1000L);
		}

    // Measure the accelerometer values
    accelSensorSample(accelSensor);

    // If a periodic update is required, send it
    if (heartbeatDue) {
      J *req = notecard.newRequest("note.add");
      JAddStringToObject(req, "file", ACCEL_NOTEFILE);
      JAddNumberToObject(req, "port", ACCEL_PORT);
      J *body = JCreateObject();

      JAddNumberToObject(body, FIELD_X, accelSensor->x);
      JAddNumberToObject(body, FIELD_Y, accelSensor->y);
      JAddNumberToObject(body, FIELD_Z, accelSensor->z);

      JAddItemToObject(req, "body", body);
      JAddBoolToObject(req, "sync", true);
      notecard.sendRequest(req);
    }

    digitalWrite(LED_BUILTIN, LOW);

		// Delay for sensor poll interval
		_delay(SENSOR_SAMPLING_INTERVAL_MS);
	}
}

void accelSensorSample(accelContext *s) {
  sensors_event_t event;

  // Lock access to I2C bus
  _lock_wire();

  if (!accel.getEvent(&event)) {
    debug.println("Failed to perform reading from Accelerometer");
  } else {
    s->x = event.acceleration.x;
    s->y = event.acceleration.y;
    s->z = event.acceleration.z;
  }

  // Unlock access to I2C bus
  _unlock_wire();
}
