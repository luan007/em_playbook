#ifndef GUARD_DISPLAY
#define GUARD_DISPLAY

#define ENABLE_GxEPD2_GFX 0

#include "src/ink/GxEPD2_BW.h"
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/Tiny3x3a2pt7b.h>

#include "shared.h"
#include "hal-fs.h"
#include "hal-io.h"
//GxEPD2_BW<GxEPD2_it60, GxEPD2_it60::HEIGHT / 24> display(GxEPD2_it60(/*CS=5*/ SS, /*DC=*/0, /*RST=*/2, /*BUSY=*/4));
//
GxEPD2_BW<GxEPD2_it60, GxEPD2_it60::HEIGHT / 24> display(GxEPD2_it60(/*CS=5*/ 5, /*DC=*/0, /*RST=*/-1, /*BUSY=*/4));

SIGNAL(EINK_DRAW, SIG_NONE, SIG_RUNTIME, 0)
SIGNAL(EINK_MEM_ONLY, SIG_NONE, SIG_RUNTIME, 0)

int e_ink_state = -1;
void display_power(int ON_OFF)
{
    if (ON_OFF == e_ink_state)
        return;
    display.epd2.PROTECTED = 1;
    DEBUG("DISPLAY", "PWR CTRL");
    e_ink_state = ON_OFF;
    pinMode(PWR_CONTROL, OUTPUT);
    digitalWrite(PWR_CONTROL, ON_OFF);
    pinMode(TPS_CONTROL, OUTPUT);
    digitalWrite(TPS_CONTROL, ON_OFF);
    if (ON_OFF > 0)
    {
        pinMode(IT8951_RESET, OUTPUT);
        digitalWrite(IT8951_RESET, LOW);
        vTaskDelay(100); //100+ , 70 is risky as f
        digitalWrite(IT8951_RESET, HIGH);
        display.init(0, false, false);
    }
    else
    {
        display.hibernate();
    }
    display.epd2.PROTECTED = 0;
    sig_set(&SIG_EINK_DRAW, millis());
    //DO NOT INTERRUPT POWER SEQUENCE
}

#define BEGIN_FULL_WINDOW_TEXT           \
    display_power(1);                    \
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
    display_power(1);                        \
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

int text_cycle = 0;

void display_dbg_print(String str)
{
    String final = "\n\n" + str;
    display_power(1);
    display.setRotation(0);
    //always refresh for first time
    text_cycle == 0 ? display.setFullWindow() : display.setPartialWindow(0, 0, 800, 600);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.firstPage();
    do
    {
        display.setCursor(10, 10);
        display.fillScreen(GxEPD_WHITE);
        display.println(final);
    } while (display.nextPage());
    text_cycle++;
    sig_set(&SIG_EINK_DRAW, millis());
}

void display_dbg_print_no_fullscreen(String str)
{
    String final = "\n\n" + str;
    display_power(1);
    display.setRotation(0);
    display.setFullWindow();
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.firstPage();
    do
    {
        display.setCursor(10, 10);
        display.fillScreen(GxEPD_WHITE);
        display.println(final);
        // taskYIELD();
    } while (display.nextPage());
    sig_set(&SIG_EINK_DRAW, millis());
}

struct
{
    int minx = 1000;
    int miny = 1000;
    int maxx = 0;
    int maxy = 0;
    bool dirty = false;
} display_dirty_area;

//draw bin

int reset_dirty_indicator()
{
    display_dirty_area.minx = 1000;
    display_dirty_area.miny = 1000;
    display_dirty_area.maxx = 0;
    display_dirty_area.maxy = 0;
    display_dirty_area.dirty = false;
}

