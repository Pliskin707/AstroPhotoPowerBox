#include "status_bar.hpp"
#include "energy saver/energy_saver.hpp"
#include "communication/communication_types.hpp"
#include "non volatile/non_volatile.hpp"
#include "switcher/switcher.hpp"

static const int8_t wifiStrengthValues[4]   = {-70, -67, -60, -56};    // quality icon bars: better than -56dbm = very good, -70 or worse = very bad

// direction: horizontal (left most pixel is the MSB of Byte0; next line (y + 1) is Byte 1, ...)
const unsigned char NoWifiSymbol[] PROGMEM  = {0x18, 0x66, 0x46, 0x89, 0x91, 0x62, 0x66, 0x18};
const unsigned char StandbySymbol[] PROGMEM = {0x18, 0x5A, 0x5A, 0x99, 0x81, 0x42, 0x66, 0x18};
const unsigned char LockedSymbol[] PROGMEM  = {0x00, 0x70, 0x88, 0x88, 0xF8, 0xD8, 0xD8, 0xF8};
const unsigned char UnlckdSymbol[] PROGMEM  = {0x70, 0x88, 0x80, 0x80, 0xF8, 0xD8, 0xD8, 0xF8};

std::shared_ptr<barSymbol> statusBar::addSymbol (const uint32_t symbolFlag)
{
    std::shared_ptr<barSymbol> symPointer = nullptr;

    switch (symbolFlag)
    {
        case STATUS_SYM_BAT_ICON:       symPointer = std::make_shared<stateOfChargeIcon>();     break;
        case STATUS_SYM_BAT_PERCENT:    symPointer = std::make_shared<stateOfChargeValue>();    break;
        case STATUS_SYM_WIFI_ICON:      symPointer = std::make_shared<wifiStrengthIcon>();      break;
        case STATUS_SYM_KEEP_AWAKE:     symPointer = std::make_shared<standbyIcon>();           break;
        case STATUS_SYM_PLAYER_NAME:    symPointer = std::make_shared<playerName>();            break;
        case STATUS_SYM_LOCK_ICON:      symPointer = std::make_shared<lockIcon>();              break;
    }

    if (symPointer)
        _symbols.push_back(symPointer);

    return symPointer;
}

void statusBar::printStatus(Adafruit_SSD1306 * const pOled)
{
    // read values once so they stay consistent
    const float SoC      = 50.0f;
    const String &user   = "unknown";
    const bool keepAwake = energy.getKeepAwake(), charging = false;
    const int8_t wifidbm = WiFi.RSSI();

    for (const auto sym : _symbols)
        sym->printSym(pOled, SoC, wifidbm, user, keepAwake, charging);
}

void statusBar::setup(const uint32_t symbolFlags, const uint16_t posY)
{
    _symbols.clear();
    uint16_t posX = 127;    // right most pixel

    for (uint32_t flag = 1; flag; flag <<= 1)
        if (flag & symbolFlags)
        {
            const auto symbol = addSymbol(flag);
            if (symbol) // success? (symbol with this flag exists and is implemented)
            {
                switch (flag)
                {
                    case STATUS_SYM_PLAYER_NAME:
                        symbol->setPos(0, posY);
                        break;

                    default:
                        symbol->setPos(posX - symbol->getWidth(), posY);
                        posX = _symbols.back()->getPosX() - 1;  // add the next symbol to the left (keep a distance of one pixel)
                        break;
                }
            }
        }
}

void barSymbol::_clearDisplayPos (Adafruit_SSD1306 * const pOled) const
{
    pOled->fillRect(_posX, _posY, _width, _height, SSD1306_BLACK);
}

void stateOfChargeIcon::printSym(STATUS_CALL_PARAMS) 
{
    bool drawNothing = false;

    // fill
    uint_fast16_t numBars = ((uint_fast8_t) (SoC / 33.3f)) + 1;    // always show at least 1 bar (let it blink if the SOC gets too low)
    if (numBars > 3)
        numBars = 3;

    // let the left most bar blink if the battery is charging
    if (charging && (SoC < SOC_FULL_USER))
    {
        if (millis() & (1 << 10))   // about each second (1024 ms)
            numBars--;
    }
    // let the frame blink if SoC is critical
    else if  (SoC < SOC_CRITICAL)
    {
        numBars = 0;
        drawNothing = (millis() & (1 << 10)) != 0;   // about each second (1024 ms)
    }

    // remove the old content
    _clearDisplayPos(pOled);
    if (drawNothing)
        return;

    // draw the frame
    pOled->drawRect(_posX + 1, _posY, _width - 1, _height, SSD1306_WHITE);
    pOled->drawFastVLine(_posX, _posY + 2, _height - 4, SSD1306_WHITE);

    for (uint_fast16_t bar = 0, barX = 9 + _posX, barY = 2 + _posY; bar < numBars; bar++, barX -= 3)
    {
        // from back to front
        pOled->fillRect(barX, barY, 2, 4, SSD1306_WHITE);
    }

    return;
}

void stateOfChargeValue::printSym(STATUS_CALL_PARAMS) 
{
    // remove the old content
    _clearDisplayPos(pOled);

    if ((millis() & (1 << 10)) && (SoC < SOC_CRITICAL))   // about each second (1024 ms)
        return; // let the value blink

    // print the value
    pOled->setCursor(_posX, _posY);
    pOled->setTextSize(1);
    pOled->printf_P(PSTR("% 3u"), (uint8_t) SoC);
}

void standbyIcon::printSym(STATUS_CALL_PARAMS) 
{
    // remove the old content
    _clearDisplayPos(pOled);

    // no connection or invalid?
    if (!keepAwake)
    {
        // draw standby icon
        pOled->drawBitmap(_posX, _posY, StandbySymbol, 8, 8, SSD1306_WHITE);
        return;
    }
}

void playerName::printSym(STATUS_CALL_PARAMS) 
{
    // remove the old content
    _clearDisplayPos(pOled);

    // get bounds of new content
    int16_t x, y;
    uint16_t h, w;

    pOled->setCursor(_posX, _posY);

    pOled->getTextBounds(user, _posX, _posY, &x, &y, &w, &h);
    _height = h;
    _width = w;

    if (_width)
        pOled->print(user);
}

void wifiStrengthIcon::printSym(STATUS_CALL_PARAMS) 
{
    // remove the old content
    _clearDisplayPos(pOled);

    // no connection or invalid?
    if ((wifidbm < wifiStrengthValues[0]) || !wifidbm)
    {
        // draw "no connection" icon
        pOled->drawBitmap(_posX, _posY, NoWifiSymbol, 8, 8, SSD1306_WHITE);
        return;
    }

    // draw the bars
    for (uint_fast16_t bar = 0, posX = 7 + _posX, posY = 6 + _posY, height = 2; (bar < 4) && (wifidbm >= wifiStrengthValues[bar]); bar++, posX -= 2, posY -=2, height += 2)
        pOled->drawFastVLine(posX, posY, height, SSD1306_WHITE);
}

void lockIcon::printSym(STATUS_CALL_PARAMS)
{
    const auto &symMem = (switcher::getKeyLockState() == switcher::keyLockState::e_unlocked_idle) ? UnlckdSymbol : LockedSymbol;
    pOled->drawBitmap(_posX, _posY, symMem, 5, 8, SSD1306_WHITE);
}