struct VS_INPUT
{
    float4 position : POSITION;
    float2 texcoord : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};


VS_OUTPUT mainVS(VS_INPUT input)
{
    VS_OUTPUT output;
    
    output.position = float(input.position, 1.0);
    output.texcoord = input.texcoord;
    return output;
}