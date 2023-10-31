#ifndef TFT_GUI_h
#define TFT_GUI_h

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>

/* Screen calibration */
#define TS_MIN 350
#define TS_MAX 3800
/* ~~~~~~~~~~~~~~~~~ */

/* For Wemos Mini and TFT screen 2.4" */
#define TFT_CS D0
#define TFT_DC D8
#define TFT_RST -1
#define TS_CS D3
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

const uint8_t _pos_corr_factor_w = 3;
const uint8_t _pos_corr_factor_h = 4;

extern Adafruit_ILI9341 tft;   /* Graphics */
extern XPT2046_Touchscreen ts; /* Touch screen */

struct TFT_entity
{
  bool roundRect = false;
  bool useBorder = false;
  bool center_txt = true;
  bool latchButton = false;

  int w = 0;
  int h = 0;
  int w_pos = 0;
  int h_pos = 0;

  uint8_t txt_size = 2;
  uint8_t border_thickness = 1;
  uint8_t corner_radius = 5;

  uint16_t txt_color = ILI9341_BLACK;
  uint16_t border_color = ILI9341_RED;
  uint16_t face_color = ILI9341_GREENYELLOW;
  uint16_t pressface_color = ILI9341_CYAN;
};

class MessageTFT
{
public:
  TFT_entity tft_entity;
  Adafruit_ILI9341 *TFT[1];

protected:
  char _txt_buf[30];

public:
  MessageTFT(Adafruit_ILI9341 &_tft = tft);
  void createMSG(const char *txt);
  void createPage(const char *txt[]); // Multiple Lines in MSG
  void updateTXT(const char *txt);
  void clear_screen(uint8_t c = 0);

private:
  void _put_text(const char *txt, uint16_t color = 0);
  void _drawFace();
  void _drawBorder();
};

class ButtonTFT : public MessageTFT
{
public:
  ButtonTFT(XPT2046_Touchscreen &_ts = ts, Adafruit_ILI9341 &_tft = tft);
  void createButton(const char *txt);
  bool wait4press();
  bool checkPress(TS_Point &p);
  bool get_buttonState();
  void set_buttonState(bool state);
  XPT2046_Touchscreen *TS[1];

protected:
private:
  bool _latchState = false;
  unsigned long _lastPress = 0;

private:
  void _press_cb();
  void _conv_ts_tft(TS_Point &p, int &retW, int &retH);
  bool _check_press_geometry(TS_Point &p);
  int _TS2TFT_x(int px);
  int _TS2TFT_y(int py);
};

/* Button Array as class template */
template <uint8_t N>
class buttonArrayTFT
{
public:
  uint8_t dw = 4;           /* define spacing between buttons */
  uint8_t dh = 4;           /* define spacing between buttons */
  uint8_t shift_pos_h = 10; /* Shifts in y director*/
  uint8_t shift_pos_w = 10; /* Shifts in x director*/

  ButtonTFT butarray[N];

public:
  buttonArrayTFT(XPT2046_Touchscreen &_ts = ts, Adafruit_ILI9341 &_tft = tft);
  void set_button_properties(TFT_entity entity);
  void create_array(uint8_t R, uint8_t C, const char *but_txt[]);
  uint8_t checkPress();
  ButtonTFT &operator[](uint8_t index)
  {
    if (index < N)
    {
      return butarray[index];
    }
  }
};

template <uint8_t N>
buttonArrayTFT<N>::buttonArrayTFT(XPT2046_Touchscreen &_ts, Adafruit_ILI9341 &_tft)
{
  for (int i = 0; i < N; i++)
  {
    butarray[i].TS[0] = &ts;
    butarray[i].TFT[0] = &tft;
  }
}

template <uint8_t N>
void buttonArrayTFT<N>::create_array(uint8_t R, uint8_t C, const char *but_txt[])
{
  int w_start_pos = 0;
  int h_start_pos = 0;

  if (butarray[0].tft_entity.h == 0 && butarray[0].tft_entity.w == 0) /* buttons side is defined manually */
  {
    /* auto size */
    butarray[0].tft_entity.w = (int)((tft.width() - dw * (C - 1)) / C);
    butarray[0].tft_entity.h = (int)((tft.height() - dh * (R - 1)) / R);
  }

  if (shift_pos_w == 0) /* No shift is defined - Array will be centered h and w*/
  {
    w_start_pos = (int)(tft.width() - (C - 1) * (butarray[0].tft_entity.w + dw)) / 2;
  }
  else
  {
    w_start_pos = shift_pos_w + butarray[0].tft_entity.w / 2;
  }
  if (shift_pos_h == 0)
  {
    h_start_pos = (int)(tft.height() - (R - 1) * (butarray[0].tft_entity.h + dh)) / 2;
  }
  else
  {
    h_start_pos = shift_pos_h + butarray[0].tft_entity.h / 2;
  }

  for (uint8_t r = 0; r < R; r++)
  {
    for (uint8_t c = 0; c < C; c++)
    {
      butarray[C * r + c].tft_entity.h = butarray[0].tft_entity.h; /* Calculated or defined */
      butarray[C * r + c].tft_entity.w = butarray[0].tft_entity.w; /* Calculated or defined */
      butarray[C * r + c].tft_entity.h_pos = h_start_pos + r * (butarray[0].tft_entity.h + dh);
      butarray[C * r + c].tft_entity.w_pos = w_start_pos + c * (butarray[0].tft_entity.w + dw);
      butarray[C * r + c].tft_entity.txt_size = butarray[0].tft_entity.txt_size;
      butarray[C * r + c].tft_entity.txt_color = butarray[0].tft_entity.txt_color;
      butarray[C * r + c].tft_entity.border_color = butarray[0].tft_entity.border_color;
      butarray[C * r + c].tft_entity.face_color = butarray[0].tft_entity.face_color;
      butarray[C * r + c].tft_entity.roundRect = butarray[0].tft_entity.roundRect;
      butarray[C * r + c].tft_entity.border_thickness = butarray[0].tft_entity.border_thickness;
      butarray[C * r + c].tft_entity.pressface_color = butarray[0].tft_entity.pressface_color;
      butarray[C * r + c].tft_entity.useBorder = butarray[0].tft_entity.useBorder;
      butarray[C * r + c].tft_entity.center_txt = butarray[0].tft_entity.center_txt;
      butarray[C * r + c].tft_entity.corner_radius = butarray[0].tft_entity.corner_radius;
      butarray[C * r + c].tft_entity.latchButton = butarray[0].tft_entity.latchButton;

      butarray[C * r + c].createButton(but_txt[C * r + c]);
    }
  }
}

