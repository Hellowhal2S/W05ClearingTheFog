#include "EditorEngine.h"
#include "ImGuiManager.h"
#include "Engine/World.h"
#include "PropertyEditor/ViewportTypePanel.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UnrealEd/UnrealEd.h"
#include "UnrealClient.h"
#include "Actors/Player.h"
#include "GameFramework/Actor.h"
#include "slate/Widgets/Layout/SSplitter.h"
#include "LevelEditor/SLevelEditor.h"
#include "UnrealEd/SceneMgr.h"

#include "Renderer/PostEffect.h" // 후처리용 : FrameBuffer 안의 내용을 ColorSRV로 복사 후, PostEffect::Render 호출
#include "UnrealEd/PrimitiveBatch.h"


class ULevel;

FGraphicsDevice UEditorEngine::graphicDevice;
FRenderEngine UEditorEngine::RenderEngine;
FResourceMgr UEditorEngine::resourceMgr;

UEditorEngine::UEditorEngine()
    : hWnd(nullptr)
    , UIMgr(nullptr)
    , GWorld(nullptr)
    , LevelEditor(nullptr)
    , UnrealEditor(nullptr)
{
}


int32 UEditorEngine::Init(HWND hwnd)
{
    /* must be initialized before window. */
    hWnd = hwnd;
    graphicDevice.Initialize(hWnd);
    RenderEngine.Initialize(&graphicDevice);
    UIMgr = new UImGuiManager;
    UIMgr->Initialize(hWnd, graphicDevice.Device, graphicDevice.DeviceContext);
    resourceMgr.Initialize(&RenderEngine.Renderer, &graphicDevice);
    
    FWorldContext EditorContext;
    EditorContext.WorldType = EWorldType::Editor;
    EditorContext.thisCurrentWorld = (FObjectFactory::ConstructObject<UWorld>());
    UWorld*  EditWorld  =EditorContext.thisCurrentWorld;
    EditWorld->InitWorld();
    EditWorld->WorldType = EWorldType::Editor;
    GWorld = EditWorld;
    worldContexts.Add(EditorContext);
    
    FWorldContext PIEContext;
    EditorContext.WorldType = EWorldType::PIE;
    worldContexts.Add(PIEContext);
    
    LevelEditor = new SLevelEditor();
    LevelEditor->Initialize();
    
    UnrealEditor = new UnrealEd();
    UnrealEditor->Initialize(LevelEditor);
    
    SceneMgr = new FSceneMgr();

    return 0;
}


void UEditorEngine::Render()
{

    graphicDevice.Prepare();
    if (LevelEditor->IsMultiViewport())
    {
        std::shared_ptr<FEditorViewportClient> viewportClient = GetLevelEditor()->GetActiveViewportClient();
        PostEffect::ClearRTV(graphicDevice.DeviceContext);
        for (int i = 0; i < 4; ++i)
        {
            graphicDevice.DeviceContext->OMSetRenderTargets(6, graphicDevice.RTVs, graphicDevice.DepthStencilView); // 렌더 타겟 설정(백버퍼를 가르킴)
            LevelEditor->SetViewportClient(i);
            RenderEngine.Render(GWorld,LevelEditor->GetActiveViewportClient());
            // graphicDevice.DeviceContext->RSSetViewports(1, &LevelEditor->GetActiveViewportClient()->GetD3DViewport());

            PostEffect::CopyBackBufferToColorSRV(graphicDevice.DeviceContext, graphicDevice.ColorTexture, graphicDevice.FrameBuffer);
            PostEffect::CopyDepthBufferToDepthOnlySRV(graphicDevice.DeviceContext, graphicDevice.DepthStencilBuffer);
            PostEffect::Render(graphicDevice.DeviceContext, graphicDevice.ColorSRV);
        }
        GetLevelEditor()->SetViewportClient(viewportClient);
    }   
    else
    {
        PostEffect::ClearRTV(graphicDevice.DeviceContext);
        graphicDevice.DeviceContext->OMSetRenderTargets(6, graphicDevice.RTVs, graphicDevice.DepthStencilView); // 렌더 타겟 설정(백버퍼를 가르킴)
        RenderEngine.Render(GWorld,LevelEditor->GetActiveViewportClient());
        PostEffect::CopyBackBufferToColorSRV(graphicDevice.DeviceContext, graphicDevice.ColorTexture, graphicDevice.FrameBuffer);
        PostEffect::CopyDepthBufferToDepthOnlySRV(graphicDevice.DeviceContext, graphicDevice.DepthStencilBuffer);
        PostEffect::Render(graphicDevice.DeviceContext, graphicDevice.ColorSRV);
    }

    // OMSEtrender - depth용 pass


    // 화면에 그려진 백버퍼의 내용을 SRV로 쓰기 위해 ColorTexture에 복사

    if (GWorld->WorldType == EWorldType::Editor)
    {
        if (LevelEditor->IsMultiViewport())
        {
            std::shared_ptr<FEditorViewportClient> viewportClient = GetLevelEditor()->GetActiveViewportClient();
            for (int i = 0; i < 4; ++i)
            {
                LevelEditor->SetViewportClient(i);
                RenderEngine.RenderDebug(GWorld, LevelEditor->GetActiveViewportClient());

            }
            GetLevelEditor()->SetViewportClient(viewportClient);
        }
        else
        {
            RenderEngine.RenderDebug(GWorld, LevelEditor->GetActiveViewportClient());
        }
    }

    //graphicDevice.DeviceContext->OMSetRenderTargets(1, &graphicDevice.FrameBufferRTV, nullptr);
    //graphicDevice.DeviceContext->CopyResource(graphicDevice.FrameBuffer, PostEffect::finalTexture);
    // PostEffect::Render(ColorSRV, DepthOnlySRV, WorldPosSRV)
}

