#include "luasys.h"
#include "display.h"
#include "hardware.h"
#include "drawbin.h"

#define MENU_COUNT 5
void _stub_setup()
{
  setup_power_switch();
  setup_eink();
  setup_fs();
  setup_lua();
  setup_hardware();
  Serial.println("Start!");
  bin_smart_draw("/e.bmp.bin", 801, 601, 0, 0, 800, 600, 0, 0);
}

int page = 0 , prePage = 0;
int select = 0, preSelect = 0;
void _stub_loop()
{
  if (sensor.isLeftTouched() == true) {
    Serial.println("Down");
  }

  if (sensor.isMiddleTouched() == true) {
    Serial.println("Middle");
  }

  if (sensor.isRightTouched() == true) {
    Serial.println("Up");
  }
  select = encoder.getCount() % MENU_COUNT;
  select +=  select < 0 ? MENU_COUNT : 0;
  page = select;
  if (page != prePage) {
    Serial.println(page);
    long t = millis();
    switch (page) {
      case 0:
        bin_smart_draw("/e.bmp.bin", 801, 601, 0, 0, 800, 600, 0, 0);
        break;
      case 1:
        bin_smart_draw("/e.bmp.bin", 801, 601, 0, 0, 800, 600, 400, 0);
        //jpeg_smart_draw("/system/b.jpg", 0, 0, 800, 600, 0, 0);
        //jpeg_smart_draw("/system/t.jpg", 0, 0, 70, 280, 0, 0);
        //jpeg_smart_draw("/system/t1.jpg", 0, 0, 70, 70, 0, 210);

        break;
      case 2:
        //        jpeg_smart_draw("/system/c.jpg", 0, 0, 800, 600, 0, 0);
        bin_smart_draw("/e.bmp.bin", 801, 601, 0, 0, 800, 600, 0, 300);
        //        jpeg_smart_draw("/system/t.jpg", 0, 0, 70, 280, 0, 0);
        //        jpeg_smart_draw("/system/t2.jpg", 0, 0, 70, 70, 0, 140);

        break;
      case 3:
        //        jpeg_smart_draw("/system/d.jpg", 0, 0, 800, 600, 0, 0);
        bin_smart_draw("/e.bmp.bin", 801, 601, 0, 0, 800, 600, 400, 300);
        //        jpeg_smart_draw("/system/t.jpg", 0, 0, 70, 280, 0, 0);
        //        jpeg_smart_draw("/system/t3.jpg", 0, 0, 70, 70, 0, 70);

        break;
      case 4:
        //        jpeg_smart_draw("/system/e.jpg", 0, 0, 800, 600, 0, 0);

        bin_smart_draw("/e.bmp.bin", 801, 601, 400, 400, 800, 600, 400, 300);
        //        jpeg_smart_draw("/system/t.jpg", 0, 0, 70, 280, 0, 0);
        //        jpeg_smart_draw("/system/t4.jpg", 0, 0, 70, 70, 0, 0);
        break;
      default:
        //        jpeg_smart_draw("/system/a.jpg", 0, 0, 800, 600, 0, 0);
        bin_smart_draw("/e.bmp.bin", 801, 601, 0, 0, 800, 600, 0, 0);
        break;
    }
    Serial.println(millis() - t);
    prePage = page;
  }

}