int display_bin_flush_screen(int16_t x, int16_t y, int16_t w, int16_t h, bool partial_mode)
{

    if (SIG_EINK_MEM_ONLY.value != 0)
        return 0;

    int tx = x + w;
    int ty = y + h;
    x = max(min((int)x, 800), 0);
    y = max(min((int)y, 600), 0);
    tx = max(min((int)tx, 800), 0);
    ty = max(min((int)ty, 600), 0);

    w = tx - x;
    h = ty - y;

    Serial.printf("FLUSH (%d,%d) (%d,%d)\n",
                  x,
                  y,
                  w,
                  h);

    if (w + h == 0)
    {
        return 0;
    }
    //this call will be suppressed during mem init
    display.epd2.refresh(x, y, w, h, partial_mode);
    sig_set(&SIG_EINK_DRAW, millis());

    return 1;
}

int display_bin_auto_flush_if_dirty(int count, bool partial_mode)
{
    if (display_dirty_area.dirty)
    {
        Serial.printf("CLEAN (%d,%d) (%d,%d)\n",
                      display_dirty_area.minx,
                      display_dirty_area.miny,
                      display_dirty_area.maxx,
                      display_dirty_area.maxy);
        for (int i = 0; i < count; i++)
        {
            display_bin_flush_screen(
                display_dirty_area.minx,
                display_dirty_area.miny,
                display_dirty_area.maxx - display_dirty_area.minx,
                display_dirty_area.maxy - display_dirty_area.miny,
                partial_mode);
        }
        reset_dirty_indicator();
    }
}

