#pragma once
// 수학 관련
#include <d3d11.h>
#include "Math/Vector.h"
#include "Math/Vector4.h"
#include "Math/Matrix.h"


// 아래의 두개 다 수정하기
constexpr unsigned FCONSTANT_NUM_DIRLIGHT = 1;
constexpr unsigned FCONSTANT_NUM_POINTLIGHT = 1;
constexpr unsigned FCONSTANT_NUM_SPOTLIGHT = 1;

const D3D_SHADER_MACRO defines[] =
{
    "FCONSTANT_NUM_DIRLIGHT", "1",
    "FCONSTANT_NUM_POINTLIGHT", "1",
    "FCONSTANT_NUM_SPOTLIGHT", "1",
    NULL, NULL
};


struct FConstantBuffers
{
    ID3D11Buffer* Camera00;
    ID3D11Buffer* Light01;
    ID3D11Buffer* Actor03;
    ID3D11Buffer* Texture05;
    ID3D11Buffer* Mesh06;
};




struct FMaterialConstants {
    FVector DiffuseColor;
    float TransparencyScalar;
    FVector AmbientColor;
    float DensityScalar;
    FVector SpecularColor;
    float SpecularScalar;
    FVector EmmisiveColor;
    float MaterialPad0;
};

////////////////////////////////////
// LIGHTS
struct alignas(16) FConstantBufferLightColor
{
    alignas(16) FVector Specular;

    alignas(16) FVector Diffuse;

    alignas(16) FVector Ambient;
};

struct alignas(16) FConstantBufferLightDir
{
    FConstantBufferLightColor Color;

    alignas(16) FVector Direction;
};

struct alignas(16) FConstantBufferLightPoint
{
    FConstantBufferLightColor Color;

    alignas(16) FVector Position;

    alignas(16) float Intensity;
    float Radius;
    float RadiusFallOff;
};

struct alignas(16) FConstantBufferLightSpot
{
    FConstantBufferLightColor Color;
    alignas(16) float ConstantTerm;
    float Linear;
    float Quadratic;

    alignas(16) float CutOff;
    float OuterCutOff;
};


/// <summary>
/// Per-Mesh 상수버퍼 : b6
/// </summary>
struct alignas(16) FConstantBufferMesh
{
    FMatrix ModelMatrix;
    FMatrix ModelInvTransMatrix;

    FMaterialConstants Material;

    alignas(16) UINT IsSelectedMesh;
    //FMatrix MVP;      // 모델
    //FMatrix ModelMatrixInverseTranspose; // normal 변환을 위한 행렬
    //FVector4 UUIDColor;
    //bool IsSelected;
    //FVector pad;
};

/// <summary>
/// Per-Texture 상수버퍼 : b5
/// </summary>
struct alignas(16) FConstantBufferTexture
{
    FVector2D UVOffset;
};

/// <summary>
/// Per-Actor 상수버퍼 : b3
/// </summary>
struct alignas(16) FConstantBufferActor
{
    FVector4 UUID; // 임시
    UINT IsSelectedActor;
};

/// <summary>
/// Lighting : b1
/// </summary>
struct alignas(16) FConstantBufferLights
{
    FConstantBufferLightDir DirLights[FCONSTANT_NUM_DIRLIGHT];
    FConstantBufferLightPoint PointLights[FCONSTANT_NUM_POINTLIGHT];
    FConstantBufferLightSpot SpotLights[FCONSTANT_NUM_SPOTLIGHT];
    alignas(16) UINT isLit;
};

/// <summary>
/// Per-Scene 상수버퍼 : b0
/// </summary>
struct alignas(16) FConstantBufferCamera
{
    FMatrix ViewMatrix;
    FMatrix ProjMatrix;
    alignas(16) FVector CameraPos;
    alignas(16) FVector CameraLookAt;
};

