#include "FireBallComponent.h"


UPointlightComponent::UPointlightComponent()
{
    Intensity = 1000.0f;
    Radius = 500.0f;
    RadiusFallOff = 2.0f;
    Color = FLinearColor::Red;
}

UPointlightComponent::~UPointlightComponent()
{
}

void UPointlightComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UPointlightComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

UObject* UPointlightComponent::Duplicate() const
{
    UPointlightComponent* ClonedActor = FObjectFactory::ConstructObjectFrom<UPointlightComponent>(this);
    ClonedActor->DuplicateSubObjects(this);
    ClonedActor->PostDuplicate();
    return ClonedActor;
}

void UPointlightComponent::DuplicateSubObjects(const UObject* Source)
{
    Super::DuplicateSubObjects(Source);
}

void UPointlightComponent::PostDuplicate()
{
    Super::PostDuplicate();
}
