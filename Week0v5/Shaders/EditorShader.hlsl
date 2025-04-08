#include "ShaderConstants.hlsli"

// Input Layout은 float3이지만, shader에서 missing w = 1로 처리해서 사용
// https://stackoverflow.com/questions/29728349/hlsl-sv-position-why-how-from-float3-to-float4
struct VS_INPUT
{
    float4 position : POSITION; // 버텍스 위치
    float4 color : COLOR; // 버텍스 색상
    float3 normal : NORMAL; // 버텍스 노멀
    float2 texcoord : TEXCOORD;
    int materialIndex : MATERIAL_INDEX;
};

struct PS_INPUT
{
    float4 position : SV_Position;
    float4 color : COLOR;
};

/////////////////////////////////////////////
// GIZMO
PS_INPUT gizmoVS(VS_INPUT input)
{
    PS_INPUT output;
    
    float4 pos;
    pos = mul(input.position, ModelMatrix);
    pos = mul(pos, ViewMatrix);
    pos = mul(pos, ProjMatrix);
    
    output.position = pos;
    
    output.color = float4(Material.DiffuseColor, 1.f);
    
    return output;
}

float4 gizmoPS(PS_INPUT input) : SV_Target
{
    return input.color;
}

/////////////////////////////////////////////
// Axis
// Input buffer는 없고 대신 Draw(6)하면됨.

const static float4 AxisPos[6] =
{
    float4(0, 0, 0, 1),
    float4(10000000, 0, 0, 1),
    float4(0, 0, 0, 1),
    float4(0, 10000000, 0, 1),
    float4(0, 0, 0, 1),
    float4(0, 0, 10000000, 1)
};

const static float4 AxisColor[3] =
{
    float4(1, 0, 0, 1),
    float4(0, 1, 0, 1),
    float4(0, 0, 1, 1)
};

// Draw()에서 NumVertices만큼 SV_VertexID만 다른채로 호출됨.
// 어차피 월드에 하나이므로 Vertex를 받지않음.
PS_INPUT axisVS(uint vertexID : SV_VertexID)
{
    PS_INPUT output;
    
    float4 Vertex = AxisPos[vertexID];
    Vertex = mul(Vertex, ViewMatrix);
    Vertex = mul(Vertex, ProjMatrix);
    output.position = Vertex;
    
    output.color = AxisColor[vertexID / 2];
    
    return output;
}

float4 axisPS(PS_INPUT input) : SV_Target
{
    return input.color;
}

/////////////////////////////////////////////
// AABB
// Input buffer는 position과 extent
struct VS_INPUT_POS_ONLY
{
    float4 position : POSITION0;
};

PS_INPUT aabbVS(VS_INPUT_POS_ONLY input, uint instanceID : SV_InstanceID)
{
    PS_INPUT output;
    
    float3 pos = DataAABB[instanceID].Position;
    float3 scale = DataAABB[instanceID].Extent;
    //scale = float3(1, 1, 1);
    
    float4 localPos = float4(input.position.xyz * scale + pos, 1.f);
        
    localPos = mul(localPos, ViewMatrix);
    localPos = mul(localPos, ProjMatrix);
    output.position = localPos;
    
    // color는 지정안해줌
    
    return output;
}

float4 aabbPS(PS_INPUT input) : SV_Target
{
    return float4(1.0f, 1.0f, 0.0f, 1.0f); // 노란색 AABB
}

PS_INPUT sphereVS(VS_INPUT_POS_ONLY input, uint instanceID : SV_InstanceID)
{
    PS_INPUT output;
    
    float3 pos = DataSphere[instanceID].Position;
    float scale = DataSphere[instanceID].Radius;
    //scale = float3(1, 1, 1);
    
    float4 localPos = float4(input.position.xyz * scale + pos, 1.f);
        
    localPos = mul(localPos, ViewMatrix);
    localPos = mul(localPos, ProjMatrix);
    output.position = localPos;
    
    // color는 지정안해줌
    
    return output;
}

float4 spherePS(PS_INPUT input) : SV_Target
{
    return float4(1.0f, 1.0f, 0.0f, 1.0f); // 노란색 AABB
}