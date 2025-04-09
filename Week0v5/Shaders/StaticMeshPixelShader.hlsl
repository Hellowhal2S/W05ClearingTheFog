#include "ShaderConstants.hlsli"

Texture2D Textures : register(t0);
SamplerState Sampler : register(s0);

struct PS_INPUT
{
    float4 position : SV_POSITION; // 변환된 화면 좌표
    float4 color : COLOR; // 전달할 색상
    float3 normal : NORMAL; // 정규화된 노멀 벡터
    bool normalFlag : TEXCOORD0; // 노멀 유효성 플래그 (1.0: 유효, 0.0: 무효)
    float2 texcoord : TEXCOORD1;
    int materialIndex : MATERIAL_INDEX;
    float4 worldPos : TEXCOORD2; // 월드 좌표
};

struct PS_OUTPUT
{
    float4 color : SV_Target0;
    float4 UUID : SV_Target1;
    float4 worldPos : SV_Target2;
    float2 worldNormal : SV_Target3;
    float4 Albedo : SV_Target4;
    float4 SpecularColor_Power : SV_Target5;
};

float noise(float3 p)
{
    return frac(sin(dot(p, float3(12.9898, 78.233, 37.719))) * 43758.5453);
}
float2 EncodeNormalOctahedral(float3 N)
{
    // 먼저 3D 벡터의 절대값 합으로 나누어 정규화합니다.
    float InvL1 = 1.0 / (abs(N.x) + abs(N.y) + abs(N.z));
    N *= InvL1;

    // 초기 2D 벡터 값은 xy, z가 음수일 때만 보정함
    float2 Enc = N.xy;

    if (N.z < 0.0)
    {
        // swap된 성분의 절대값 보정을 통해 반사 효과 적용
        Enc = (1.0 - abs(Enc.yx)) * sign(Enc.xy);
    }
    return Enc;
}


float4 PaperTexture(float3 originalColor)
{
    // 입력 색상을 0~1 범위로 제한
    float3 color = saturate(originalColor);
    
    float3 paperColor = float3(0.95, 0.95, 0.95);
    float blendFactor = 0.5;
    float3 mixedColor = lerp(color, paperColor, blendFactor);
    
    // 정적 grain 효과
    float grain = noise(color * 10.0) * 0.1;
    
    // 거친 질감 효과: 두 단계의 노이즈 레이어를 결합
    float rough1 = (noise(color * 20.0) - 0.5) * 0.15; // 첫 번째 레이어: 큰 규모의 노이즈
    float rough2 = (noise(color * 40.0) - 0.5) * 0.01; // 두 번째 레이어: 세부 질감 추가
    float rough = rough1 + rough2;
    
    // vignette 효과: 중앙에서 멀어질수록 어두워지는 효과
    float vignetting = smoothstep(0.4, 1.0, length(color.xy - 0.5) * 2.0);
    
    // 최종 색상 계산
    float3 finalColor = mixedColor + grain + rough - vignetting * 0.1;
    return float4(saturate(finalColor), 1.0);
}

PS_OUTPUT mainPS(PS_INPUT input)
{
    PS_OUTPUT output;
    
    output.UUID = UUID;
    output.worldPos = input.worldPos;
    output.worldNormal = EncodeNormalOctahedral(input.normal);
    output.SpecularColor_Power = float4(Material.SpecularColor, Material.SpecularScalar);
    
    float3 texColor = Textures.Sample(Sampler, input.texcoord + UVOffset);
    output.Albedo = float4(Material.DiffuseColor + texColor, 1.0f);
    float3 color;
    if (texColor.g == 0) // TODO: boolean으로 변경
        color = saturate(Material.DiffuseColor);
    else
    {
        color = texColor + Material.DiffuseColor;
    }
    
    if (IsSelectedActor)
    {
        color += float3(0.2f, 0.2f, 0.0f); // 노란색 틴트로 하이라이트
        if (IsSelectedMesh)
            color = float3(1, 1, 1);
    }
 
        
    output.color = float4(color, 1);
        // 투명도 적용
    output.color.a = Material.TransparencyScalar;
            
    return output;
}

PS_OUTPUT applePS(PS_INPUT input)
{
    PS_OUTPUT output;
    
    output.UUID = UUID;
    output.worldPos = input.worldPos;
    output.worldNormal = EncodeNormalOctahedral(input.normal);
    output.SpecularColor_Power = float4(AppleMaterial.SpecularColor, AppleMaterial.SpecularScalar);
    
    float3 texColor = Textures.Sample(Sampler, input.texcoord);
    output.Albedo = float4(AppleMaterial.DiffuseColor + texColor, 1.0f);
    float3 color;
    if (texColor.g == 0) // TODO: boolean으로 변경
        color = saturate(AppleMaterial.DiffuseColor);
    else
    {
        color = texColor + AppleMaterial.DiffuseColor;
    }
    
    if (IsSelectedActor)
    {
        color += float3(0.2f, 0.2f, 0.0f); // 노란색 틴트로 하이라이트
        if (IsSelectedMesh)
            color = float3(1, 1, 1);
    }
 
        
    output.color = float4(color, 1);
        // 투명도 적용
    output.color.a = AppleMaterial.TransparencyScalar;
            
    return output;
}