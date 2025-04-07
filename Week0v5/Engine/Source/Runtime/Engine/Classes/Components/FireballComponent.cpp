#include "FireballComponent.h"

#include "UObject/ObjectFactory.h"

UFireBallComponent::UFireBallComponent()
{
    SetType(StaticClass()->GetName());
    Intensity = 1000.0f;
    Radius = 500.0f;
    RadiusFallOff = 2.0f;
    Color = FLinearColor::Red;
}

UFireBallComponent::~UFireBallComponent()
{
}

UFireBallComponent::UFireBallComponent(const UFireBallComponent& other) : UPrimitiveComponent(other),
Intensity(other.Intensity), Radius(other.Radius), RadiusFallOff(other.RadiusFallOff), Color(other.Color)
{
}

void UFireBallComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UFireBallComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

UObject* UFireBallComponent::Duplicate() const
{
    UFireBallComponent* ClonedActor = FObjectFactory::ConstructObjectFrom<UFireBallComponent>(this);
    ClonedActor->DuplicateSubObjects(this);
    ClonedActor->PostDuplicate();
    return ClonedActor;
}

void UFireBallComponent::DuplicateSubObjects(const UObject* Source)
{
    UPrimitiveComponent::DuplicateSubObjects(Source);
}

void UFireBallComponent::PostDuplicate()
{
    UPrimitiveComponent::PostDuplicate();
}
