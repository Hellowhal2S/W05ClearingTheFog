#pragma once
// 수학 관련
#include <d3d11.h>
#include "Math/Vector.h"
#include "Math/Vector4.h"
#include "Math/Matrix.h"


// 아래의 두개 다 수정하기
#define MACRO_FCONSTANT_NUM_DIRLIGHT 1
#define MACRO_FCONSTANT_NUM_POINTLIGHT 1
#define MACRO_FCONSTANT_NUM_SPOTLIGHT 1

// 숫자 값을 문자열로 변환하는 매크로 정의
#define FCONSTANT_STRINGIFY(x) #x
#define FCONSTANT_TOSTRING(x) FCONSTANT_STRINGIFY(x)


const D3D_SHADER_MACRO defines[] =
{
    "FCONSTANT_NUM_DIRLIGHT", FCONSTANT_TOSTRING(MACRO_FCONSTANT_NUM_DIRLIGHT),
    "FCONSTANT_NUM_POINTLIGHT", FCONSTANT_TOSTRING(MACRO_FCONSTANT_NUM_POINTLIGHT),
    "FCONSTANT_NUM_SPOTLIGHT", FCONSTANT_TOSTRING(MACRO_FCONSTANT_NUM_SPOTLIGHT),
    NULL, NULL
};


struct FConstantBuffersStaticMesh
{
    ID3D11Buffer* Camera00;
    ID3D11Buffer* Light01;
    ID3D11Buffer* Actor03;
    ID3D11Buffer* Texture05;
    ID3D11Buffer* Mesh06;
};

struct FConstantBuffersBatchLine
{
    ID3D11Buffer* Camera00;
    ID3D11Buffer* Grid01;
    ID3D11Buffer* BoundingBox02;
    ID3D11Buffer* Counts03;
};



struct FMaterialConstants {
    FVector DiffuseColor = { 0,0,0 };
    float TransparencyScalar = 0;
    FVector AmbientColor = {0,0,0};
    float DensityScalar = 0;
    FVector SpecularColor = {0,0,0};
    float SpecularScalar = 0;
    FVector EmmisiveColor = {0,0,0};
    float MaterialPad0;
};

////////////////////////////////////
// LIGHTS
struct alignas(16) FConstantBufferLightColor
{
    alignas(16) FVector Specular = { 0,0,0 };

    alignas(16) FVector Diffuse = {0,0,0};

    alignas(16) FVector Ambient = {0,0,0};
};

struct alignas(16) FConstantBufferLightDir
{
    FConstantBufferLightColor Color;

    alignas(16) FVector Direction = { 0,0,0 };
    float pad0 =0;
};

struct alignas(16) FConstantBufferLightPoint
{
    FConstantBufferLightColor Color;

    alignas(16) FVector Position = { 0,0,0 };
    float pad0 = 0;

    alignas(16) float Intensity = 0;
    float Radius = 0;
    float RadiusFallOff = 0;
    float pad1 = 0;

};

struct alignas(16) FConstantBufferLightSpot
{
    FConstantBufferLightColor Color;
    alignas(16) float ConstantTerm = 0;
    float Linear = 0;
    float Quadratic = 0;
    float pad0 = 0;

    alignas(16) float CutOff = 0;
    float OuterCutOff = 0;
    float pad1 = 0;
    float pad2 = 0;

};


/// <summary>
/// Per-Mesh 상수버퍼 : b6
/// </summary>
struct alignas(16) FConstantBufferMesh
{
    FMatrix ModelMatrix;
    FMatrix ModelInvTransMatrix;

    FMaterialConstants Material;

    alignas(16) UINT IsSelectedMesh = 0;
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
    FVector2D UVOffset = { 0,0 };
};

/// <summary>
/// Per-Actor 상수버퍼 : b3
/// </summary>
struct alignas(16) FConstantBufferActor
{
    FVector4 UUID; // 임시
    UINT IsSelectedActor = 0;
};

/// <summary>
/// Lighting : b1
/// </summary>
struct alignas(16) FConstantBufferLights
{
    FConstantBufferLightDir DirLights[MACRO_FCONSTANT_NUM_DIRLIGHT];
    FConstantBufferLightPoint PointLights[MACRO_FCONSTANT_NUM_POINTLIGHT];
    FConstantBufferLightSpot SpotLights[MACRO_FCONSTANT_NUM_SPOTLIGHT];
    alignas(16) UINT isLit = 1;
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

