#include "TFT_GUI.h"

MessageTFT::MessageTFT(Adafruit_ILI9341 &_tft)
{
  TFT[0] = &_tft;
}
void MessageTFT::createMSG(const char *txt)
{
  _drawFace();
  _put_text(txt);
}
void MessageTFT::createPage(const char *txt[])
{
  createMSG("");
  const uint8_t shiftxy = 5;
  for (uint8_t i = 0; i < tft_entity.corner_radius; i++) // multiple lines
  {
    TFT[0]->setCursor(TFT[0]->getCursorX() + shiftxy, TFT[0]->getCursorY() + shiftxy);
    TFT[0]->println(txt[i]);
  }
}
void MessageTFT::updateTXT(const char *txt)
{
  _put_text(_txt_buf,tft_entity.face_color); // faster to change color txt to face color, than redraw (mostly for clks)
  _put_text(txt);
}
void MessageTFT::clear_screen(uint8_t c)
{
  if (c == 0)
  {
    TFT[0]->fillScreen(ILI9341_BLACK);
  }
  else if (c == 1)
  {
    TFT[0]->fillScreen(ILI9341_YELLOW);
  }
  else if (c == 2)
  {
    TFT[0]->fillScreen(ILI9341_BLUE);
  }
}
void MessageTFT::_drawFace()
{
  if (tft_entity.roundRect == false)
  {
    TFT[0]->fillRect(tft_entity.w_pos - tft_entity.w / 2, tft_entity.h_pos - tft_entity.h / 2,
                     tft_entity.w, tft_entity.h, tft_entity.face_color);
  }
  else
  {
    TFT[0]->fillRoundRect(tft_entity.w_pos - tft_entity.w / 2, tft_entity.h_pos - tft_entity.h / 2,
                          tft_entity.w, tft_entity.h, tft_entity.corner_radius, tft_entity.face_color);
  }

  if (tft_entity.face_color != tft_entity.border_color && tft_entity.useBorder == true)
  {
    _drawBorder();
  }
}
void MessageTFT::_drawBorder()
{
  for (uint8_t t = 0; t < tft_entity.border_thickness; t++)
  {
    if (tft_entity.roundRect == false)
    {
      TFT[0]->drawRect(tft_entity.w_pos - tft_entity.w / 2 - t, tft_entity.h_pos - tft_entity.h / 2 - t,
                       tft_entity.w + 2 * t, tft_entity.h + 2 * t, tft_entity.border_color); /* how to change border direction ?*/
    }
    else
    {
      TFT[0]->drawRoundRect((tft_entity.w_pos - tft_entity.w / 2) - t, (tft_entity.h_pos - tft_entity.h / 2) - t,
                            tft_entity.w + 2 * t, tft_entity.h + 2 * t, tft_entity.corner_radius + t, tft_entity.border_color);
    }
  }
}
void MessageTFT::_put_text(const char *txt, uint16_t color)
{
  strcpy(_txt_buf, txt);
  uint8_t x = strlen(_txt_buf);
  if (tft_entity.center_txt)
  {
    TFT[0]->setCursor(tft_entity.w_pos - x * _pos_corr_factor_x * tft_entity.txt_size, tft_entity.h_pos - _pos_corr_factor_y * tft_entity.txt_size);
  }
  else
  {
    TFT[0]->setCursor(tft_entity.w_pos - tft_entity.w / 2, tft_entity.h_pos - tft_entity.h / 2);
  }

  if (color != 0)
  {
    TFT[0]->setTextColor(tft_entity.txt_color);
  }
  else
  {
    TFT[0]->setTextColor(color);
  }
  // TFT[0]->setTextColor(tft_entity.txt_color);
  TFT[0]->setTextSize(tft_entity.txt_size);
  TFT[0]->print(_txt_buf);
}

