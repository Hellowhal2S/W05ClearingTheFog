#include "LightConstants.hlsli"

// 무지성으로 t10에서부터 시작

Texture2D g_renderTex : register(t10);    // 원본 렌더링 결과
Texture2D g_depthOnlyTex : register(t11); // Depth Map SRV
Texture2D g_worldPosTex : register(t12); // Depth Map SRV
Texture2D g_worldNormalTex : register(t13); // Depth Map SRV
Texture2D g_albedoTex : register(t14); // Depth Map SRV
Texture2D g_specularTex : register(t15); // Depth Map SRV
//Texture2D worldPosTex : register(t12); // World Position SRV

SamplerState g_Sampler : register(s0); // Linear Clamp Sampler

cbuffer CameraConstants : register(b10)
{
    row_major float4x4 invProj; // Projection matrix의 역행렬
    row_major float4x4 invView;
    float3 eyeWorld;
    float camNear;
    float camFar;
    float3 Camerapadding;
};

cbuffer FogConstants : register(b11)
{
    float depthStart;
    float depthFalloff;
    float heightStart;
    float heightFalloff;
    float fogDensity;
    float heightDensity;
    int mode; // 1: Rendered image, 2: DepthOnly
    bool fogEnabled;
    float4 fogColor;
};


struct SamplingPixelShaderInput
{
    float4 posProj : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

float4 TexcoordToView(float2 texcoord)
{
    float4 posProj;

    // [0, 1]x[0, 1] -> [-1, 1]x[-1, 1]
    posProj.xy = texcoord * 2.0 - 1.0;
    posProj.y *= -1; // 주의: y 방향을 뒤집어줘야 합니다.
    posProj.z = g_depthOnlyTex.Sample(g_Sampler, texcoord).r;
    posProj.w = 1.0;

    // ProjectSpace -> ViewSpace
    float4 posView = mul(posProj, invProj);
    float4 posWorld = mul(posView, invView);
    posWorld.xyz /= posWorld.w;
    
    return posView;
}

float LinearizeAndNormalizeDepth(float z_buffer, float nearZ, float farZ)
{
    float linearizedZ = (nearZ * farZ) / (farZ - z_buffer * (farZ - nearZ));
    return saturate((linearizedZ - nearZ) / (farZ - nearZ)); // scale 0 to 1

}

float4 mainPS(SamplingPixelShaderInput input) : SV_TARGET
{
    float2 EncodedNormal = g_worldNormalTex.Sample(g_Sampler, input.texcoord).rg;
    float3 normal = DecodeNormalOctahedral(EncodedNormal);
    
    if (mode ==1)
    {
        return float4(normal, 1.0f);
    }
    else if (mode == 2)
    {
        float depth = g_depthOnlyTex.Sample(g_Sampler, input.texcoord).r;
        depth = LinearizeAndNormalizeDepth(depth, 0.1f, 100.0f);
        
        return float4(depth.rrr, 1.0f);
    }
    else if (mode == 3)
    {
        return float4(g_worldPosTex.Sample(g_Sampler, input.texcoord).rgb, 1.0f);
    }
    else if (mode == 4)
    {
        return float4(g_albedoTex.Sample(g_Sampler, input.texcoord).rgb, 1.0f);
    }
    else if (mode == 5)
    {
        return float4(g_specularTex.Sample(g_Sampler, input.texcoord).rgb, 1.0f);
    }

    float3 litColor = g_renderTex.Sample(g_Sampler, input.texcoord).rgb;
    // 모드 1: 렌더링 이미지에 안개 효과 적용
    if (isLit == 1)
    {
        float3 materialDiffuseColor = g_albedoTex.Sample(g_Sampler, input.texcoord).rgb;
        float3 materialSpecularColor = g_specularTex.Sample(g_Sampler, input.texcoord).rgb;
        float3 worldPos = g_worldPosTex.Sample(g_Sampler, input.texcoord).rgb;
        float materialSpecularScalar = g_specularTex.Sample(g_Sampler, input.texcoord).a;
        float3 viewDirection = normalize(float3(eyeWorld - worldPos));

        float3 color = CalculateDirectionLight(DirLights[0], worldPos, normal, viewDirection, materialDiffuseColor, materialSpecularColor, materialSpecularScalar);
            
        for (int i = 0; i < NumPointLights; i++)
        {
            color += CalculatePointLight(PointLights[i], worldPos, normal, viewDirection, materialDiffuseColor, materialSpecularColor, materialSpecularScalar);
        }
            
        litColor = color;
    }
        // // 뷰 공간 좌표 복원 (거리 기반 안개 계산용)
    if (fogEnabled)
    {
        float4 posView = TexcoordToView(input.texcoord);
        
        float dist = length(posView.xyz);
            //float distFog = saturate((dist - depthStart) / (depthFalloff - depthStart));
        float rawDepth = g_depthOnlyTex.Sample(g_Sampler, input.texcoord).r;
        float linearDepth = LinearizeAndNormalizeDepth(rawDepth, 0.1f, 100.0f);
        float fogFactor = 1.0 - exp(-fogDensity * linearDepth);
        float3 color = g_renderTex.Sample(g_Sampler, input.texcoord).rgb;
        
        float worldHeight = g_worldPosTex.Sample(g_Sampler, input.texcoord).z;
        float heightFactor = 1.0 - saturate((worldHeight - heightStart) / heightFalloff);
        fogFactor += heightDensity * heightFactor;
        litColor = lerp(litColor, fogColor.rgb, fogFactor);
    }
    
    return float4(litColor, 1.0);
}
