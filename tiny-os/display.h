#ifndef GUARD_DISPLAY
#define GUARD_DISPLAY

#define ENABLE_GxEPD2_GFX 0

#include "src/ink/GxEPD2_BW.h"
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/Tiny3x3a2pt7b.h>

#include "io.h"
//GxEPD2_BW<GxEPD2_it60, GxEPD2_it60::HEIGHT / 24> display(GxEPD2_it60(/*CS=5*/ SS, /*DC=*/0, /*RST=*/2, /*BUSY=*/4));
//
GxEPD2_BW<GxEPD2_it60, GxEPD2_it60::HEIGHT / 24> display(GxEPD2_it60(/*CS=5*/ 5, /*DC=*/0, /*RST=*/15, /*BUSY=*/4));

// const char DisplayInit[] = "Display Init..";
void setup_eink()
{
    // display.init(115200);
    display.init(115200, false, false);
}

void shutdown_eink()
{
    display.hibernate();
}

int e_ink_state = -1;
void power_eink(int ON_OFF)
{
    if (ON_OFF == e_ink_state)
        return;
    e_ink_state = ON_OFF;
    pinMode(PWR_CONTROL, OUTPUT);
    digitalWrite(PWR_CONTROL, ON_OFF);
    pinMode(TPS_CONTROL, OUTPUT);
    digitalWrite(TPS_CONTROL, ON_OFF);
    if (ON_OFF > 0)
    {
        pinMode(IT8951_RESET, OUTPUT);
        digitalWrite(IT8951_RESET, LOW);
        delay(100);
        digitalWrite(IT8951_RESET, HIGH);
        setup_eink();
    }
    else
    {
        // shutdown_eink();
    }
}

const char Hibernating[] = "hibernating ...";
void deepSleepTest()
{
    display.init(115200);

    display.setRotation(3);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_BLACK);

    int16_t tbx, tby;
    uint16_t tbw, tbh;
    display.getTextBounds(Hibernating, 0, 0, &tbx, &tby, &tbw, &tbh);
    // center bounding box by transposition of origin:
    uint16_t x = ((display.width() - tbw) / 2) - tbx;
    uint16_t y = ((display.height() - tbh) / 2) - tby;

    display.setFullWindow();
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(x, y);
        display.print(Hibernating);
    } while (display.nextPage());
    display.hibernate();
}

void draw_bitmap_buffered()
{

    display.setPartialWindow(200, 200, 256, 256);
    display.setRotation(0);
    display.firstPage();
    do
    {
        // display.fillScreen(GxEPD_WHITE);
        for (int y = 0; y < 256; y++)
        {
            //test if this is in range
            if (display.getPagedPixelIndex(200, y + 200) == -1)
                continue;
            for (int x = 0; x < 256; x++)
            {
                display.drawPixel(x + 200, y + 200, (unsigned char)(x + y) % 255);
            }
        }
    } while (display.nextPage(false));
}

void display_end_fullwindow_text()
{
    display.setFullWindow();
    display.setRotation(0);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_BLACK);
}

void draw_text(String s)
{
    display.setPartialWindow(0, 0, 300, 40);
    display.setRotation(0);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(0, 0);
        display.println();
        display.println(s);
        display.println(millis());
    } while (display.nextPage());
}

void console_log(String s)
{
    display.setPartialWindow(0, 0, 300, 40);
    display.setRotation(0);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(0, 0);
        display.println();
        display.println(s);
        display.println(millis());
    } while (display.nextPage());
}

uint8_t bitmap[256];

int i = 1;
void raw_pattern()
{
    for (int t = 0; t < 256; t++)
    {
        bitmap[t] = ((t + i) % 256); //(char)((t % (i + 1) == 0) ? 255 : 0);
    }
    i += 1;
    i = i % 10;
    display.epd2.beginWriteNative(150, 150, 256, 256);
    for (int y = 0; y < 256; y++)
    {
        display.epd2.writeChunk(bitmap, 256);
    }
    display.epd2.endWriteNative(150, 150, 256, 256);
}

#define BEGIN_FULL_WINDOW_TEXT           \
    power_eink(1);                       \
    display.setFullWindow();             \
    display.setRotation(0);              \
    \ 
    display.setFont(&FreeMonoBold9pt7b); \
    display.setTextColor(GxEPD_BLACK);   \
    display.firstPage();                 \
    do                                   \
    {                                    \
        display.fillScreen(GxEPD_WHITE); \
        display.setCursor(0, 0);         \
        display.println();               \
        display.println();

#define END_FULL_WINDOW_TEXT   \
    }                          \
    while (display.nextPage()) \
        ;

#define BEGIN_SMALL_WINDOW_TEXT              \
    power_eink(1);                           \
    display.setPartialWindow(0, 0, 300, 50); \
    display.setRotation(0);                  \
    \ 
    display.setFont(&FreeMonoBold9pt7b);     \
    display.setTextColor(GxEPD_BLACK);       \
    display.firstPage();                     \
    do                                       \
    {                                        \
        display.fillScreen(GxEPD_WHITE);     \
        display.setCursor(0, 0);

#define END_SMALL_WINDOW_TEXT  \
    }                          \
    while (display.nextPage()) \
        ;

void dbg_print(String str)
{
    String final = "\n\n" + str;
    power_eink(1);
    display.setRotation(0);
    display.setPartialWindow(0, 0, 600, 400);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.firstPage();
    do
    {
        display.setCursor(0, 0);
        display.fillScreen(GxEPD_WHITE);
        display.println(final);
    } while (display.nextPage());
}

#endif