void UEditorEngine::Tick(float deltaSeconds)
{
    // for (FWorldContext& WorldContext : worldContexts)
    // {
    //     std::shared_ptr<UWorld> EditorWorld = WorldContext.World();
    //     // GWorld = EditorWorld;
    //     // GWorld->Tick(levelType, deltaSeconds);
    //     if (EditorWorld && WorldContext.WorldType == EWorldType::Editor)
    //     {
    //         // GWorld = EditorWorld;
    //         GWorld->Tick(LEVELTICK_ViewportsOnly, deltaSeconds);
    //     }
    //     else if (EditorWorld && WorldContext.WorldType == EWorldType::PIE)
    //     {
    //         // GWorld = EditorWorld;
    //         GWorld->Tick(LEVELTICK_All, deltaSeconds);
    //     }
    // }
    GWorld->Tick(levelType, deltaSeconds);
    Input();
    // GWorld->Tick(LEVELTICK_All, deltaSeconds);
    LevelEditor->Tick(deltaSeconds);
    Render();
    UIMgr->BeginFrame();
    UnrealEditor->Render();

    Console::GetInstance().Draw();

    UIMgr->EndFrame();

    // Pending 처리된 오브젝트 제거

    // TODO : 이거 잘 안되는 것 이유 파악 
    // GUObjectArray.ProcessPendingDestroyObjects();

    graphicDevice.SwapBuffer();
}

float UEditorEngine::GetAspectRatio(IDXGISwapChain* swapChain) const
{
    DXGI_SWAP_CHAIN_DESC desc;
    swapChain->GetDesc(&desc);
    return static_cast<float>(desc.BufferDesc.Width) / static_cast<float>(desc.BufferDesc.Height);
}

void UEditorEngine::Input()
{
    if (GetAsyncKeyState('M') & 0x8000)
    {
        if (!bTestInput)
        {
            bTestInput = true;
            if (LevelEditor->IsMultiViewport())
            {
                LevelEditor->OffMultiViewport();
            }
            else
                LevelEditor->OnMultiViewport();
        }
    }
    else
    {
        bTestInput = false;
    }
}


void UEditorEngine::PreparePIE()
{
    // 1. World 복제
    worldContexts[1].thisCurrentWorld = (Cast<UWorld>(GWorld->Duplicate()));
    GWorld = worldContexts[1].thisCurrentWorld;
    GWorld->WorldType = EWorldType::PIE;
    levelType = LEVELTICK_All;
    
}

void UEditorEngine::StartPIE()
{
    // 1. BeingPlay() 호출
    GWorld->BeginPlay();
    levelType = LEVELTICK_All;
    UE_LOG(LogLevel::Error, "Start PIE");
}

void UEditorEngine::PausedPIE()
{
    if (levelType == LEVELTICK_All)
        levelType = LEVELTICK_PauseTick;
    else if (levelType == LEVELTICK_PauseTick)
        levelType = LEVELTICK_All;
    UE_LOG(LogLevel::Error, "Puase PIE");
}

void UEditorEngine::ResumingPIE()
{

}

void UEditorEngine::StopPIE()
{
    // 1. World Clear
    GWorld = worldContexts[0].thisCurrentWorld;

    for (auto iter : worldContexts[1].World()->GetActors())
    {
        iter->Destroy();
        GUObjectArray.MarkRemoveObject(iter);
    }
    GUObjectArray.MarkRemoveObject(worldContexts[1].World()->GetLevel());
    worldContexts[1].World()->GetEditorPlayer()->Destroy();
    GUObjectArray.MarkRemoveObject( worldContexts[1].World()->GetWorld());
    worldContexts[1].thisCurrentWorld = nullptr;
    
    // GWorld->WorldType = EWorldType::Editor;
    levelType = LEVELTICK_ViewportsOnly;
    // if (GWorld && GWorld->IsPIEWorld())
    // {
    //     GWorld->ClearScene();
    // }
    //
    // GWorld = GetEditorWorldContext()->World();
}

void UEditorEngine::Exit()
{
    LevelEditor->Release();
    GWorld->Release();
    UIMgr->Shutdown();
    delete UIMgr;
    delete SceneMgr;
    resourceMgr.Release(&RenderEngine.Renderer);
    RenderEngine.Renderer.Release();
    graphicDevice.Release();
}


