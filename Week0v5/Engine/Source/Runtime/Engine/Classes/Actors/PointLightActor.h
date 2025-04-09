#pragma once
#include "GameFramework/Actor.h"

class UPointLightComponent;

class APointLightActor : public AActor
{
    DECLARE_CLASS(APointLightActor, AActor)
public:
    APointLightActor();
    UPointLightComponent* GetPointLightComponent() const { return PointLightComponent; }
private:
    UPointLightComponent* PointLightComponent = nullptr;
};