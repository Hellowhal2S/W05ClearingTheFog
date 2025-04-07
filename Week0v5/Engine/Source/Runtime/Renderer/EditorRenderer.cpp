#include "EditorRenderer.h"

#include <d3dcompiler.h>
#include "Engine/World.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UObject/UObjectIterator.h"
#include "BaseGizmos/GizmoBaseComponent.h"
#include "D3D11RHI/GraphicDevice.h"
#include "Engine\Classes\Actors\Player.h"
#include "Renderer.h"

void FEditorRenderer::Initialize(FRenderer* InRenderer)
{
    Renderer = InRenderer;
    CreateShaders();
    CreateConstantBuffers();
}

void FEditorRenderer::Release()
{
    ReleaseShaders();
}

void FEditorRenderer::CreateShaders()
{
    ID3DBlob* errorBlob = nullptr;
    ID3DBlob* VertexShaderCSO;
    ID3DBlob* PixelShaderCSO;

    D3DCompileFromFile(L"Shaders/EditorShader.hlsl", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "gizmoVS", "vs_5_0", 0, 0, &VertexShaderCSO, &errorBlob);
    if (errorBlob)
    {
        OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        errorBlob->Release();
    }
    Renderer->Graphics->Device->CreateVertexShader(VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), nullptr, &Resources.Shaders.Gizmo.Vertex);
    
    D3DCompileFromFile(L"Shaders/EditorShader.hlsl", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "gizmoPS", "ps_5_0", 0, 0, &PixelShaderCSO, &errorBlob);

    Renderer->Graphics->Device->CreatePixelShader(PixelShaderCSO->GetBufferPointer(), PixelShaderCSO->GetBufferSize(), nullptr, &Resources.Shaders.Gizmo.Pixel);

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"MATERIAL_INDEX", 0, DXGI_FORMAT_R32_UINT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    Renderer->Graphics->Device->CreateInputLayout(
        layout, ARRAYSIZE(layout), VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), &Resources.Shaders.Gizmo.Layout
    );

    VertexShaderCSO->Release();
    PixelShaderCSO->Release();

    VertexShaderCSO = nullptr;
    PixelShaderCSO = nullptr;

    D3DCompileFromFile(L"Shaders/EditorShader.hlsl", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "axisVS", "vs_5_0", 0, 0, &VertexShaderCSO, &errorBlob);
    if (errorBlob)
    {
        OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        errorBlob->Release();
    }
    Renderer->Graphics->Device->CreateVertexShader(VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), nullptr, &Resources.Shaders.AxisLine.Vertex);

    D3DCompileFromFile(L"Shaders/EditorShader.hlsl", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "axisPS", "ps_5_0", 0, 0, &PixelShaderCSO, &errorBlob);

    Renderer->Graphics->Device->CreatePixelShader(PixelShaderCSO->GetBufferPointer(), PixelShaderCSO->GetBufferSize(), nullptr, &Resources.Shaders.AxisLine.Pixel);
    
    // AxisLine은 layout을 받지 않고, SV_VertexID를 사용합니다.
    Resources.Shaders.AxisLine.Layout = nullptr;
    Resources.Shaders.AxisLine.Topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;

    VertexShaderCSO->Release();
    PixelShaderCSO->Release();

}

void FEditorRenderer::PrepareShader(FShaderResource ShaderResource) const
{
    Renderer->Graphics->DeviceContext->VSSetShader(ShaderResource.Vertex, nullptr, 0);
    Renderer->Graphics->DeviceContext->PSSetShader(ShaderResource.Pixel, nullptr, 0);
    Renderer->Graphics->DeviceContext->IASetInputLayout(ShaderResource.Layout);
    Renderer->Graphics->DeviceContext->IASetPrimitiveTopology(ShaderResource.Topology);
}

void FEditorRenderer::ReleaseShaders()
{

    if (Resources.Shaders.Line.Pixel)
    {
        Resources.Shaders.Line.Pixel->Release();
        Resources.Shaders.Line.Pixel = nullptr;
    }
    if (Resources.Shaders.Line.Vertex)
    {
        Resources.Shaders.Line.Vertex->Release();
        Resources.Shaders.Line.Vertex = nullptr;
    }
    if (Resources.Shaders.Line.Layout)
    {
        Resources.Shaders.Line.Layout->Release();
        Resources.Shaders.Line.Layout = nullptr;
    }
}

