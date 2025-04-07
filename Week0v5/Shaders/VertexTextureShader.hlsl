#include "ShaderConstants.hlsli"

struct VSInput {
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
};

struct PSInput {
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

PSInput main(VSInput input) {
    PSInput output;
    
    float4 pos;
    pos = mul(float4(input.position, 1.0f), ModelMatrix);
    pos = mul(pos, ViewMatrix);
    pos = mul(pos, ProjMatrix);
    output.position = pos;
    
    output.texCoord = input.texCoord;
    
    return output;
}
