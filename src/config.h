// Copyright 2023 Blues Inc.  All rights reserved.
// Use of this source code is governed by licenses granted by the
// copyright holder including that found in the LICENSE file.

#pragma once

// Monitor task definition
#define TASKNAME_APP				"env"
#define TASKSTACK_APP				4096
#define TASKPRI_APP		      ( configMAX_PRIORITIES - 4 )        // normal, in the middle
#define TASKNAME_MONITOR		"monitor"
#define TASKSTACK_MONITOR   4096
#define TASKPRI_MONITOR     ( configMAX_PRIORITIES - 4 )        // normal, in the middle

// QR Reader task definition
#define TASKNAME_QR_READER		"code_reader"
#define TASKSTACK_QR_READER   4096
#define TASKPRI_QR_READER     ( configMAX_PRIORITIES - 1 )

// Notefile/Note definitions
#define	AIR_NOTEFILE		"air.qo"
#define	AIR_PORT				1
#define	ACCEL_NOTEFILE	"accel.qo"
#define	ACCEL_PORT			2
#define QR_NOTEFILE     "qr.qo"
#define QR_PORT         3

#define FIELD_TEMPERATURE	"temperature"
#define FIELD_HUMIDITY		"humidity"
#define FIELD_PRESSURE		"pressure"
#define FIELD_GAS				  "gas"
#define FIELD_ALTITUDE		"altitude"

#define FIELD_X					  "x"
#define FIELD_Y					  "y"
#define FIELD_Z					  "z"

#define FIELD_CONTENT     "content"

// Environment variable definitions
#define	VAR_MINUTES				"minutes"		// how often to send a full report

// Cached values of the above environment variables
extern uint32_t envHeartbeatMins;

// monitor.cpp
void monitorTask(void *param);

// qr.cpp
void qrReaderTask(void *param);