template <uint8_t N>
uint8_t buttonArrayTFT<N>::checkPress()
{
  if (butarray[0].TS[0]->touched()) // check in any press occured
  {
    TS_Point p = butarray[0].TS[0]->getPoint();
    for (uint8_t i = 0; i < N; i++)
    {
      if (butarray[i].checkPress(p))
      {
        return i;
      }
    }
    return 99;
  }
  else
  {
    return 99;
  }
}

template <uint8_t N>
void buttonArrayTFT<N>::set_button_properties(TFT_entity entity)
{
  butarray[0].tft_entity = entity;
}

// bool buttonArrayTFT<N>::wait4press()
// {
//     if (TS[0]->touched())
//   {
//     TS_Point p = TS[0]->getPoint();
//     return checkPress(p); /* in or out ? */
//   }
//   else
//   {
//     return 0;
//   }
// }

/* End of template */

// class keypadTFT
// {
// #define RESET_KEYPAD_TIMEOUT 10 // seconds

// public:
//   keypadTFT(XPT2046_Touchscreen &_ts = ts, Adafruit_ILI9341 &_tft = tft);
//   void create_keypad();
//   bool getPasscode(TS_Point &p);

// protected:
//   buttonArrayTFT<12> _butarray;

// public:
//   uint8_t counter = 0;
//   uint8_t &scale_f = _butarray.scale_f;
//   uint8_t &shift_pos_h = _butarray.shift_pos_h;
//   uint8_t &shift_pos_w = _butarray.shift_pos_w;
//   uint8_t &txt_size = _butarray.butarray[0].txt_size;
//   int &shrink_shift = _butarray.shrink_shift;
//   uint16_t &txt_color = _butarray.butarray[0].txt_color;
//   uint16_t &border_color = _butarray.butarray[0].border_color;
//   uint16_t &face_color = _butarray.butarray[0].face_color;
//   bool &roundRect = _butarray.butarray[0].roundRect;
//   char keypad_value[15]; /* To reach externally */

// private:
//   void _create_buttons(uint8_t R, uint8_t C, char *but_txt[]);
//   void _reset_keypad_values();
//   bool _check_pressed_in(TS_Point &p);

// private:
//   char _stored_keypad_value[15];
// };

#endif
/*
#define ILI9341_BLACK 0x0000       ///<   0,   0,   0
#define ILI9341_NAVY 0x000F        ///<   0,   0, 123
#define ILI9341_DARKGREEN 0x03E0   ///<   0, 125,   0
#define ILI9341_DARKCYAN 0x03EF    ///<   0, 125, 123
#define ILI9341_MAROON 0x7800      ///< 123,   0,   0
#define ILI9341_PURPLE 0x780F      ///< 123,   0, 123
#define ILI9341_OLIVE 0x7BE0       ///< 123, 125,   0
#define ILI9341_LIGHTGREY 0xC618   ///< 198, 195, 198
#define ILI9341_DARKGREY 0x7BEF    ///< 123, 125, 123
#define ILI9341_BLUE 0x001F        ///<   0,   0, 255
#define ILI9341_GREEN 0x07E0       ///<   0, 255,   0
#define ILI9341_CYAN 0x07FF        ///<   0, 255, 255
#define ILI9341_RED 0xF800         ///< 255,   0,   0
#define ILI9341_MAGENTA 0xF81F     ///< 255,   0, 255
#define ILI9341_YELLOW 0xFFE0      ///< 255, 255,   0
#define ILI9341_WHITE 0xFFFF       ///< 255, 255, 255
#define ILI9341_ORANGE 0xFD20      ///< 255, 165,   0
#define ILI9341_GREENYELLOW 0xAFE5 ///< 173, 255,  41
#define ILI9341_PINK 0xFC18        ///< 255, 130, 198
*/