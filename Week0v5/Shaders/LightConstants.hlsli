float3 CalculatePointLight(FConstantBufferLightPoint Light, float3 WorldPos, float3 Normal, float3 ViewDir, float3 DiffuseColor, float3 SpecularColor, float3 SpecularPower)
{
    float3 LightDir = normalize(Light.Position - WorldPos);
    float Distance = length(Light.Position - WorldPos);
    if (Distance > Light.Radius)
        return float3(0, 0, 0);
    float NormalizedDistance = saturate(Distance / Light.Radius);
    float RadiusAttenuation = 1.0 - NormalizedDistance * NormalizedDistance; // 부드러운 경계 추가
    float DistanceAttenuation = 1.0f / (1.0f + Light.RadiusFallOff * Distance * Distance);
    float Attenuation = RadiusAttenuation * DistanceAttenuation * Light.Intensity;
    float Diff = max(dot(Normal, LightDir), 0.0f);
    float3 Diffuse = Light.Color.rgb * Diff * DiffuseColor * Attenuation; // float3으로 수정
    float3 ReflectDir = normalize(reflect(-LightDir, Normal)); // 눈으로 향하는 빛 반사 벡터
    float Spec = pow(max(dot(ViewDir, ReflectDir), 0.0f), SpecularPower);
    float3 Specular = Light.Color.rgb * SpecularColor * Spec * Attenuation;
    return Diffuse + Specular;
}

float3 BlinnPhong(float3 lightStrength, float3 lightVec, float3 Normal,
                   float3 toEye, Material mat)
{
    float3 Halfway = normalize(lightVec + toEye);
    float HdotN = dot(Halfway, Normal);
    float3 specular = mat.specular * pow(max(HdotN, 0.0f), mat.shininess);

    return mat.ambient + (mat.diffuse + specular) * lightStrength;
}

float3 ComputeDirectionalLight(Light L, Material mat, float3 Normal, float3 toEye)
{
    // TODO:
    float3 lightVec = -L.direction;

    float3 NdotL = max(dot(lightVec, Normal), 0.0f);

    float3 lightStrength = NdotL * L.strength;
    return BlinnPhong(lightStrength, lightVec, Normal, toEye, mat);
}

float3 ComputeSpotLight(Light L, Material mat, float3 pos, float3 Normal, float3 toEye)
{
    float3 lightVec = L.position - pos;

    // 쉐이딩할 지점부터 조명까지의 거리 계산
    float Dist = length(lightVec);

    // 너무 멀면 조명이 적용되지 않음
    if (Dist > L.Radius)
    {
        return float3(0.0f, 0.0f, 0.0f);
    }
    else
    {
        lightVec /= Dist;
        
        float NdotL = max(dot(lightVec, Normal), 0.0);
        float3 lightStrength = L.Intensity * NdotL;
        
        float att = CalcAttenuation(d, L.fallOffStart, L.fallOffEnd);
        lightStrength *= att;
        
        float spotFactor = pow(max(dot(-lightVec, L.direction), 0.0), L.spotPower);
        lightStrength *= spotFactor;

        return BlinnPhong(lightStrength, lightVec, Normal, toEye, mat);
    }
    
    // if에 else가 없을 경우 경고 발생
    // warning X4000: use of potentially uninitialized variable
}