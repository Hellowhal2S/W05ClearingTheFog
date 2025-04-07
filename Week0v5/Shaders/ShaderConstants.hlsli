struct FMaterialConstants
{
    float3 DiffuseColor;
    float TransparencyScalar;
    float3 AmbientColor;
    float DensityScalar;
    float3 SpecularColor;
    float SpecularScalar;
    float3 EmmisiveColor;
    float MaterialPad0;
};

struct FConstantLightColor
{
    float3 Specular;
    float _pad0;
    
    float3 Diffuse;
    float _pad1;
    
    float3 Ambient;
    float _pad2;
};

struct FConstantBufferLightDir
{
    FConstantLightColor Color;
    float3 Direction;
    float _pad0;
};

struct FConstantBufferLightPoint
{
    FConstantLightColor Color;
    float3 Position;
    float _pad0;
    
    float Intensity;
    float Radius;
    float RadiusFallOff;
    float _pad1;
};

struct FConstantBufferLightSpot
{
    FConstantLightColor Color;
    float ConstantTerm;
    float Linear;
    float Quadratic;
    float _pad0;
    
    float CutOff;
    float OuterCutOff;
    float _pad1;
    float _pad2;
};



cbuffer ConstantBufferScene : register(b0)
{
    row_major matrix ViewMatrix;
    row_major matrix ProjMatrix;
    float3 CameraPos;
    float3 CameraLookAt;
};

// FCONSTANT_NUM 관련 컴파일 에러(HLSL0033)는 무시하세요
// hlsl 외부에서 정의되고 있기 때문에 이 스코프에서는 선언되지 않지만,
// 셰이더 컴파일시에 정의되고 있습니다.
cbuffer ConstantBufferLights : register(b1)
{
    FConstantBufferLightDir DirLights[FCONSTANT_NUM_DIRLIGHT];
    FConstantBufferLightPoint PointLights[FCONSTANT_NUM_POINTLIGHT];
    FConstantBufferLightSpot SpotLights[FCONSTANT_NUM_SPOTLIGHT];
    uint isLit;
}

cbuffer ConstantBufferActor : register(b3)
{
    float4 UUID; // 임시
    uint IsSelectedActor;
};

cbuffer ConstantBufferTexture : register(b5)
{
    float2 UVOffset;
};

cbuffer ConstantBufferMesh : register(b6)
{
    row_major matrix ModelMatrix;
    row_major matrix ModelInvTransMatrix;
    FMaterialConstants Material;
    uint IsSelectedMesh;
};

// 마지막 슬롯인 b13은 debug용으로 예약.