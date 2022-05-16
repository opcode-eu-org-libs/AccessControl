#if __has_include("config_user.h")
# include "myconfig.h"
#endif 

#ifndef DOOR_OPEN_TIME
#define DOOR_OPEN_TIME  13
#endif

#ifndef TWO_FACTOR_TIME
#define TWO_FACTOR_TIME 23
#endif

#ifndef MAIN_LOOP_SLEEP_TIME
#define MAIN_LOOP_SLEEP_TIME 300000
#endif

#ifndef ALARM_RESEND_TIMER
#define ALARM_RESEND_TIMER 200
#endif

#ifndef ALARM_MASK
#define ALARM_MASK 1<<3 // disable alarm on missing DI_IS_DOOR_LOCK signal
#endif


#if 0 // optional defines:

#define DOOR_OUTPUTS_NUM 2 // set number of "door unlock" outputs

#define SIGNAL_OUTPUTS_NUM 2 // set number of outputs for alarm / door-open signalisation (buzzer)

// DOOR_OUTPUTS_NUM + SIGNAL_OUTPUTS_NUM  (must be less-equal then physical outputs supported by gpio module, eg. 4 for gpio-orangepizero.c)


#define NO_PULLUPDOWN // disable pull up/down resistors on inputs

#define ACTIVE_ON_GND // inputs are active when short to gnd (electrical: when set - use pullup, when not set - use pull down)

#define REVERSE_INPUT_LOGIC // bitmask of inputs to reverse (see InputTypes in gpio.h for bits description)

#endif
