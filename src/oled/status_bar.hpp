#ifndef __STATUS_BAR_H__
#define __STATUS_BAR_H__

#include <Adafruit_SSD1306.h>
#include <vector>
#include <memory>

#include "config.hpp"
#include "energy saver/energy_saver.hpp"
#include "communication/communication_types.hpp"
#include "non volatile/non_volatile.hpp"
#include "switcher/switcher.hpp"

#define STATUS_SYM_BAT_ICON     (1 << 0)
#define STATUS_SYM_BAT_PERCENT  (1 << 1)
#define STATUS_SYM_WIFI_ICON    (1 << 2)
#define STATUS_SYM_WIFI_VALUE   (1 << 3)
#define STATUS_SYM_PLAYER_NAME  (1 << 4)
#define STATUS_SYM_KEEP_AWAKE   (1 << 5)
#define STATUS_SYM_LOCK_ICON    (1 << 6)

#define STATUS_CALL_PARAMS      Adafruit_SSD1306 * const pOled, const float SoC, const int16_t wifidbm, const String &user, const bool keepAwake, const bool charging

class barSymbol
{
    protected:
        uint16_t _posX = 0;
        uint16_t _posY = 0;
        uint16_t _width = 0;
        uint16_t _height = 8;

        void _clearDisplayPos (Adafruit_SSD1306 * const pOled) const;

    public:
        virtual ~barSymbol() {};
        void setPos (const uint16_t x, const uint16_t y) {_posX = x; _posY = y;};
        uint16_t getPosX (void) const {return _posX;};
        uint16_t getPosY (void) const {return _posY;};
        uint16_t getWidth (void) const {return _width;};
        uint16_t getHeight (void) const {return _height;};
        virtual void printSym (STATUS_CALL_PARAMS) = 0;
        virtual uint32_t getType (void) const = 0;
};

class stateOfChargeIcon : public barSymbol
{
    public:
        stateOfChargeIcon() {_width = 13; _height = 8;};
        void printSym (STATUS_CALL_PARAMS) override;
        uint32_t getType (void) const override {return STATUS_SYM_BAT_ICON;};
};

class stateOfChargeValue : public barSymbol
{
    public:
        stateOfChargeValue() {_width = 17; _height = 7; _posY = 1;};
        void setPos (const uint16_t x, const uint16_t y) {_posX = x; _posY = y + 1;};   // the OLED lib starts printing at the top so the lower parts of characters like 'y' and 'g' can be printed without being moved up, but this is not required here for numbers so move it down
        void printSym (STATUS_CALL_PARAMS) override;
        uint32_t getType (void) const override {return STATUS_SYM_BAT_PERCENT;};
};

class wifiStrengthIcon : public barSymbol
{
    public:
        wifiStrengthIcon() {_width = 8; _height = 8;};
        void printSym (STATUS_CALL_PARAMS) override;
        uint32_t getType (void) const override {return STATUS_SYM_WIFI_ICON;};
};

class standbyIcon : public barSymbol
{
    public:
        standbyIcon() {_width = 8; _height = 8;};
        void printSym (STATUS_CALL_PARAMS) override;
        uint32_t getType (void) const override {return STATUS_SYM_KEEP_AWAKE;};
};

class playerName : public barSymbol
{
    public:
        playerName() {_width = 1; _height = 8;};
        void printSym (STATUS_CALL_PARAMS) override;
        uint32_t getType (void) const override {return STATUS_SYM_PLAYER_NAME;};
};

class lockIcon : public barSymbol
{
    public:
        lockIcon() {_width = 5; _height = 8;};
        void printSym (STATUS_CALL_PARAMS) override;
        uint32_t getType (void) const override {return STATUS_SYM_LOCK_ICON;};
};

class statusBar
{
    private:
        std::vector<std::shared_ptr<barSymbol>> _symbols;

    public:
        std::shared_ptr<barSymbol> addSymbol (const uint32_t symbolFlag);
        void printStatus (Adafruit_SSD1306 * const pOled);
        void setup (const uint32_t symbolFlags = STATUS_SYM_BAT_ICON | STATUS_SYM_WIFI_ICON, const uint16_t posY = 0);
};

#endif // __STATUS_BAR_H__