void FEditorRenderer::CreateBuffers()
{
    // Box 버퍼 생성
    TArray<FVector> CubeFrameVertices;
    CubeFrameVertices.Add({ -0.5f, -0.5f, -0.5f}); // 0
    CubeFrameVertices.Add({ -0.5f, 0.5f, -0.5f}); // 1
    CubeFrameVertices.Add({ 0.5f, -0.5f, -0.5f}); // 2
    CubeFrameVertices.Add({ 0.5f, 0.5f, -0.5f}); // 3
    CubeFrameVertices.Add({ -0.5f, -0.5f, 0.5f}); // 4
    CubeFrameVertices.Add({ 0.5f, -0.5f, 0.5f}); // 5
    CubeFrameVertices.Add({ -0.5f, 0.5f, 0.5f}); // 6
    CubeFrameVertices.Add({ 0.5f, 0.5f, 0.5f}); // 7

    TArray<uint32> CubeFrameIndices = {
        // Bottom face
        0, 1, 1, 3, 3, 2, 2, 0,
        // Top face
        4, 6, 6, 7, 7, 5, 5, 4,
        // Side faces
        0, 4, 1, 6, 2, 5, 3, 7
    };

    // 버텍스 버퍼 생성
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // will never be updated 
    bufferDesc.ByteWidth = sizeof(FVector) * 8;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = CubeFrameVertices.GetData();

    HRESULT hr = Renderer->Graphics->Device->CreateBuffer(&bufferDesc, &initData, &Resources.Primitives.Box.Vertex);

    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.ByteWidth = sizeof(uint32) * 24;

    initData.pSysMem = CubeFrameIndices.GetData();

    hr = Renderer->Graphics->Device->CreateBuffer(&bufferDesc, &initData, &Resources.Primitives.Box.Index);

    Resources.Primitives.Box.NumVertices = 8;
}

void FEditorRenderer::CreateConstantBuffers()
{
    D3D11_BUFFER_DESC ConstantBufferDesc = {};
    ConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    ConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    ConstantBufferDesc.ByteWidth = sizeof(FConstantBufferCamera);
    Renderer->Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &Resources.ConstantBuffers.Camera00);

    // 초기값은 8개 배열로
    ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    ConstantBufferDesc.ByteWidth = sizeof(FConstantBufferDebugAABB) * AABBMaxNum;
    Renderer->Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &Resources.ConstantBuffers.AABB13);
}

void FEditorRenderer::PrepareConstantbufferGlobal()
{
    if (Resources.ConstantBuffers.Camera00)
    {
        Renderer->Graphics->DeviceContext->VSSetConstantBuffers(0, 1, &Resources.ConstantBuffers.Camera00);
    }
}

void FEditorRenderer::UpdateConstantbufferGlobal(FConstantBufferCamera Buffer)
{
    if (Resources.ConstantBuffers.Camera00)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����

        Renderer->Graphics->DeviceContext->Map(Resources.ConstantBuffers.Camera00, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        memcpy(ConstantBufferMSR.pData, &Buffer, sizeof(Buffer));
        Renderer->Graphics->DeviceContext->Unmap(Resources.ConstantBuffers.Camera00, 0); // GPU�� �ٽ� ��밡���ϰ� �����
    }
}

void FEditorRenderer::Render(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    PrepareConstantbufferGlobal();
    FConstantBufferCamera buf;
    buf.ViewMatrix = ActiveViewport->GetViewMatrix();
    buf.ProjMatrix = ActiveViewport->GetProjectionMatrix();
    UpdateConstantbufferGlobal(buf);

    RenderAxis(ActiveViewport);
    RenderGizmos(World, ActiveViewport);
}

