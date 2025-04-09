#include "DirectionalLightActor.h"
#include "Components/LightComponent.h"

ADirectionalLightActor::ADirectionalLightActor()
{
    DirectionalLightComponent = AddComponent<UDirectionalLightComponent>();
    RootComponent = DirectionalLightComponent;
}
