#pragma once
#include "GameFramework/Actor.h"

class UDirectionalLightComponent;

class ADirectionalLightActor : public AActor
{
    DECLARE_CLASS(ADirectionalLightActor, AActor)
public:
    ADirectionalLightActor();
    UDirectionalLightComponent* GetDirectionalLightComponent() const { return DirectionalLightComponent; }
private:
    UDirectionalLightComponent* DirectionalLightComponent = nullptr;
};