#pragma once

#include "Define.h"

class SWindow
{
public:
    SWindow();
    SWindow(FRect initRect);
    virtual ~SWindow();

    virtual void Initialize(FRect initRect);
    virtual void OnResize(float width, float height);

    FRect Rect;
    void SetRect(FRect newRect) { Rect = newRect; }
    bool IsHover(FPoint coord);
    virtual bool OnPressed(FPoint coord);
    virtual bool OnReleased();
    bool IsPressing() { return bIsPressed; }
protected:
    bool bIsHoverd = false;
    bool bIsPressed = false;

};

