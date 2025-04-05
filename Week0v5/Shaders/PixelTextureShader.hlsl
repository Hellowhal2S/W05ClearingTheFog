#include "ShaderConstants.hlsli"

Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);


struct PSInput {
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

struct PSOutput
{
    float4 color : SV_Target0;
    float4 uuid : SV_Target1;
};

float4 main(PSInput input) : SV_TARGET {
    PSOutput output;
    
    float2 uv = input.texCoord + UVOffset;
    //float4 col = gTexture.Sample(gSampler, input.texCoord);
    float4 col = gTexture.Sample(gSampler, uv);
    float threshold = 0.05; // 필요한 경우 임계값을 조정
    if (col.r < threshold && col.g < threshold && col.b < threshold)
        clip(-1); // 픽셀 버리기
    
    output.color = col;
    output.uuid = UUID;
    
    return col;
}
