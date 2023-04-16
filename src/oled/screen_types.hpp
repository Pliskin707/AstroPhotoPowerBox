#ifndef __SCREEN_TYPES_H__
#define __SCREEN_TYPES_H__

#include <Adafruit_SSD1306.h>

namespace pliskin
{

typedef enum
{
    home = 0,
    startup,
    power_bat_cam,
    power_pc_mount,
    power_heaters,
    idle,
    off
} e_screen;

class screenBaseClass
{
    public:
        virtual ~screenBaseClass() {};
        virtual uint32_t show (Adafruit_SSD1306 * const pOled) = 0;
        virtual e_screen getType (void) const = 0;
};

}
#endif // __SCREEN_TYPES_H__