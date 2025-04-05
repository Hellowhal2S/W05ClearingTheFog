// 무지성으로 t10에서부터 시작

Texture2D renderTex : register(t10);    // 원본 렌더링 결과
Texture2D depthOnlyTex : register(t11); // Depth Map SRV
//Texture2D worldPosTex : register(t12); // World Position SRV

SamplerState Sampler : register(s0); // Linear Clamp Sampler

cbuffer GlobalConstants : register(b0)
{
    float4x4 invProj; // Projection matrix의 역행렬
};

cbuffer FogConstants : register(b1)
{
    float heightStart;
    float heightFalloff;
    float fogDensity;
    int mode; // 1: Rendered image, 2: DepthOnly
    float4 fogColor;
};

struct SamplingPixelShaderInput
{
    float4 posProj : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

//float4 TexcoordToView(float2 texcoord)
//{
//    float4 posProj;

//    // [0, 1]x[0, 1] -> [-1, 1]x[-1, 1]
//    posProj.xy = texcoord * 2.0 - 1.0;
//    posProj.y *= -1; // 주의: y 방향을 뒤집어줘야 합니다.
//    posProj.z = depthOnlyTex.Sample(Sampler, texcoord).r;
//    posProj.w = 1.0;

//    // ProjectSpace -> ViewSpace
//    float4 posView = mul(posProj, invProj);
//    posView.xyz /= posView.w;
    
//    return posView;
//}

float4 mainPS(SamplingPixelShaderInput input) : SV_TARGET
{
    return renderTex.Sample(Sampler, input.texcoord);
    //if (mode == 1)
    //{
    //    float4 posView = TexcoordToView(input.texcoord);
    //    float fogMin = 1.0;
    //    float fogMax = 10.0;
        
    //    float dist = length(posView.xyz); // 눈의 위치가 원점인 좌표계
    //    float distFog = saturate((dist - fogMin) / (fogMax - fogMin));
    //    float fogFactor = exp(-distFog * fogDensity);
        
    //    float3 fogColor = float3(1, 1, 1);
    //    float3 color = renderTex.Sample(Sampler, input.texcoord).rgb;
    //    color = lerp(fogColor, color, fogFactor);
    //    return float4(color, 1.0);
        
    //    // TODO: Fog
    //}
    //else // if (mode == 2)
    //{
    //    float z = TexcoordToView(input.texcoord).z /* TODO  : 수정 필요 -> 원본 (* depthScale) */;
    //    return float4(z, z, z, 1);
    //}
}
