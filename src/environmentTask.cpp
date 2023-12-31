// Copyright 2023 Blues Inc.  All rights reserved.
// Use of this source code is governed by licenses granted by the
// copyright holder including that found in the LICENSE file.

#include "app.h"
#include "config.h"

// Sensor poll interval
#define	SENSOR_SAMPLING_INTERVAL_MS	60000

// Sensor contexts
typedef struct {
	float temperature;
  float humidity;
  float pressure;
  float gas_resistance;
  float altitude;
} bmeContext;

extern Adafruit_BME680 bme;

bmeContext *bmeSensor;

// Last time heartbeat was sent
uint32_t lastEnvEnvironmentHeartbeatMins = 0;
uint32_t heartbeatEnvironmentDueMs = 0;

// Forwards
void bmeSensorSample(bmeContext *s);

// Worker task that monitors connected sensors
void environmentTask(void *param)
{
	(void) param;

  digitalWrite(LED_BUILTIN, HIGH);

	// Initialize context for sensors
  bmeSensor = (bmeContext *) _malloc(sizeof(bmeContext));
	if (bmeSensor == NULL) {
		debug.printf("monitor: can't allocate BME sensor context\n");
		return;
	}
	memset(bmeSensor, 0, sizeof(bmeContext));

	// Loop indefinitely, polling sensors
	for (;;) {

		// See if a heartbeat is due
		if (envHeartbeatMins != lastEnvEnvironmentHeartbeatMins) {
			heartbeatEnvironmentDueMs = 0;
			lastEnvEnvironmentHeartbeatMins = envHeartbeatMins;
		}
		uint32_t nowMs = _millis();
		bool heartbeatDue = (nowMs >= heartbeatEnvironmentDueMs);
		if (heartbeatDue) {
			heartbeatEnvironmentDueMs = nowMs + (envHeartbeatMins * 60L * 1000L);
		}

    // Measure the BME sensor values
    bmeSensorSample(bmeSensor);

    // If a periodic update is required, send it
    if (heartbeatDue) {
      J *req = notecard.newRequest("note.add");
      JAddStringToObject(req, "file", AIR_NOTEFILE);
      JAddNumberToObject(req, "port", AIR_PORT);
      J *body = JCreateObject();

      JAddNumberToObject(body, FIELD_TEMPERATURE, bmeSensor->temperature);
      JAddNumberToObject(body, FIELD_HUMIDITY, bmeSensor->humidity);
      JAddNumberToObject(body, FIELD_PRESSURE, bmeSensor->pressure);
      JAddNumberToObject(body, FIELD_GAS, bmeSensor->gas_resistance);
      JAddNumberToObject(body, FIELD_ALTITUDE, bmeSensor->altitude);

      JAddItemToObject(req, "body", body);
      JAddBoolToObject(req, "sync", true);
      notecard.sendRequest(req);
    }

    digitalWrite(LED_BUILTIN, LOW);

		// Delay for sensor poll interval
		_delay(SENSOR_SAMPLING_INTERVAL_MS);
	}

}

// Sample the BME680 sensor
void bmeSensorSample(bmeContext *s) {
  // Lock access to I2C bus
  _lock_wire();

  if (!bme.performReading()) {
    debug.println("Failed to perform reading from BME680 sensor");
  } else {
    s->temperature = bme.temperature;
    s->humidity = bme.humidity;
    s->pressure = bme.pressure / 100.0;
    s->gas_resistance = bme.gas_resistance / 100.0;
    s->altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  }

  // Unlock access to I2C bus
  _unlock_wire();
}
