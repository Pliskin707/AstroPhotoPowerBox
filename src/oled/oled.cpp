#include "oled.hpp"

pliskin::statusDisplay display(128, 32, &Wire, -1);
namespace pliskin
{
// 'WarningSymbol, 16x16px
const unsigned char WarningSymbol[] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x03, 0xc0, 0x02, 0x40, 0x06, 0x60, 0x06, 0x60, 0x0e, 0x70, 
	0x1e, 0x78, 0x1e, 0x78, 0x3f, 0xfc, 0x3e, 0x7c, 0x7e, 0x7e, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00
};

void statusDisplay::setup (void)
{
    Wire.begin();
    begin(SSD1306_SWITCHCAPVCC, 0x3C);
    setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    setTextWrap(false);
    showStartupScreen();
    dim(true);
    _statusBar.setup(STATUS_SYM_WIFI_ICON | STATUS_SYM_LOCK_ICON);
}

void statusDisplay::loop(void)
{
    const uint32_t sysTime = millis();
    bool show = false;

    if (sysTime >= _nextBarUpdate)
    {
        _nextBarUpdate = sysTime + _barUpdateDelay;
        clearDisplay();

        _statusBar.printStatus(this);
        // static float testSoC = 0;   // TODO replace with real SoC
        // showFullScreenSoC(testSoC);
        // testSoC++;
        // if (testSoC > 110.0f) testSoC = 0;

        const auto &btnInfo = switcher::getButtonInfo();
        setCursor(0, 0);
        printf_P(PSTR("LED:%5u"), switcher::getButtonLedPwm());
        setCursor(0, 8);
        printf_P(PSTR("Lock: %u; Cns: %u"), switcher::getKeyLockState(), switcher::getConsumersState());
        setCursor(0, 16);
        printf_P(PSTR("PrsDur:%6lu ms"), btnInfo.lastButtonReleaseTime - btnInfo.lastButtonPressTime);
        setCursor(0, 24);
        printf_P(PSTR("Num:%d (%s)"), btnInfo.numPressesSinceStart, (btnInfo.pressed ? "pressed":"released"));

        show = true;
    }

    // show on screen
    if (show)
        display();
}

void statusDisplay::showStartupScreen (void)
{
    clearDisplay();
    setCursor(0, 0);
    setTextSize(2);
    print(DEVICE_PREFIX);
    setTextSize(1);
    setCursor(0, 16);
    print(F("Version: "));
    print(VERSION_STRING);
    setCursor(0, 24);
    print(WiFi.macAddress().c_str());
    display();
}

void statusDisplay::printBarAt (const int y, const float percent, const char * const text, const int height)
{
    const int16_t 
    frameWitdh  = 2, 
    maxWidth    = width() - 2 * frameWitdh, 
    barLength   = maxWidth * (percent / 100.0f),
    barXEnd     = frameWitdh + barLength,
    textYPos    = y + height - 2 * frameWitdh - 8;  // size of text at 1 is 8 pixels

    // print the frame
    drawRect(0, y, width(), height, SSD1306_WHITE);

    // fill it
    fillRect(2, y + frameWitdh, barLength, height - 2 * frameWitdh, SSD1306_WHITE);

    if (text)
    {
        setCursor(0, textYPos);
        // char text[10];
        // itoa((int) percent, text, 10);
        const int16_t textXStart = centerText(text);

        const int16_t textXEnd = getCursorX();
        if (textXStart < barXEnd)
        {
            // reverse the part of the text which should not yet be black on white
            for (int16_t invertY = 0; invertY < 8; invertY++)
            {
                const int16_t 
                nextYPos = textYPos + invertY, 
                endXPos = ((textXEnd < barXEnd) ? textXEnd : barXEnd);

                for (int16_t nextXPos = textXStart; nextXPos < endXPos; nextXPos++)
                {
                    bool pixelIsOn = getPixel(nextXPos, nextYPos);
                    drawPixel(nextXPos, nextYPos, (pixelIsOn ? SSD1306_BLACK : SSD1306_WHITE));
                }
            }
        }
    }

    return;
}


int16_t statusDisplay::centerText (const String &text)
{
    int16_t xPos = width(), x1, y1;
    const int16_t yPos = getCursorY();
    uint16_t width, height;
    getTextBounds(text, 0, yPos, &x1, &y1, &width, &height);
    xPos -= width;
    xPos /= 2;

    if (xPos < 0)
        xPos = 0;
    
    setCursor(xPos, yPos);
    print(text);

    return xPos;
}

int16_t statusDisplay::rightText(const String &text)
{
    int16_t xPos = width(), x1, y1;
    const int16_t yPos = getCursorY();
    uint16_t width, height;
    getTextBounds(text, 0, yPos, &x1, &y1, &width, &height);
    xPos -= width;

    if (xPos < 0)
        xPos = 0;
    
    setCursor(xPos, yPos);
    print(text);

    return xPos;
}

void statusDisplay::showWarning (const char * const text)
{
    fillRect(0, 0, width(), 16, SSD1306_BLACK);

    if (text)
    {
        drawBitmap(1, 0, WarningSymbol, 16, 16, SSD1306_WHITE);
        drawBitmap(111, 0, WarningSymbol, 16, 16, SSD1306_WHITE);
        drawRect(23, 2, 82, 12, SSD1306_WHITE);
        setCursor(0, 4);
        centerText(text);
    }
    else
    {
        for (uint16_t ii = 0, xpos = 1; ii < 6; ii++, xpos += (16 + 6))
            drawBitmap(xpos, 0, WarningSymbol, 16, 16, SSD1306_WHITE);
    }
}

void statusDisplay::showFullScreenSoC (const float SoC)
{
    const int16_t fillHeight = height() - 2 * 2;
    const int16_t fillWidth = fillHeight * 2;
    const int16_t rectWidth = fillWidth + 2 * 2;
    int16_t iSoC = (int16_t) ((fmaxf(0.0f, fminf(100.0f, SoC)) / 100.0f) * fillWidth);

    // carving method
    fillRect(0, 0, rectWidth, height(), SSD1306_WHITE);
    fillRect(rectWidth, 8, 3, (height() - 2 * 8), SSD1306_WHITE);
    drawRect(1, 1, rectWidth - 2, height() - 2, SSD1306_BLACK);

    // invert the content of the empty part
    for (int fillLine = 0; fillLine < fillHeight; fillLine++)
    {
        for (int vPos = iSoC + (fillLine / 3); vPos < fillWidth; vPos++)  // slightly angeled
        {
            const int16_t x = vPos + 2, y = fillLine + 2;
            drawPixel(x, y, !getPixel(x, y));
        }
    }

    // print text info
    setTextSize(2);
    setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    setCursor(rectWidth + 8, 0);
    printf_P(PSTR("%3d"), (int) SoC);
    setTextSize(1);
    setCursor(getCursorX(), 8);
    print('%');
    // printf_P(PSTR("%03.0f%%"), SoC);
    setTextSize(2);
    setCursor(rectWidth + 8, 16);
    printf_P(PSTR("%3d"), (int) battery.getCapacityRemaining());
    setTextSize(1);
    setCursor(getCursorX(), 24);
    print("Wh");
    // printf_P(PSTR("%3d Wh"), battery.getCapacityRemaining());

    // restore default values
    setTextSize(1);
    setTextColor(SSD1306_WHITE, SSD1306_BLACK);
}

};