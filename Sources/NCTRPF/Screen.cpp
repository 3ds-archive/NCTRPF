#include <string.h>

#include "common.h"
#include "LinuxFont.h"
#include "NCTRPF/Screen.hpp"

namespace Global {
  Screen *topScreen;
  Screen *bottomScreen;
}

void Screen::Initialize()
{
  Global::topScreen = new Screen(true);
  Global::bottomScreen = new Screen(false);
}

void Screen::Exit()
{
  delete Global::topScreen;
  delete Global::bottomScreen;
}

Screen::Screen(bool isTop): _isTop(isTop)
{
  _infoBase = isTop ? TopInfoBase : BottomInfoBase;
  _width = isTop ? 400 : 320;

  _framebuffer[0] = (u8*)PA_PTR(REG32(_infoBase + Framebuf0Offset));
  _framebuffer[1] = (u8*)PA_PTR(REG32(_infoBase + Framebuf1Offset));
  _usingFramebuf = (REG32(_infoBase + 0x78) & 1);
  _format = (GSPGPU_FramebufferFormat)(REG32(_infoBase + FormatOffset) & 7);
  _stride = REG32(_infoBase + StrideOffset);
  _bytesPerPixel = gspGetBytesPerPixel(_format);
  RecursiveLock_Init(&_lock);
}

Screen::~Screen() {}

Screen &Screen::GetTop()
{
  Global::topScreen->Update();
  return *Global::topScreen;
}

Screen &Screen::GetBottom()
{
  Global::bottomScreen->Update();
  return *Global::bottomScreen;
}

void Screen::Lock()
{
  RecursiveLock_Lock(&_lock);
}

void Screen::Unlock()
{
  RecursiveLock_Unlock(&_lock);
}

void Screen::Update()
{
  _framebuffer[0] = (u8*)PA_PTR(REG32(_infoBase + Framebuf0Offset));
  _framebuffer[1] = (u8*)PA_PTR(REG32(_infoBase + Framebuf1Offset));
  _usingFramebuf = (REG32(_infoBase + NextFramebuffer) & 1);
  _format = (GSPGPU_FramebufferFormat)(REG32(_infoBase + FormatOffset) & 7);
  _stride = REG32(_infoBase + StrideOffset);
  _bytesPerPixel = gspGetBytesPerPixel(_format);
}

void Screen::Flash(u32 color)
{
  u32 addr = _isTop ? 0x10202204 : 0x10202A04;
  color |= 0x01000000;
  for (u32 i = 0; i < 64; i++)
  {
    REG32(addr) = color;
    svcSleepThread(5000000);
  }
  REG32(addr) = 0;
}

u8 *Screen::GetFramebuffer(u16 posX, u16 posY)
{
  return (u8*)((u32)_framebuffer[_usingFramebuf] + _bytesPerPixel * (239 - posY + (_stride / _bytesPerPixel) * posX));
}

void Screen::Clear()
{
  memset(_framebuffer[0], 0, _width * Height * _bytesPerPixel);
  memset(_framebuffer[1], 0, _width * Height * _bytesPerPixel);
}

void Screen::_drawPixel(u8 *framebuf, u32 color)
{
  u8 *colorPtr = (u8*)&color;

  switch(this->_format)
  {
    case GSP_RGBA8_OES:
    {
      framebuf[0] = colorPtr[0];
      framebuf[1] = colorPtr[1];
      framebuf[2] = colorPtr[2];
      framebuf[3] = colorPtr[3];
      break;
    }
    case GSP_BGR8_OES:
    {
      framebuf[0] = colorPtr[2];
      framebuf[1] = colorPtr[1];
      framebuf[2] = colorPtr[0];
      break;
    }

    case GSP_RGB565_OES:
    {
      u16* fb = (u16*)framebuf;
      fb[0] = (u16) ( (colorPtr[2] >> 3) & 0x1f ) | ( ((colorPtr[1] >> 2) & 0x3f) << 5 ) | ( ((colorPtr[0] >> 3) & 0x1f) << 11 );
      break;
    }

    case GSP_RGB5_A1_OES:
    {
      u16* fb = (u16*)framebuf;
      fb[0] = (u16) ( (colorPtr[2] >> 3) & 0x1f ) | ( ((colorPtr[1] >> 3) & 0x1f) << 5 ) | ( ((colorPtr[0] >> 3) & 0x1f) << 10 );
      break;
    }

    case GSP_RGBA4_OES:
    {
      framebuf[0] = (u8)( (colorPtr[3] >> 4) & 0xf ) | (u8)( (colorPtr[2] >> 4) & 0xf ) << 4;
      framebuf[1] = (u8)( (colorPtr[1] >> 4) & 0xf ) | (u8)( (colorPtr[0] >> 4) & 0xf ) << 4;
      break;
    }
    default: break;
  }
}

void Screen::DrawPixel(u16 posX, u16 posY, u32 color)
{
  if(posX <= _width && posY <= Height)
  {
    _drawPixel(GetFramebuffer(posX, posY), color);
  }
}

void Screen::DrawRect(u16 posX, u16 posY, u16 width, u16 height, u32 color, bool fill) 
{
  for(u16 x = 0; x <= width; x++)
  {
    for(u16 y = 0; y <= height; y++)
    {
      if(fill || x == 0 || y == 0 || x == width || y == height)
        DrawPixel(posX + x, posY + y, color);
    }
  }
}

void Screen::_drawCharacter(char character, u16 posX, u16 posY, u32 foreground, u32 background)
{
  for(u16 y = 0; y < 10; y++)
  {
    char charPos = Global::LinuxFont[character * 10 + y];
    for(u16 x = 6; x >= 1; x--)
    {
      u32 color = ((charPos >> x) & 1) ? foreground : background;
      DrawPixel(posX + (6 - x), posY + y, color);
    }
  }
}

u32 Screen::DrawString(const std::string &str, u32 posX, u32 posY, u32 foreground, u32 background)
{
  const char *string = str.c_str();
  for(u32 i = 0, x_count = 0; i < ((u32) strlen(string)); i++)
  {
    if(string[i] == '\n')
    {
      posY += SPACING_Y;
      x_count = 0;
      continue;
    }
    _drawCharacter(string[i], posX + x_count * SPACING_X, posY, foreground, background);
    x_count++;
  }
  return posY;
}

/*
void Screen::DrawLine(u16 posX1, u16 posY1, u16 posX2, u16 posY2, u32 color)
{
  u16 dx = posX2 - posX1, dy = posY2 - posY1;

  if (dx == 0)
  {
    for (u8 y = 0; y < dy; ++y)
    {
      DrawPixel(posX1, posY1 + y, color);
    }
    return;
  }

  if (dy == 0)
  {
    for (u8 x = 0; x < dx; ++x)
    {
      DrawPixel(posX1 + x, posY1, color);
    }
    return;
  }

  float slope = (float)dy / dx;

  if (std::abs(slope) <= 1)
  {
    for (u8 x = 0; x < dx; ++x)
    {
      DrawPixel(posX1 + x, posY1 + (u8)(x*slope), color);
    }
  }
  else
    for (u8 y = 0; y < dy; ++y)
    {
      DrawPixel(posX1 + (u8)(y*(1/slope)), posY1 + y, color);
    }
}
*/