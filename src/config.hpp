#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

#ifndef STR
#define STRINGIFY(x)    #x
#define STR(x)          STRINGIFY(x)
#endif

#define VERSION_MAJOR   1
#define VERSION_MINOR   0
#define VERSION_PATCH   3
#define VERSION_STRING  STR(VERSION_MAJOR) "." STR(VERSION_MINOR) "." STR(VERSION_PATCH)

#define DEVICE_PREFIX           "AstroNode"

#define UDP_PORT                (5051)
#define SERVICE_NAME            "AstroPowerBox"

#define MAX_SLEEP_DURATION_MS   (1000uL)
// #define BAT_MIN_TURN_ON_VOLTAGE  (11.0f)    // do not turn off if the voltage is lower than this value
#define BAT_MIN_TURN_ON_VOLTAGE  (-1.0f)    //! do not turn off if the voltage is lower than this value // FIXME change this back
#define SOC_CRITICAL             (20.0f)    // battery icons start blinking if the state of charge drops below this value
#define SOC_FULL_USER            (95.0f)    // battery is displayed as "full" if this value is reached (charging animation stops)

#define TIMEZONESTR   "CET-1CEST,M3.5.0,M10.5.0/3"  // Berlin (see https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv)
#define NTPSERVERSTR  "time.google.com"

#endif