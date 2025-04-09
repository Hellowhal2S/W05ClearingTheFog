#include "AExponentialHeightFog.h"

#include "EditorEngine.h"
#include "Components/HeightFogComponent.h"
#include "Engine/World.h"

AExponentialHeightFog::AExponentialHeightFog()
{
    fogComponent = FObjectFactory::ConstructObject<UHeightFogComponent>();
    RootComponent = fogComponent;
    AddComponent(fogComponent);
    GEngine->GetWorld()->Fog = this;
}

AExponentialHeightFog::~AExponentialHeightFog()
{

}

AExponentialHeightFog::AExponentialHeightFog(const AExponentialHeightFog& other) : AActor(other), fogComponent(other.fogComponent)
{
}

UObject* AExponentialHeightFog::Duplicate() const
{
    AExponentialHeightFog* ClonedFog = FObjectFactory::ConstructObjectFrom(this);
    ClonedFog->DuplicateSubObjects(this);
    ClonedFog->PostDuplicate();
    return ClonedFog;
}

void AExponentialHeightFog::DuplicateSubObjects(const UObject* Source)
{
    UObject::DuplicateSubObjects(Source);
    const AExponentialHeightFog* Fog = Cast<AExponentialHeightFog>(Source);
    fogComponent = Cast<UHeightFogComponent>(fogComponent->Duplicate());
    RootComponent = fogComponent;
}
inline void AExponentialHeightFog::Destroyed()
{
    AActor::Destroyed();
    // if (GEngine->GetWorld()->WorldType == EWorldType::Editor)
    //     GEngine->GetWorld()->Fog = nullptr;
}