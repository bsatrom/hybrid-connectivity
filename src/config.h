// Copyright 2023 Blues Inc.  All rights reserved.
// Use of this source code is governed by licenses granted by the
// copyright holder including that found in the LICENSE file.

#pragma once

// Application task definition
#define TASKNAME_APP				    "env"
#define TASKSTACK_APP				    4096
#define TASKPRI_APP		          ( configMAX_PRIORITIES - 4 )        // normal, in the middle

// Environment task definition
#define TASKNAME_ENVIRONMENT		"environment"
#define TASKSTACK_ENVIRONMENT   4096
#define TASKPRI_ENVIRONMENT     ( configMAX_PRIORITIES - 4 )        // normal, in the middle

// Accel task definition
#define TASKNAME_ACCEL      		"accel"
#define TASKSTACK_ACCEL         4096
#define TASKPRI_ACCEL           ( configMAX_PRIORITIES - 4 )        // normal, in the middle

// QR Reader task definition
#define TASKNAME_QR_READER		  "qrReader"
#define TASKSTACK_QR_READER     2048
#define TASKPRI_QR_READER       ( configMAX_PRIORITIES - 2 )

// Inbound Note task definition
#define TASKNAME_INBOUND_NOTE		"inboundNote"
#define TASKSTACK_INBOUND_NOTE  2048
#define TASKPRI_INBOUND_NOTE    ( configMAX_PRIORITIES - 2 )

// Notefile/Note definitions
#define	AIR_NOTEFILE		     "air.qo"
#define	AIR_PORT             1
#define	ACCEL_NOTEFILE	     "accel.qo"
#define	ACCEL_PORT			     2
#define QR_NOTEFILE          "qr.qo"
#define QR_PORT              3
#define INBOUND_NOTEFILE     "info.qi"
#define INBOUND_PORT         4

#define FIELD_TEMPERATURE	"temperature"
#define FIELD_HUMIDITY		"humidity"
#define FIELD_PRESSURE		"pressure"
#define FIELD_GAS				  "gas"
#define FIELD_ALTITUDE		"altitude"

#define FIELD_X					  "x"
#define FIELD_Y					  "y"
#define FIELD_Z					  "z"

#define FIELD_CONTENT     "content"

#define FIELD_NOTIFY      "notify"

// Environment variable definitions
#define	VAR_MINUTES				"minutes"		// how often to send a full report

// Cached values of the above environment variables
extern uint32_t envHeartbeatMins;

// environmentTask.cpp
void environmentTask(void *param);

// accelTask.cpp
void accelTask(void *param);

// qrReaderTask.cpp
void qrReaderTask(void *param);

// inboundNoteTask.cpp
void inboundNoteTask(void *param);