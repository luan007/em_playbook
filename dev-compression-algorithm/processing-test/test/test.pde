byte b[];

void setup() {
  size(800, 600);
  b = loadBytes("test.bin");
  println(b.length);
  
  int _ended = 0;
  int x = 0;
  int y = 0;
  int __x = 0;
  int __y = 0;
  while (true) {
    for (int i = 0; i < b.length; i += 2) {
      int c = (b[i + 1] & 0xff);
      int count = (b[i] & 0xff);
      for (int j = 0; j < count; j++) {
        if (x < 800 && y < 600) {
          fill(c);
          noStroke();
          rect(__x, __y, 1 ,1);
          __x++;
          if(__x >= width) {
              __x = 0;
              __y ++;
          }
        }
//        display.epd2.writeSingleByte(0x00);
        
        x++;
        
        if (x >= 801) {
          y++;
//          Serial.println(y);
          x = 0;
        }
        if (y >= 601) {
          _ended = 1;
        }
      }
    }
    if (_ended == 1) break;
  }
}
