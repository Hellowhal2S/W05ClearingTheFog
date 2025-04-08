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
cbuffer ViewportConstants : register(b4)
{
    float screenWidth;
    float screenHeight;
    float topLeftX;
    float topLeftY;
    float width;
    float height;
    float2 padding;
};
const static float2 QuadPos[6] =
{
    float2(-1, -1), float2(-1, 1), float2(1, -1),   // 좌하단, 좌상단, 우하단
    float2(-1,  1), float2(1,  1), float2(1, -1)    // 좌상단, 우상단, 우하단
};
const static float2 QuadTex[6] =
{
    float2(0.0, 1.0), float2(0.0, 0.0), float2(1.0, 1.0),
    float2(0.0, 0.0), float2(1.0, 0.0), float2(1.0, 1.0)
};

VS_OUTPUT mainVS(uint vertexID : SV_VertexID)
{
    VS_OUTPUT output;
    
    output.position = float4(QuadPos[vertexID], 0.0, 1.0);
    
    float2 baseUV = QuadTex[vertexID];
    
    // 전체 해상도에서 뷰포트의 위치와 크기를 정규화하여 UV 오프셋/스케일을 계산 (픽셀 -> [0,1])
    float2 uvOffset = float2(topLeftX / screenWidth, topLeftY / screenHeight);
    float2 uvScale  = float2(width / screenWidth, height / screenHeight);
    
    // 최종 텍스처 좌표 계산  
    float2 finalUV = uvOffset + baseUV * uvScale;
    
    output.texcoord = finalUV;
    return output;
}