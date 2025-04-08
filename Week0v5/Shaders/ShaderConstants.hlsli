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
// 디버그는 13번만 쓰고 계속 set과 memcpy가 필요
// 8개는 FEditorRenderer에서 Constantbuffersize에서 온것
// 일단은 hardcoding인데 나중에 compile time에 defines로 넣어야함
struct AABBData
{
    float3 Position;
    float3 Extent;
};
cbuffer ConstantBufferDebugAABB : register(b13)
{
    AABBData DataAABB[8];
}

struct SphereData
{
    float3 Position;
    float Radius;
};
cbuffer ConstantBufferDebugSphere : register(b13)
{
    SphereData DataSphere[8];
}