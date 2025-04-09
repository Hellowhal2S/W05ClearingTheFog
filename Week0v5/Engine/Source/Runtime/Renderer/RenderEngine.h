#pragma once
#include <thread>
#include <condition_variable>
#include "Renderer.h"
#include "EditorRenderer.h"
#include "Engine/Source/Runtime/Windows/D3D11RHI/GraphicDevice.h"

class FGraphicsDevice;
class FRenderer;
class FEditorRenderer;

class FRenderEngine
{
public:
    FRenderEngine() = default;
    ~FRenderEngine() = default;

    void Start();

    void Initialize(FGraphicsDevice* graphics);
    void Render(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport);
    void RenderDebug(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport);
    std::condition_variable IsMainReady;
    std::condition_variable IsRenderReady;
    std::condition_variable IsMainStop;
    std::condition_variable IsRenderStop;
    
    FRenderer* GetRenderer() { return &Renderer; }
    FGraphicsDevice* GetGraphicsDevice() { return &graphicDevice; }

private:
    std::thread RenderThread;
    FRenderer Renderer;
    FEditorRenderer RenderDebugger;
    FGraphicsDevice graphicDevice;
};

