// Copyright 2023 Blues Inc.  All rights reserved.
// Use of this source code is governed by licenses granted by the
// copyright holder including that found in the LICENSE file.

#include "app.h"
#include "config.h"

// Cached values of the above environment variables
uint32_t envHeartbeatMins;

// Time used to determine whether or not we should refresh the environment vars
uint32_t environmentModifiedTime = 0;

// Time when the notecard was last rebooted
uint32_t notecardBootTime = 0;

// Initialize the BME680 and Accelerometer
Adafruit_BME680 bme;
Adafruit_LIS3DH accel = Adafruit_LIS3DH();
MicroOLED oled(-1, 1);

// Forwards
void refreshEnvironmentVarCache(J *rsp);
bool appInit(void);
void appTask(void *param);

// Set up everything we can that does NOT need FreeRTOS running
bool appSetup(void)
{
	// Create the tasks
  xTaskCreate(appTask, TASKNAME_APP, TASKSTACK_APP, NULL, TASKPRI_APP, NULL);

	// Done
	return true;
}

// Set up the Notecard in preparation for the power measurement task
bool appInit(void)
{
	J *req, *rsp, *body;

  // Use LED_BUILTIN to indicate the host is awake
  digitalWrite(LED_BUILTIN, HIGH);

  // Turn on monitor mode
  req = notecard.newRequest("card.aux");
	JAddStringToObject(req, "mode", "neo-monitor");
  if (!notecard.sendRequest(req)) {
    debug.printf("notecard not responding\n");
    return false;
  }

  // Initialize the Notecard
  // Configure Notecard connection to Notehub.io
  req = notecard.newRequest("hub.set");
  JAddStringToObject(req, "hub", NOTEHUB_HUB);
  JAddStringToObject(req, "product", NOTEHUB_PRODUCT_UID);
	JAddStringToObject(req, "mode", "continuous");
  JAddBoolToObject(req, "sync", true);
  JAddNumberToObject(req, "inbound", 1);
  JAddNumberToObject(req, "outbound", 15);
  notecard.sendRequestWithRetry(req, 5); // 5 seconds

	// EnvVar Template
  req = notecard.newRequest("env.template");

  body = JCreateObject();
	JAddNumberToObject(body, VAR_MINUTES, TUINT16);
  JAddItemToObject(req, "body", body);
  if (!notecard.sendRequest(req)) {
	  debug.println("unable to set env template");
    return false;
	}

  // Notefile Template for Air
  req = notecard.newRequest("note.template");
  JAddStringToObject(req, "file", AIR_NOTEFILE);
  JAddNumberToObject(req, "port", AIR_PORT);

  body = JCreateObject();
  JAddNumberToObject(body, FIELD_TEMPERATURE, TFLOAT32);
  JAddNumberToObject(body, FIELD_HUMIDITY, TFLOAT32);
  JAddNumberToObject(body, FIELD_PRESSURE, TFLOAT32);
  JAddNumberToObject(body, FIELD_GAS, TFLOAT32);
  JAddNumberToObject(body, FIELD_ALTITUDE, TFLOAT32);
  JAddNumberToObject(body, "_time", TINT32);

  JAddItemToObject(req, "body", body);
  if (!notecard.sendRequest(req)) {
    debug.println("unable to set air template");
    return false;
  }

  // Notefile Template for Accel
  req = notecard.newRequest("note.template");
  JAddStringToObject(req, "file", ACCEL_NOTEFILE);
  JAddNumberToObject(req, "port", ACCEL_PORT);

  body = JCreateObject();
  JAddNumberToObject(body, FIELD_X, TFLOAT32);
  JAddNumberToObject(body, FIELD_Y, TFLOAT32);
  JAddNumberToObject(body, FIELD_Z, TFLOAT32);
  JAddNumberToObject(body, "_time", TINT32);

  JAddItemToObject(req, "body", body);
  if (!notecard.sendRequest(req)) {
    debug.println("unable to set accel template");
    return false;
  }

  // Notefile Template for QR
  req = notecard.newRequest("note.template");
  JAddStringToObject(req, "file", QR_NOTEFILE);
  JAddNumberToObject(req, "port", QR_PORT);

  body = JCreateObject();
  JAddStringToObject(body, FIELD_CONTENT, "qr_code_value");
  JAddNumberToObject(body, "_time", TINT32);

  JAddItemToObject(req, "body", body);
  if (!notecard.sendRequest(req)) {
    debug.println("unable to set qr template");
    return false;
  }

  // Notefile Template for Inbound
  req = notecard.newRequest("note.template");
  JAddStringToObject(req, "file", INBOUND_NOTEFILE);
  JAddNumberToObject(req, "port", INBOUND_PORT);

  body = JCreateObject();
  JAddBoolToObject(body, FIELD_NOTIFY, true);

  JAddItemToObject(req, "body", body);
  if (!notecard.sendRequest(req)) {
    debug.println("unable to set inbound template");
    return false;
  }

  // Load the environment vars for the first time
  rsp = notecard.requestAndResponse(notecard.newRequest("env.get"));
	if (rsp != NULL) {
		refreshEnvironmentVarCache(rsp);
    notecard.deleteResponse(rsp);
	}

  if (!bme.begin()) {
    debug.println("BME680 sensor not found");
  } else {
    debug.println("BME680 sensor found");
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  if (!accel.begin(LIS3DH_DEFAULT_ADDRESS)) {
    debug.println("Accelerometer sensor not found");
  } else {
    debug.println("Accelerometer sensor found.");
  }
  accel.setRange(LIS3DH_RANGE_2_G);   // 2, 4, 8 or 16 G!

  if (!oled.begin()) {
    debug.println("OLED initialization failed");
  } else {
    debug.println("OLED initialization done");

    oled.clear(PAGE);
    oled.clear(ALL);

    oled.setFontType(0);
    oled.setCursor(0, 0);
    oled.println("Hello TTC!");

    oled.display();
  }

  digitalWrite(LED_BUILTIN, LOW);

  // Create the QR Reader Task
  // xTaskCreate(qrReaderTask, TASKNAME_QR_READER, TASKSTACK_QR_READER, NULL, TASKPRI_QR_READER, NULL);

  // Create in inbound note task
  xTaskCreate(inboundNoteTask, TASKNAME_INBOUND_NOTE, TASKSTACK_INBOUND_NOTE, NULL, TASKPRI_INBOUND_NOTE, NULL);

	// Create the Environment Task
  xTaskCreate(environmentTask, TASKNAME_ENVIRONMENT, TASKSTACK_ENVIRONMENT, NULL, TASKPRI_ENVIRONMENT, NULL);

  // Create the Accel Task
  xTaskCreate(accelTask, TASKNAME_ACCEL, TASKSTACK_ACCEL, NULL, TASKPRI_ACCEL, NULL);

  return true;
}

// Worker task for the app
void appTask(void *param)
{
	(void) param;

  // Initialize the app and create other worker tasks as needed
  while (!appInit()) {
      _delay(750);
  }

	// Perform periodic housekeeping
	for (;;) {

		// Look for environment variable changes, and process them if received
		J *req = notecard.newRequest("env.get");
	  JAddNumberToObject(req, "time", environmentModifiedTime);
	  J *rsp = notecard.requestAndResponse(req);
		if (rsp != NULL) {
			if (!notecard.responseError(rsp)) {
				refreshEnvironmentVarCache(rsp);
			}
			notecard.deleteResponse(rsp);
		}

		// Wait for a while
		_delay(15000);
	}
}

// Re-load all env vars, returning the modified time
void refreshEnvironmentVarCache(J *rsp)
{
  // Update the env modified time
  environmentModifiedTime = JGetNumber(rsp, "time");

  // Update the environment
  J *body = JGetObject(rsp, "body");
  if (body == NULL) {
		return;
	}

	// Update heartbeat period
	envHeartbeatMins = JAtoN(JGetString(body, VAR_MINUTES), NULL);
	if (envHeartbeatMins == 0) {
		envHeartbeatMins = 1;
	}

	// See if the notecard has been rebooted, and reboot ourselves if so
  rsp = notecard.requestAndResponse(notecard.newRequest("card.status"));
	uint32_t lastBootTime = JGetNumber(rsp, "time");
	if (lastBootTime != 0) {
	  if (notecardBootTime != 0 && lastBootTime != notecardBootTime) {
			NVIC_SystemReset();
		}
		notecardBootTime = lastBootTime;
	}
  notecard.deleteResponse(rsp);
}