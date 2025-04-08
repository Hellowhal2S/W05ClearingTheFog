#include "RenderEngine.h"


void FRenderEngine::Initialize(FGraphicsDevice* graphics)
{
    Renderer.Initialize(graphics);
    RenderDebugger.Initialize(&Renderer);
}

void FRenderEngine::Render(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    Renderer.Render(World, ActiveViewport);
}

void FRenderEngine::RenderDebug(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    // 앞에서 depth test 꺼주는듯...
    RenderDebugger.Render(World, ActiveViewport);
}

