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
    _statusBar.setup(STATUS_SYM_WIFI_ICON);
}

void statusDisplay::loop(void)
{
    const uint32_t sysTime = millis();
    bool show = false;

    if (sysTime >= _nextBarUpdate)
    {
        _nextBarUpdate = sysTime + _barUpdateDelay;

        _statusBar.printStatus(this);
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

};