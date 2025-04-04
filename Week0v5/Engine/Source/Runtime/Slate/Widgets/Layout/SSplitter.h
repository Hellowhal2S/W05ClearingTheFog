#pragma once
#include "SlateCore/Widgets/SWindow.h"
#include "Container/Map.h"
#include "fstream"
#include "sstream"
class SSplitter : public SWindow
{
public:
    SWindow* SideLT; // Left or Top
    SWindow* SideRB; // Right or Bottom
    
    virtual void Initialize(FRect initRect);

    virtual void OnDragStart(const FPoint& mousePos) { /* 초기화 */ }
    virtual void OnDrag(const FPoint& delta) = 0; // 가로/세로에 따라 구현 다름.
    virtual void OnResize(float width, float height);
    virtual bool OnPressed(FPoint coord);
    virtual bool OnReleased();

    virtual void OnDragEnd() {

    }

    virtual void LoadConfig(const TMap<FString, FString>& config);
    virtual void SaveConfig(TMap<FString, FString>& config) const;

    // 스플리터가 포함된 영역에 따라 자식 창의 Rect를 재계산하는 함수
    virtual void UpdateChildRects() = 0;

    template <typename T>
    T GetValueFromConfig(const TMap<FString, FString>& config, const FString& key, T defaultValue) {
        if (const FString* Value = config.Find(key))
        {
            std::istringstream iss(**Value);
            T value;
            if (iss >> value)
            {
                return value;
            }
        }
        return defaultValue;
    }
};

class SSplitterH : public SSplitter
{
public:
    virtual void Initialize(FRect initRect) override;
    virtual void OnResize(float width, float height) override;

    virtual void LoadConfig(const TMap<FString, FString>& config) override;
    virtual void SaveConfig(TMap<FString, FString>& config) const override;
    void OnDrag(const FPoint& delta) override {
        // 수평 스플리터의 경우, 좌우로 이동
        Rect.leftTopX += delta.x;

        UpdateChildRects();
    }

    void UpdateChildRects() override {

        if (SideLT)
            SideLT->Rect.width = Rect.leftTopX - SideLT->Rect.leftTopX;
        if (SideRB)
        {
            float prevleftTopX = SideRB->Rect.leftTopX;
            SideRB->Rect.leftTopX = Rect.leftTopX + Rect.width;
            SideRB->Rect.width =  SideRB->Rect.width + prevleftTopX  - SideRB->Rect.leftTopX;
        }
    }
};

class SSplitterV : public SSplitter
{
public:
    virtual void Initialize(FRect initRect) override;
    virtual void OnResize(float width, float height)    override;

    virtual void LoadConfig(const TMap<FString, FString>& config)   override;
    virtual void SaveConfig(TMap<FString, FString>& config) const   override;
    void OnDrag(const FPoint& delta) override {

        Rect.leftTopY += delta.y;
        UpdateChildRects();
    }
    void UpdateChildRects() override {

        if (SideLT)
            SideLT->Rect.height = Rect.leftTopY - SideLT->Rect.leftTopY;
        if (SideRB)
        {
            float prevleftTopY = SideRB->Rect.leftTopY;
            SideRB->Rect.leftTopY = Rect.leftTopY + Rect.height;
            SideRB->Rect.height = SideRB->Rect.height + prevleftTopY - SideRB->Rect.leftTopY;
        }
    }
};
