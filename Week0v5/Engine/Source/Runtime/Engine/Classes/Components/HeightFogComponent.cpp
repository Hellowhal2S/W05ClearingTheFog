#include "HeightFogComponent.h"

#include "UObject/ObjectFactory.h"

UHeightFogComponent::UHeightFogComponent(const UHeightFogComponent& other) :
                                                                           depthStart(other.depthStart),
                                                                           depthFalloff(other.depthFalloff),
                                                                           heightStart(other.heightStart),
                                                                           heightFalloff(other.heightFalloff),
                                                                           fogDensity(other.fogDensity),
                                                                           heightDensity(other.heightDensity),
                                                                           fogColor(other.fogColor)
{
}

UObject* UHeightFogComponent::Duplicate() const
{
    UHeightFogComponent* Cloned = FObjectFactory::ConstructObjectFrom(this);
    return Cloned;
}
