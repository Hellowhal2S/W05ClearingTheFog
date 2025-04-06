// 무지성으로 t10에서부터 시작

Texture2D renderTex : register(t10);    // 원본 렌더링 결과
Texture2D depthOnlyTex : register(t11); // Depth Map SRV
//Texture2D worldPosTex : register(t12); // World Position SRV

SamplerState Sampler : register(s0); // Linear Clamp Sampler

cbuffer CameraConstants : register(b0)
{
    float4x4 invProj; // Projection matrix의 역행렬
    float4x4 invView;
    float3 eyeWorld;
    float camNear;
    float camFar;
    float3 Camerapadding;
};

cbuffer FogConstants : register(b1)
{
    float heightStart;
    float heightFalloff;
    float fogDensity;
    int mode; // 1: Rendered image, 2: DepthOnly
    float4 fogColor;
    float depthScale;
    float3 padding;
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
    posProj.z = depthOnlyTex.Sample(Sampler, texcoord).r;
    posProj.w = 1.0;

    // ProjectSpace -> ViewSpace
    float4 posView = mul(posProj, invProj);
    posView.xyz /= posView.w;
    
    return posView;
}

float LinearizeAndNormalizeDepth(float z_buffer, float nearZ, float farZ)
{
    float linearizedZ = (nearZ * farZ) / (farZ - z_buffer * (farZ - nearZ));
    return saturate((linearizedZ - nearZ) / (farZ - nearZ));

}

float4 mainPS(SamplingPixelShaderInput input) : SV_TARGET
{
    //return float4(1.0f, 0.0f, 0.0f, 1.0f); // TODO: 수정 필요)
    //return float4(renderTex.Sample(Sampler, input.texcoord).rgb,1.0f);

    
    if (mode == 2)
    {
        float depth = depthOnlyTex.Sample(Sampler, input.texcoord).r;
        depth = LinearizeAndNormalizeDepth(depth, 0.1f, 100.0f);
        
        return float4(depth.rrr, 1.0f);
        // TODO: Fog
    }
    else // 모드 1: 렌더링 이미지에 안개 효과 적용
    {
        // 뷰 공간 좌표 계산
        float4 posView = TexcoordToView(input.texcoord);
        float dist = length(posView.xyz); // 카메라(원점)로부터의 거리

        // 기본 지수 안개 계산: 거리에 따른 안개 양
        float fogAmount = saturate(1.0 - exp(-fogDensity * dist * depthScale));

        // (선택 사항) 높이 기반 안개 감쇠 추가:
        // float heightFactor = saturate((posView.y - heightStart) / heightFalloff);
        // fogAmount *= heightFactor;

        float3 sceneColor = renderTex.Sample(Sampler, input.texcoord).rgb;
        // 안개 색상은 상수 버퍼의 fogColor 사용
        float3 finalColor = lerp(sceneColor, fogColor.rgb, fogAmount);
        return float4(finalColor, 1.0f);
    }
}