void FEditorRenderer::RenderGizmos(const UWorld* World, const std::shared_ptr<FEditorViewportClient>& ActiveViewport)
{
    PrepareShader(Resources.Shaders.Gizmo);

    //Renderer->PrepareShader(Renderer->RenderResources.Shaders.StaticMesh);
    //Renderer->PrepareShader(Resources.Shaders.Gizmo);

    if (!World->GetSelectedActor())
    {
        return;
    }

    TArray<UGizmoBaseComponent*> GizmoObjs;

    for (const auto iter : TObjectRange<UGizmoBaseComponent>())
    {
        GizmoObjs.Add(iter);
    }

    //  fill solid,  Wirframe 에서도 제대로 렌더링되기 위함
    Renderer->Graphics->DeviceContext->RSSetState(UEditorEngine::graphicDevice.RasterizerStateSOLID);

    for (auto GizmoComp : GizmoObjs)
    {

        if ((GizmoComp->GetGizmoType() == UGizmoBaseComponent::ArrowX ||
            GizmoComp->GetGizmoType() == UGizmoBaseComponent::ArrowY ||
            GizmoComp->GetGizmoType() == UGizmoBaseComponent::ArrowZ)
            && World->GetEditorPlayer()->GetControlMode() != CM_TRANSLATION)
            continue;
        else if ((GizmoComp->GetGizmoType() == UGizmoBaseComponent::ScaleX ||
            GizmoComp->GetGizmoType() == UGizmoBaseComponent::ScaleY ||
            GizmoComp->GetGizmoType() == UGizmoBaseComponent::ScaleZ)
            && World->GetEditorPlayer()->GetControlMode() != CM_SCALE)
            continue;
        else if ((GizmoComp->GetGizmoType() == UGizmoBaseComponent::CircleX ||
            GizmoComp->GetGizmoType() == UGizmoBaseComponent::CircleY ||
            GizmoComp->GetGizmoType() == UGizmoBaseComponent::CircleZ)
            && World->GetEditorPlayer()->GetControlMode() != CM_ROTATION)
            continue;
        FMatrix Model = GizmoComp->GetComponentTransform();
        {
            FConstantBufferActor buf;
            buf.IsSelectedActor = 0;
            buf.UUID = GizmoComp->EncodeUUID() / 255.0f;
            Renderer->UpdateConstantbufferActor(buf);
        }
        {
            FConstantBufferMesh buf;
            buf.ModelMatrix = Model;
            if (GizmoComp == World->GetPickingGizmo())
            {
                buf.IsSelectedMesh = 1;
            }
            else
            {
                buf.IsSelectedMesh = 0;
            }
            Renderer->UpdateConstantbufferMesh(buf);
        }
        // TODO: 기즈모 선택효과 안보임
        // 현재 RenderPrimitive()에서 다시 SelectedMesh를 갱신하는데, 안에서 기즈모를 인식시킬 수 없음.

        if (!GizmoComp->GetStaticMesh()) continue;

        OBJ::FStaticMeshRenderData* renderData = GizmoComp->GetStaticMesh()->GetRenderData();
        if (renderData == nullptr) continue;

        Renderer->RenderPrimitive(Model, renderData, GizmoComp->GetStaticMesh()->GetMaterials(), GizmoComp->GetOverrideMaterials(), -1);

    }

    Renderer->Graphics->DeviceContext->RSSetState(Renderer->Graphics->GetCurrentRasterizer());

#pragma region GizmoDepth
    ID3D11DepthStencilState* originalDepthState = Renderer->Graphics->DepthStencilState;
    Renderer->Graphics->DeviceContext->OMSetDepthStencilState(originalDepthState, 0);
#pragma endregion GizmoDepth
}

void FEditorRenderer::PrepareShaderGizmo()
{
    
}

void FEditorRenderer::PrepareConstantbufferGizmo()
{
}

void FEditorRenderer::RenderAxis(const std::shared_ptr<FEditorViewportClient>& ActiveViewport)
{
    PrepareShader(Resources.Shaders.AxisLine);

    Renderer->Graphics->DeviceContext->Draw(6, 0);
}

void FEditorRenderer::PrepareShaderAxis()
{

}

void FEditorRenderer::RenderAABBInstanced(const UWorld* World, const std::shared_ptr<FEditorViewportClient>& ActiveViewport)
{
    PrepareShaderAABB();
    UdpateConstantbufferAABB();
    PrepareConstantbufferAABB();
}

void FEditorRenderer::PrepareShaderAABB()
{
}

void FEditorRenderer::PrepareConstantbufferAABB()
{
    if (Resources.ConstantBuffers.AABB13)
    {
        Renderer->Graphics->DeviceContext->VSSetConstantBuffers(0, 1, &Resources.ConstantBuffers.AABB13);
    }
}

void FEditorRenderer::UdpateConstantbufferAABBInstanced(TArray<FConstantBufferDebugAABB> Buffer)
{
    // 원래 배열 개수보다 많을 경우 다시 생성
    if (Buffer.Num() > AABBMaxNum)
    {
        AABBMaxNum = (1 + Buffer.Num() / 8) * 8; // 8의 배수로 증가

        // release 후 새로 생성
        Resources.ConstantBuffers.AABB13->Release(); 
        D3D11_BUFFER_DESC ConstantBufferDesc = {};
        ConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        ConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

        // 초기값은 8개 배열로
        ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        ConstantBufferDesc.ByteWidth = sizeof(FConstantBufferDebugAABB) * AABBMaxNum;
        Renderer->Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &Resources.ConstantBuffers.AABB13);
    }

    if (Resources.ConstantBuffers.AABB13)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����

        Renderer->Graphics->DeviceContext->Map(Resources.ConstantBuffers.AABB13, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        memcpy(ConstantBufferMSR.pData, &Buffer, sizeof(FConstantBufferDebugAABB) * Buffer.Num());
        Renderer->Graphics->DeviceContext->Unmap(Resources.ConstantBuffers.AABB13, 0); // GPU�� �ٽ� ��밡���ϰ� �����
    }
}