bool GLOBAL_4BIT_MODE = false;
//dirty code - needs clean up!
//memory = 485k
int display_bin_smart_draw(const char *name,
                           int16_t _w, int16_t _h,
                           int16_t BIN_SRC_MINX,
                           int16_t BIN_SRC_MINY,
                           int16_t BIN_SRC_MAXX,
                           int16_t BIN_SRC_MAXY,
                           int16_t DSTX,
                           int16_t DSTY,
                           int16_t FLUSH_COUNT)
{

    GLOBAL_4BIT_MODE = true;
    Serial.printf("DBSD (%d %d) (%d,%d) (%d,%d) => [%d,%d]\n",
                  _w, _h, BIN_SRC_MINX, BIN_SRC_MINY, BIN_SRC_MAXX, BIN_SRC_MAXY, DSTX, DSTY);

    int EINKW = 800;
    int EINKH = 600;
    DSTX = DSTX < 0 ? 0 : DSTX;
    DSTY = DSTY < 0 ? 0 : DSTY;
    int w = BIN_SRC_MAXX - BIN_SRC_MINX;
    int h = BIN_SRC_MAXY - BIN_SRC_MINY;
    int remain_w = EINKW - DSTX;
    int remain_h = EINKH - DSTY;
    int clamp_w = min(w, remain_w);
    int clamp_h = min(h, remain_h);
    int REAL_MAX_X = BIN_SRC_MINX + clamp_w;
    int REAL_MAX_Y = BIN_SRC_MINY + clamp_h;
    // Time recorded for test purposes
    // Get the width and height in pixels of the jpeg if you wish
    // TJpgDec.getFsJpgSize(&w, &h, name); // Note name preceded with "/"

    File file = USE_FS.open(name);
    //  unsigned char buf[2048];
    long len = 0;
    int x = 0;
    int y = 0;
    display.epd2.beginWriteNative(DSTX, DSTY, clamp_w, clamp_h);

    int x_pad_begin = ((DSTX % 4) == 0) ? 0 : (4 - (DSTX % 4));
    int x_pad_end = (((DSTX + clamp_w) % 4) == 0) ? 0 : (4 - ((DSTX + clamp_w) % 4));

    Serial.printf("DISP CRP (%d %d) (%d %d)\n",
                  DSTX, DSTY, DSTX + clamp_w, DSTY + clamp_h);

    Serial.printf("DISP PAD (%d %d)\n", x_pad_begin, x_pad_end);

    int _ended = 0;

    int color = 0;
    int count = 0;
    uint8_t buf[512];
    uint8_t out_buf[512];
    int buf_len = 0;
    int bit_c = 0;
    while (true)
    {
        int len = file.read(buf, sizeof(buf));
        if (len <= 0)
        {
            Serial.println("EOF");
            break;
        }
        for (int i = 0; i < len; i += 2)
        {
            int count = buf[i];
            int color = buf[i + 1];
            for (int j = 0; j < count; j++)
            {
                if (y >= BIN_SRC_MINY && y < REAL_MAX_Y && x == 0)
                { // src line begin
                    for (int q = 0; q < x_pad_begin; q++)
                    {
                        if (bit_c == 0)
                        {
                            bit_c = 1;
                            out_buf[buf_len] = 0;
                        }
                        else
                        {
                            bit_c = 0;
                            out_buf[buf_len++] |= 0;
                        }
                        if (buf_len >= 512)
                        {
                            SPI.transfer(out_buf, buf_len);
                            buf_len = 0;
                        }
                    }
                }

                if (x >= BIN_SRC_MINX && x < REAL_MAX_X &&
                    y >= BIN_SRC_MINY && y < REAL_MAX_Y)
                {
                    //          display.epd2.writeSingleByte((unsigned char)(color & 0xff));
                    //            SPI.transfer(color & 0xff);
                    if (bit_c == 0)
                    {
                        bit_c = 1;
                        out_buf[buf_len] = ((color >> 4) & 0xF) << 4;
                    }
                    else
                    {
                        bit_c = 0;
                        out_buf[buf_len++] |= ((color >> 4) & 0xF);
                    }
                    if (buf_len >= 512)
                    {
                        SPI.transfer(out_buf, buf_len);
                        buf_len = 0;
                    }
                }
                x++;

                if (y >= BIN_SRC_MINY && y < REAL_MAX_Y && x == _w)
                { // src line begin
                    for (int q = 0; q < x_pad_end; q++)
                    {
                        if (bit_c == 0)
                        {
                            bit_c = 1;
                            out_buf[buf_len] = 0;
                        }
                        else
                        {
                            bit_c = 0;
                            out_buf[buf_len++] |= 0;
                        }
                        if (buf_len >= 512)
                        {
                            SPI.transfer(out_buf, buf_len);
                            buf_len = 0;
                        }
                    }
                }

                if (x >= _w)
                {
                    y++;
                    //          Serial.println(y);
                    x = 0;
                }
                if (y >= _h)
                {
                    _ended = 1;
                }
            }
        }
        if (_ended)
            break;
    }
    if (buf_len > 0)
    {
        SPI.transfer(out_buf, buf_len);
        buf_len = 0;
    }
    display.epd2.endWriteNativeNoRefresh();

    GLOBAL_4BIT_MODE = false; //end 4bit mode
    file.close();
    for (int i = 0; SIG_EINK_MEM_ONLY.value == 0 && i < FLUSH_COUNT; i++)
    {
        display.epd2.refresh(DSTX, DSTY, clamp_w, clamp_h, false);
    }
    //real stuff will taint the display!
    if (SIG_EINK_MEM_ONLY.value == 0 && FLUSH_COUNT == 0) //no flush, screen dirty
    {
        //push to dirty area
        display_dirty_area.minx = min(display_dirty_area.minx, (int)DSTX);
        display_dirty_area.miny = min(display_dirty_area.miny, (int)DSTY);
        display_dirty_area.maxx = max(display_dirty_area.maxx, (int)(clamp_w + DSTX));
        display_dirty_area.maxy = max(display_dirty_area.maxy, (int)(clamp_h + DSTY));
        display_dirty_area.dirty = true;
        Serial.printf("DIRTY (%d,%d) (%d,%d)\n",
                      display_dirty_area.minx,
                      display_dirty_area.miny,
                      display_dirty_area.maxx,
                      display_dirty_area.maxy);
    }
    sig_set(&SIG_EINK_DRAW, millis());
}

#endif
