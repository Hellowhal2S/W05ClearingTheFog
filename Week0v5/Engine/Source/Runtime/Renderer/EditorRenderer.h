#pragma once

#define _TCHAR_DEFINED
#include <d3d11.h>
#include "Math/Vector.h"
#include "Math/Vector4.h"
#include "Math/Matrix.h"
#include "Container/Set.h"
#include "RenderResources.h"

class FGraphicsDevice;
class UWorld;
class FEditorViewportClient;
class FRenderer;

class FEditorRenderer
{
public:
    void Initialize(FRenderer* InRenderer);
    void Render(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport);
    void Release();

private:
    FRenderer* Renderer;
    FRenderResourcesDebug Resources;

    void CreateShaders();
    void PrepareShader(FShaderResource ShaderResource) const;
    void ReleaseShaders();

    void CreateBuffers();
    void CreateConstantBuffers();

    void LazyLoad();

    void PrepareRendertarget();
    void PrepareComponents(UWorld* World);

    void PrepareConstantbufferGlobal();
    void UpdateConstantbufferGlobal(FConstantBufferCamera Buffer);

    // Gizmo 관련 함수
    void RenderGizmos(const UWorld* World);
    void PrepareShaderGizmo();
    void PrepareConstantbufferGizmo();

    // Axis
    void RenderAxis();

    // AABB
    void RenderAABBInstanced(const UWorld* World);
    void PrepareConstantbufferAABB();
    void UdpateConstantbufferAABBInstanced(TArray<FConstantBufferDebugAABB> Buffer);

    // Sphere
    void RenderPointlightInstanced(const UWorld* World);
    void PrepareConstantbufferPointlight();
    void UdpateConstantbufferPointlightInstanced(TArray<FConstantBufferDebugSphere> Buffer);

    // Cone
    void RenderSpotlightInstanced(const UWorld* World);
    void PrepareConstantbufferSpotlight();
    void UdpateConstantbufferSpotlightInstanced(TArray<FConstantBufferDebugCone> Buffer);

    // Grid
    void RenderGrid(std::shared_ptr<FEditorViewportClient> ActiveViewport);
    void PrepareConstantbufferGrid();
    void UdpateConstantbufferGrid(FConstantBufferDebugGrid Buffer);

    // Icon
    void RenderIcons(const UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport);
    void PrepareConstantbufferIcon();
    void UdpateConstantbufferIcon(FConstantBufferDebugIcon Buffer);
    void UpdateTextureIcon(IconType type);

    // Arrow
    void RenderArrows(const UWorld* World);
    void PrepareConstantbufferArrow();
    void UdpateConstantbufferArrow(FConstantBufferDebugArrow Buffer);



    const UINT32 ConstantBufferSizeAABB = 8;
    const UINT32 ConstantBufferSizeSphere = 8;
    const UINT32 ConstantBufferSizeCone = 8;
};

