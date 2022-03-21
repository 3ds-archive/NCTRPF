#include "NCTRPF/Key.hpp"

void Key::Update()
{
  hidScanInput();
}

bool Key::IsDown(u32 keys)
{
  return ((hidKeysHeld() & keys) == keys);
}

bool Key::IsPressed(u32 keys)
{
  return ((hidKeysDown() & keys) == keys);
}

bool Key::IsReleased(u32 keys)
{
  return ((hidKeysUp() & keys) == keys);
}

u32 Key::GetKeysDown()
{
  return hidKeysHeld();
}

touchPosition Key::ReadTouch()
{
  touchPosition tPos;
  hidTouchRead(&tPos);
  return tPos;
}

bool Key::IsRectTouched(u16 posX, u16 posY, u16 width, u16 height)
{
  touchPosition tPos = ReadTouch();
  return ((posX <= tPos.px && tPos.px <= posX + width) 
       && (posY <= tPos.py && tPos.py <= posY + height));
}
