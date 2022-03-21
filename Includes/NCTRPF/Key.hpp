#pragma once

#include <3ds.h>

class Key {
public:
  // Buttons
  static void Update();
  static bool IsDown(u32 keys);
  static bool IsPressed(u32 keys);
  static bool IsReleased(u32 keys);
  static u32 GetKeysDown();

  // Touch screen
  static touchPosition ReadTouch();
  static bool IsRectTouched(u16 posX, u16 posY, u16 width, u16 height);
};