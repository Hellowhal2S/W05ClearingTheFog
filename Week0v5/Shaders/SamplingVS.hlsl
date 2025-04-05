struct VS_INPUT
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};
const static float2 QuadPos[6] =
{
    float2(-1, -1), float2(1, -1), float2(-1, 1), // 첫 번째 삼각형
    float2(-1, 1), float2(1, -1), float2(1, 1) // 두 번째 삼각형
};
const static float2 QuadTex[6] =
{
    float2(0.0, 0.0), float2(1.0, 0.0), float2(0.0, 1.0),
    float2(1.0, 0.0), float2(1.0, 0.0), float2(1.0, 1.0)
};

VS_OUTPUT mainVS(uint vertexID : SV_VertexID)
{
    VS_OUTPUT output;
    
    output.position = float4(QuadPos[vertexID], 0.0, 1.0);
    
    output.texcoord = QuadTex[vertexID];
    
    return output;
}