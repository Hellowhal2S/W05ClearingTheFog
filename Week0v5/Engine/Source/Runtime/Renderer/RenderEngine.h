#pragma once
#include <thread>
#include <condition_variable>
#include "Renderer.h"
#include "EditorRenderer.h"

class FGraphicsDevice;
class FRenderer;
class FEditorRenderer;

class FRenderEngine
{
public:
    FRenderEngine() = default;
    ~FRenderEngine() = default;

    void Initialize(FGraphicsDevice* graphics);

    void Start();
    void Render(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport);
    void RenderDebug(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport);
    std::condition_variable IsMainReady;
    std::condition_variable IsRenderReady;
    std::condition_variable IsMainStop;
    std::condition_variable IsRenderStop;
    
    std::thread RenderThread;
    FRenderer Renderer;
    FEditorRenderer RenderDebugger;
private:
};

