#pragma once
#include "Core/Container/Array.h"
#include "ShaderConstants.h"
#include <d3d11.h> // Array.h보다 늦게 선언해야 오류 안뜸 ???

/// <summary>
/// Shader관련 모음.
/// VS, PS, InputLayout
/// 상수 버퍼 관련은 ShaderConstants.h로
/// </summary>
struct FShaderResource
{
    ID3D11VertexShader* Vertex = nullptr;
    ID3D11PixelShader* Pixel = nullptr;
    ID3D11InputLayout* Layout = nullptr;
    D3D11_PRIMITIVE_TOPOLOGY Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};


struct FRenderResources 
{
    struct FPrimitiveComponentContainer
    {
        TArray<class UStaticMeshComponent*> StaticMeshObjs;
        TArray<class UGizmoBaseComponent*> GizmoObjs;
        TArray<class UBillboardComponent*> BillboardObjs;
        TArray<class ULightComponentBase*> LightObjs;
    } Primitives;

    struct FShaderResourceContainer
    {
        FShaderResource StaticMesh;
        FShaderResource Texture;
        FShaderResource Text;
        FShaderResource Line;
    } Shaders;

    struct FConstantBufferContainer
    {
        FConstantBuffersStaticMesh StaticMesh;
        FConstantBuffersBatchLine BatchLine;
        //FConstantBuffersBatchLine BatchLine; // line text 추가해야함
        //FConstantBuffersBatchLine BatchLine;
    } ConstantBuffers;

    // 나중에 DebugShader로 넘어가야함.
    struct FShaderResourceViewContainer
    {
        ID3D11ShaderResourceView* AABB = nullptr;
        ID3D11ShaderResourceView* Cone = nullptr;
        ID3D11ShaderResourceView* OBB = nullptr;
    } ShaderResourceView;
};

