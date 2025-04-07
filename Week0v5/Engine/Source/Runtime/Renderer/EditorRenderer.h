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

    void PreparePrimitives();

    void PrepareConstantbufferGlobal();
    void UpdateConstantbufferGlobal(FConstantBufferCamera Buffer);
    // Gizmo 관련 함수
    void RenderGizmos(const UWorld* World);
    void PrepareShaderGizmo();
    void PrepareConstantbufferGizmo();

    // Axis
    void RenderAxis(const std::shared_ptr<FEditorViewportClient>& ActiveViewport);
    void PrepareShaderAxis();

    // AABB
    void RenderAABBInstanced(const UWorld* World);
    void PrepareConstantbufferAABB();
    void UdpateConstantbufferAABBInstanced(TArray<FConstantBufferDebugAABB> Buffer);


    const UINT32 ConstantBufferSizeAABB = 8;
};

