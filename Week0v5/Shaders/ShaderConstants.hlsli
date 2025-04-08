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

float2 EncodeNormalOctahedral(float3 N)
{
    // 먼저 3D 벡터의 절대값 합으로 나누어 정규화합니다.
    float InvL1 = 1.0 / (abs(N.x) + abs(N.y) + abs(N.z));
    N *= InvL1;

    // 초기 2D 벡터 값은 xy, z가 음수일 때만 보정함
    float2 Enc = N.xy;

    if (N.z < 0.0)
    {
        // swap된 성분의 절대값 보정을 통해 반사 효과 적용
        Enc = (1.0 - abs(Enc.yx)) * sign(Enc.xy);
    }
    return Enc;
}
