#pragma once

#include <3ds.h>
#include <string>

#define SPACING_Y 11
#define SPACING_X 6

class Screen {
public:
  static void Initialize();
  static void Exit();

  static Screen &GetTop();
  static Screen &GetBottom();

  void Lock();
  void Unlock();

  u8 *GetFramebuffer(u16 posX, u16 posY);

  void DrawPixel(u16 posX, u16 posY, u32 color);
  void DrawRect(u16 posX, u16 posY, u16 width, u16 height, u32 color, bool fill = true);
  void DrawLine(u16 posX1, u16 posY1, u16 posX2, u16 posY2, u32 color);
  u32 DrawString(const std::string &str, u32 posX, u32 posY, u32 foreground = 0xFFFFFFFF, u32 background = 0);

  void Clear();
  void Update();
  void Flash(u32 color);

private:
  bool _isTop;
  u32 _infoBase;
  u16 _width;
  u8 *_framebuffer[2];
  u8 _usingFramebuf;
  u32 _stride;
  u8 _bytesPerPixel;
  GSPGPU_FramebufferFormat _format;
  RecursiveLock _lock;

  Screen(bool isTop);
  ~Screen();
  void _drawPixel(u8 *framebuf, u32 color);
  void _drawCharacter(char character, u16 posX, u16 posY, u32 foreground, u32 background);

  static constexpr u16 Height = 240;
  static constexpr u32 TopInfoBase = 0x10400400;
  static constexpr u32 BottomInfoBase = 0x10400500;
  static constexpr u32 Framebuf0Offset = 0x68;
  static constexpr u32 Framebuf1Offset = 0x6C;
  static constexpr u32 FormatOffset = 0x70;
  static constexpr u32 NextFramebuffer = 0x78;
  static constexpr u32 StrideOffset = 0x90;
};