struct VSInput {
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
};

struct PSInput {
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

cbuffer Constants : register(b0)
{
    row_major float4x4 Model;
    row_major float4x4 View;
    row_major float4x4 Projection;
    row_major float4x4 MInverseTranspose;
    float4 UUID;
    bool isSelected;
    float3 MatrixPad0;
};

PSInput main(VSInput input) {


    PSInput output;
    
    float4 pos;
    pos = mul(float4(input.position, 1.0f), Model);
    pos = mul(pos, View);
    pos = mul(pos, Projection);
    output.position = pos;
    
    output.texCoord = input.texCoord;
    
    return output;
}
