#pragma once

#define _TCHAR_DEFINED
#include <d3d11.h> // Array.h보다 늦게 선언해야 오류 안뜸 ??? 
#include "Core/Container/Array.h"
#include "ShaderConstants.h"

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
    struct FWorldComponentContainer
    {
        TArray<class UStaticMeshComponent*> StaticMeshObjs;
        TArray<class UBillboardComponent*> BillboardObjs;
        TArray<class ULightComponentBase*> LightObjs;
    } Components;

    struct FShaderResourceContainer
    {
        FShaderResource StaticMesh;
        FShaderResource Texture;
        FShaderResource Text;

    } Shaders;

    struct FConstantBufferContainer
    {
        FConstantBuffersStaticMesh StaticMesh;
        // texture관련 cb필요.
        //FConstantBuffersBatchLine BatchLine;
        //FConstantBuffersBatchLine BatchLine; // line text 추가해야함
        //FConstantBuffersBatchLine BatchLine;
    } ConstantBuffers;
};

struct FDebugPrimitiveData
{
    ID3D11Buffer* Vertex;
    ID3D11Buffer* Index;
    UINT32 NumVertices;
};
struct FRenderResourcesDebug
{
    struct FWorldComponentContainer
    {
        TArray<class UPrimitiveComponent*> PrimitiveObjs;
    } Components;

    struct FShaderResourceContainer
    {
        FShaderResource Gizmo;
        FShaderResource AxisLine;
        FShaderResource AABB;
        FShaderResource Sphere;
        FShaderResource Line;
        FShaderResource GridLine;
    } Shaders;

    struct FConstantBuffers
    {
        ID3D11Buffer* Camera00;
        ID3D11Buffer* AABB13;
    } ConstantBuffers;

    struct FPrimitiveResourceContainer
    {
        FDebugPrimitiveData Box;
    } Primitives;
};