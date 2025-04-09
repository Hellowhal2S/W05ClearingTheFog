#pragma once

#include <memory>
#define _TCHAR_DEFINED
#include <d3d11.h>
#include "Core/Container/Array.h"
#include "Core/Container/Map.h"
#include "ShaderConstants.h"
#include "Engine/Classes/Engine/Texture.h"

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
    UINT32 VertexStride;
    UINT32 NumIndices;
};

// Icon
enum class IconType
{
    None,
    DirectionalLight,
    PointLight,
    SpotLight,
    ExponentialFog,
    AtmosphericFog,
};

struct FRenderResourcesDebug
{
    struct FWorldComponentContainer
    {
        TArray<class UStaticMeshComponent*> StaticMesh;
        TArray<class ULightComponentBase*> Light;
        TArray<class UHeightFogComponent*> Fog;
    } Components;

    struct FShaderResourceContainer
    {
        FShaderResource Gizmo;
        FShaderResource AxisLine;
        FShaderResource AABB;
        FShaderResource Sphere;
        FShaderResource Grid;
        FShaderResource Cone;
        FShaderResource Icon;
        FShaderResource Arrow;
    } Shaders;

    struct FConstantBuffers
    {
        ID3D11Buffer* Camera00;
        ID3D11Buffer* AABB13;
        ID3D11Buffer* Sphere13;
        ID3D11Buffer* Cone13;
        ID3D11Buffer* Grid13;
        ID3D11Buffer* Icon13;
        ID3D11Buffer* Arrow13;
    } ConstantBuffers;

    struct FPrimitiveResourceContainer
    {
        FDebugPrimitiveData Box;
        FDebugPrimitiveData Sphere;
        FDebugPrimitiveData Cone;
        FDebugPrimitiveData Arrow;
    } Primitives;
    
    TMap<IconType, std::shared_ptr<FTexture>> IconTextures;
};