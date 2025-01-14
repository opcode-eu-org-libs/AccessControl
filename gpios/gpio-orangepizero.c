#include <stdio.h>
#include <gpiod.h>
#include <string.h>

#include "config.h"
#include "error_reporting.h"
#include "gpios/gpio.h"

#ifdef ACTIVE_ON_GND  // input activated via connect to GND
#define GPIOD_ACTIVE true
#define GPIOD_FLAGS  GPIOD_CTXLESS_FLAG_BIAS_PULL_UP
#else                 // input activated via connect to 3.3V
#define GPIOD_ACTIVE false
#define GPIOD_FLAGS  GPIOD_CTXLESS_FLAG_BIAS_PULL_DOWN
#endif

#ifndef GPIO_DEVICE
#define GPIO_DEVICE "gpiochip0"
#endif

enum {
	DO_UNLOCK_DOOR_1 = 13, // PA13
	DI_MANUAL_OPEN_1 = 14, // PA14
	DI_IS_EMERGENCY_OPEN_1 = 15, // PA15
	DI_IS_DOOR_LOCK_1 = 16, // PA16
	
	DO_UNLOCK_DOOR_2 = 7, // PA7
	DI_MANUAL_OPEN_2 = 6, // PA6
	DI_IS_EMERGENCY_OPEN_2 = 11, // PA11
	DI_IS_DOOR_LOCK_2 = 12, // PA12
	
	PA18 = 18,
	PA19 = 19,
	
	PG06 = 6*32+6,
	PG07 = 6*32+7,
};

#ifndef DOOR_OUTPUTS_NUM
 #ifdef SIGNAL_OUTPUTS_NUM
  #define DOOR_OUTPUTS_NUM (4 - SIGNAL_OUTPUTS_NUM)
 #else
  #define DOOR_OUTPUTS_NUM 4
 #endif
#endif
const unsigned int outputs[] = {DO_UNLOCK_DOOR_1, DO_UNLOCK_DOOR_2, PG06, PG07};

#define INPUTS_DOORS 2
#define INPUTS_TYPES 4
const unsigned int inputs[] = {
	DI_MANUAL_OPEN_1, DI_IS_EMERGENCY_OPEN_1, DI_IS_DOOR_LOCK_1, PA18,
	DI_MANUAL_OPEN_2, DI_IS_EMERGENCY_OPEN_2, DI_IS_DOOR_LOCK_2, PA19,
};

void set_door_state(enum DoorOperations doorOperation, int32_t mask) {
	int val = (doorOperation == DO_UNLOCK) ? 0 : 1;
	
	for (int i=0; i<DOOR_OUTPUTS_NUM; ++i) {
		if (mask & (1<<i)) {
			int ret  __attribute__((unused));
			#ifdef REVERSE_OUTPUT_LOGIC
			ret = gpiod_ctxless_set_value(GPIO_DEVICE, outputs[i], val ^ (REVERSE_OUTPUT_LOGIC << i), false, "gpioset", 0, 0);
			#else
			ret = gpiod_ctxless_set_value(GPIO_DEVICE, outputs[i], val, false, "gpioset", 0, 0);
			#endif
			#if GPIO_DEBUG
			printf("Do door operation %d, mask=0x%02X, ret=%d\n", doorOperation, mask, ret);
			#endif
		}
	}
}

void set_alarm_signal(bool on, int32_t mask) {
	#ifdef SIGNAL_OUTPUTS_NUM
	for (int i=0; i<SIGNAL_OUTPUTS_NUM; ++i) {
		if (mask & (1<<i)) {
			int ret  __attribute__((unused));
			#ifdef REVERSE_OUTPUT_LOGIC
			ret = gpiod_ctxless_set_value(GPIO_DEVICE, outputs[DOOR_OUTPUTS_NUM + i], on ^ (REVERSE_OUTPUT_LOGIC << (DOOR_OUTPUTS_NUM + i)), false, "gpioset", 0, 0);
			#else
			ret = gpiod_ctxless_set_value(GPIO_DEVICE, outputs[DOOR_OUTPUTS_NUM + i], on, false, "gpioset", 0, 0);
			#endif
			#if GPIO_DEBUG
			printf("Set alarm output to %d, mask=0x%02X, ret=%d\n", on, mask, ret);
			#endif
		}
	}
	#endif
}

void get_input_state(uint8_t *values, int32_t mask) {
	int inputsValues[INPUTS_DOORS * INPUTS_TYPES];
	memset(inputsValues, 0x13, sizeof(int)*INPUTS_DOORS * INPUTS_TYPES);
	int ret  __attribute__((unused));
	#ifdef NO_PULLUPDOWN
	ret = gpiod_ctxless_get_value_multiple(GPIO_DEVICE, inputs, inputsValues, INPUTS_DOORS * INPUTS_TYPES, GPIOD_ACTIVE, "gpioget");
	#else
	ret = gpiod_ctxless_get_value_multiple_ext(GPIO_DEVICE, inputs, inputsValues, INPUTS_DOORS * INPUTS_TYPES, GPIOD_ACTIVE, "gpioget", GPIOD_FLAGS);
	#endif
	
	#if GPIO_DEBUG
	printf("input read (status=%d) values: ", ret);
	for (int i=0; i<INPUTS_DOORS * INPUTS_TYPES; ++i) {
		printf("%d->%d ", inputs[i], inputsValues[i]);
	}
	printf("\n");
	#endif
	
	for (int i=0; mask && i<INPUTS_DOORS; ++i) {
		if (mask & 0x01) {
			values[i] = 0;
			for (int j=0; j< INPUTS_TYPES; ++j)
				values[i] |= inputsValues[i*4 + j] << j;
			#ifdef REVERSE_INPUT_LOGIC
			values[i] ^= REVERSE_INPUT_LOGIC;
			#endif
		}
		mask = mask >> 1;
	}
}
