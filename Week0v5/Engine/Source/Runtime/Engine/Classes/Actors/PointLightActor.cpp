#include "PointLightActor.h"
#include "Components/LightComponent.h"

APointLightActor::APointLightActor()
{
    PointLightComponent = AddComponent<UPointLightComponent>();
    RootComponent = PointLightComponent;
}