ButtonTFT::ButtonTFT(XPT2046_Touchscreen &_ts, Adafruit_ILI9341 &_tft)
{
  TFT[0] = &_tft;
  TS[0] = &_ts;
}
void ButtonTFT::createButton(const char *txt, bool center_txt, uint8_t _radius)
{
  createMSG(txt);
}
bool ButtonTFT::wait4press() /* include getPoint loop - use for simple cases*/
{
  if (TS[0]->touched())
  {
    TS_Point p = TS[0]->getPoint();
    return checkPress(p); /* in or out ? */
  }
  else
  {
    return 0;
  }
}
bool ButtonTFT::checkPress(TS_Point &p) /* can be called from code outside lib when lots of buttons involved */
{
  if (_check_press_geometry(p))
  {
    if (latchButton)
    {
      _latchState = !_latchState;
    }
    _press_cb();
    return 1;
  }
  else
  {
    return 0;
  }
}
bool ButtonTFT::get_buttonState()
{
  return _latchState;
}
void ButtonTFT::_press_cb()
{
  int _press_del = 500; // slow down re-triggerring
  if (latchButton == false)
  {
    _face_color_t = face_color;
    face_color = pressedColor;
    createMSG(_txt_buf);
    delay(_press_del);
    face_color = _face_color_t;
    createMSG(_txt_buf);
    delay(_press_del);
  }
  else
  {
    if (_latchState == true) /* Pressed ON*/
    {
      _face_color_t = face_color;
      face_color = pressedColor;
      createMSG(_txt_buf);
      delay(_press_del);
    }
    else /* Pressed Off*/
    {
      face_color = _face_color_t;
      createMSG(_txt_buf);
      delay(_press_del);
    }
  }
}
bool ButtonTFT::_check_press_geometry(TS_Point &p)
{
  _conv_ts_tft(p);
  return ((_tft_x <= tft_entity.h_pos + tft_entity.h / 2 && _tft_x >= tft_entity.h_pos - tft_entity.h / 2) && (_tft_y <= tft_entity.w_pos + tft_entity.w / 2 && _tft_y >= tft_entity.w_pos - tft_entity.w / 2));
}
void ButtonTFT::_conv_ts_tft(TS_Point &p)
{
  if (TFT[0]->getRotation() == 1 || TFT[0]->getRotation() == 3)
  {
    _tft_x = _TS2TFT_x(p.x);
    _tft_y = _TS2TFT_y(p.y);
  }
  else
  {
    _tft_y = _TS2TFT_x(p.x);
    _tft_x = _TS2TFT_y(p.y);
  }
}
int ButtonTFT::_TS2TFT_x(int px)
{
  if (TFT[0]->getRotation() == 0)
  {
    return map(px, TS_MAX_X, TS_MIN_X, 0, tft.height());
  }
  else if (TFT[0]->getRotation() == 1)
  {
    return map(px, TS_MAX_X, TS_MIN_X, 0, tft.width());
  }
  else if (TFT[0]->getRotation() == 2)
  {
    return map(px, TS_MIN_X, TS_MAX_X, 0, tft.height());
  }
  else if (TFT[0]->getRotation() == 3)
  {
    return map(px, TS_MIN_X, TS_MAX_X, 0, tft.width());
  }
  else
  {
    return 9999;
  }
}
int ButtonTFT::_TS2TFT_y(int py)
{
  if (TFT[0]->getRotation() == 0)
  {
    return map(py, TS_MAX_Y, TS_MIN_Y, 0, tft.width());
  }
  else if (TFT[0]->getRotation() == 1)
  {
    return map(py, TS_MIN_Y, TS_MAX_Y, 0, tft.height());
  }
  else if (TFT[0]->getRotation() == 2)
  {
    return map(py, TS_MIN_Y, TS_MAX_Y, 0, tft.width());
  }
  else if (TFT[0]->getRotation() == 3)
  {
    return map(py, TS_MAX_Y, TS_MIN_Y, 0, tft.height());
  }
  else
  {
    return 99999999;
  }
}

keypadTFT::keypadTFT(XPT2046_Touchscreen &_ts, Adafruit_ILI9341 &_tft)
{
  _butarray.butarray[0].TFT[0] = &_tft;
  _butarray.butarray[0].TS[0] = &_ts;
}
void keypadTFT::create_keypad()
{
  const char *txt_buttons[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "*", "0", "#"};
  _butarray.create_array(4, 3, txt_buttons);
}
bool keypadTFT::getPasscode(TS_Point &p)
{
  static unsigned long last_touch = 0;
  if (millis() - last_touch > 500)
  {
    if (millis() - last_touch > RESET_KEYPAD_TIMEOUT * 1000 && strcmp(_stored_keypad_value, "") != 0) /* Clear buffer after timeout */
    {
      _reset_keypad_values();
    }
    last_touch = millis();
    return _check_pressed_in(p); /* true only when passcode is delivered ( not only pressed ) */
  }
  else
  {
    return false;
  }
}
void keypadTFT::_reset_keypad_values()
{
  strcpy(_stored_keypad_value, "");
  counter = 0;
}
bool keypadTFT::_check_pressed_in(TS_Point &p)
{
  uint8_t i = _butarray.checkPress(p);
  if (i != 99)
  {
    if (i == 9) /* Erase buffer */
    {
      _reset_keypad_values();
      delay(100);
      return false;
    }
    else if (i == 11) /* Send passcode */
    {
      if (strcmp(_stored_keypad_value, "") != 0)
      {
        strcpy(keypad_value, _stored_keypad_value);
        _reset_keypad_values();
        return true;
      }
      else
      {
        return false;
      }
    }
    else
    {
      // strcat(_stored_keypad_value, _butarray.butarray[i]._txt_buf); // NEED TO BE FIXED SINCE IT IS PROTECTED NOW //
      counter = strlen(_stored_keypad_value);
      return false;
    }
  }
  else
  {
    return false;
  }
}
