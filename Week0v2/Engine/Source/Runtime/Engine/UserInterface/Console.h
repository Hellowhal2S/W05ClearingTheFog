#pragma once
#include "ImGUI/imgui.h"
#include "Define.h"
#include "PropertyEditor/IWindowToggleable.h"
#include <windows.h>
#include <psapi.h>
enum class LogLevel { Display, Warning, Error };
class StatOverlay {
public:
    bool showFPS = false;
    bool showMemory = false;
    bool showRender = false;
    void ToggleStat(const std::string& command) {
        if (command == "stat fps") {showFPS = true; showRender = true;}
        else if (command == "stat memory") {showMemory = true; showRender = true;}
        else if (command == "stat none") {
            showFPS = false;
            showMemory = false;
            showRender = false;
        }
    }

    void Render(ID3D11DeviceContext* context, UINT width, UINT height) {

        if (!showRender)
            return;
        ImVec2 displaySize = ImGui::GetIO().DisplaySize;
        // 창 크기를 화면의 50%로 설정합니다.
        ImVec2 windowSize(displaySize.x * 0.5f, displaySize.y * 0.5f);
        // 창을 중앙에 배치하기 위해 위치를 계산합니다.
        ImVec2 windowPos((displaySize.x - windowSize.x) * 0.5f, (displaySize.y - windowSize.y) * 0.5f);

    
        ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
        ImGui::Begin("Stat Overlay", nullptr,
                     ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoScrollbar);
        if (showFPS) {
            static float lastTime = ImGui::GetTime();
            static int frameCount = 0;
            static float fps = 0.0f;

            frameCount++;
            float currentTime = ImGui::GetTime();
            float deltaTime = currentTime - lastTime;

            if (deltaTime >= 1.0f) { // 1초마다 FPS 업데이트
                fps = frameCount / deltaTime;
                frameCount = 0;
                lastTime = currentTime;
            }
            ImGui::Text("FPS: %.2f", fps);
        }


        if (showMemory)
        {
            ImGui::Text("Allocated Object Count: %llu", FPlatformMemory::GetAllocationCount<EAT_Object>());
            ImGui::Text("Allocated Object Memory: %llu B", FPlatformMemory::GetAllocationBytes<EAT_Object>());
            ImGui::Text("Allocated Container Count: %llu", FPlatformMemory::GetAllocationCount<EAT_Container>());
            ImGui::Text("Allocated Container memory: %llu B", FPlatformMemory::GetAllocationBytes<EAT_Container>());
        }
        ImGui::PopStyleColor();
        ImGui::End();
    }

private:
    float CalculateFPS() {
        static int frameCount = 0;
        static float elapsedTime = 0.0f;
        static float lastTime = 0.0f;

        float currentTime = GetTickCount64() / 1000.0f;
        elapsedTime += (currentTime - lastTime);
        lastTime = currentTime;
        frameCount++;

        if (elapsedTime > 1.0f) {
            float fps = frameCount / elapsedTime;
            frameCount = 0;
            elapsedTime = 0.0f;
            return fps;
        }
        return 0.0f;
    }

    void DrawTextOverlay(const std::string& text, int x, int y) {
        // ImGui 사용 시
        ImGui::SetNextWindowPos(ImVec2(x, y));
        ImGui::Text("%s", text.c_str());
    }
};
class Console : public IWindowToggleable
{
private:
    Console();
    ~Console();
public:
    static Console& GetInstance(); // 참조 반환으로 변경

    void Clear();
    void AddLog(LogLevel level, const char* fmt, ...);
    void Draw();
    void ExecuteCommand(const std::string& command);
    void OnResize(HWND hWnd);
    void Toggle() override { 
        if (bWasOpen) {
            bWasOpen = false;
        }
        else {
            bWasOpen = true;
        }
    } // Toggle() 구현 

public:
    struct LogEntry {
        LogLevel level;
        FString message;
    };

    TArray<LogEntry> items;
    TArray<FString> history;
    int32 historyPos = -1;
    char inputBuf[256] = "";
    bool scrollToBottom = false;

    ImGuiTextFilter filter;  // 필터링을 위한 ImGuiTextFilter

    // 추가된 멤버 변수들
    bool showLogTemp = true;   // LogTemp 체크박스
    bool showWarning = true;   // Warning 체크박스
    bool showError = true;     // Error 체크박스

    bool bWasOpen = true;
    bool showFPS = false;
    bool showMemory = false;
    // 복사 방지
    Console(const Console&) = delete;
    Console& operator=(const Console&) = delete;
    Console(Console&&) = delete;
    Console& operator=(Console&&) = delete;

    StatOverlay overlay;
private:
    bool bExpand = true;
    UINT width;
    UINT height;

};
