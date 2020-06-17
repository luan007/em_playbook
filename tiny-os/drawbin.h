#ifndef GUARD_BIN
#define GUARD_BIN

#include "hardware.h"
#include "display.h"

int bin_flush_screen(int16_t x, int16_t y, int16_t w, int16_t h, bool partial_mode)
{
    display.epd2.refresh(x, y, w, h, partial_mode);
}

int bin_smart_draw(const char *name,
                   int16_t _w, int16_t _h,
                   int16_t BIN_SRC_MINX,
                   int16_t BIN_SRC_MINY,
                   int16_t BIN_SRC_MAXX,
                   int16_t BIN_SRC_MAXY,
                   int16_t DSTX,
                   int16_t DSTY,
                   int16_t FLUSH_COUNT)
{
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
  int _ended = 0;

  int color = 0;
  int count = 0;
  uint8_t buf[512];
  uint8_t out_buf[512];
  int buf_len = 0;
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
        if (x >= BIN_SRC_MINX && x < REAL_MAX_X &&
            y >= BIN_SRC_MINY && y < REAL_MAX_Y)
        {
          //          display.epd2.writeSingleByte((unsigned char)(color & 0xff));
          //            SPI.transfer(color & 0xff);
          out_buf[buf_len++] = color & 0xff;
          if (buf_len >= 512)
          {
            SPI.transfer(out_buf, buf_len);
            buf_len = 0;
          }
        }
        x++;
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
  file.close();

  for (int i = 0; i < FLUSH_COUNT; i++)
  {
    display.epd2.refresh(DSTX, DSTY, clamp_w, clamp_h, false);
  }
}

#endif
