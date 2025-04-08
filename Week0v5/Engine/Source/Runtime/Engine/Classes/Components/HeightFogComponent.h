#pragma once
#include "SceneComponent.h"

class UHeightFogComponent : public USceneComponent
{
    DECLARE_CLASS(UHeightFogComponent, USceneComponent)
public:
    UHeightFogComponent(){}
    ~UHeightFogComponent(){}
private:
    float depthStart = 0.0f;
    float depthFalloff = 50.0f;
    float heightStart = 0.0f;
    float heightFalloff = 50.0f;
    float fogDensity = 0.1f;
    float heightDensity =0.3f;
    FVector4 fogColor  = { 1.0f,1.0f,1.0f,1.0f};

public:
    float GetDepthStart() const { return depthStart; }
    float GetDepthFalloff() const  { return depthFalloff; }
    float GetHeightStart() const { return heightStart; }
    float GetHeightFalloff() const { return heightFalloff; }
    float GetFogDensity() const  { return fogDensity; }
    float GetHeightDensity() const { return heightDensity; }
    FVector4 GetFogColor() const  { return fogColor; }
    void SetDepthStart(float newDepthStart) { depthStart = newDepthStart; }
    void SetDepthFalloff(float newDepthFalloff) { depthFalloff = newDepthFalloff; }
    void SetHeightStart(float newHeightStart) { heightStart = newHeightStart; }
    void SetHeightFalloff(float newHeightFalloff) { heightFalloff = newHeightFalloff; }
    void SetFogDensity(float newFogDensity) { fogDensity = newFogDensity; }
    void SetHeightDensity(float newHeightDensity) { heightDensity = newHeightDensity; }
    void SetFogColor(FVector4 newFogColor) { fogColor = newFogColor; }
};
