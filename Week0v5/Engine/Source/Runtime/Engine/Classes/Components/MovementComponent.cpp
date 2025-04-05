#include "MovementComponent.h"

#include "UObject/ObjectFactory.h"

UMovementComponent::UMovementComponent()
{
}

UMovementComponent::UMovementComponent(const UMovementComponent& other)
    : UActorComponent(other),
    Velocity(other.Velocity),
    UpdatedComponent(other.UpdatedComponent)
{
}

UMovementComponent::~UMovementComponent()
{
}

void UMovementComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

UObject* UMovementComponent::Duplicate() const
{
    UMovementComponent* ClonedActor = FObjectFactory::ConstructObjectFrom<UMovementComponent>(this);
    ClonedActor->DuplicateSubObjects(this);
    ClonedActor->PostDuplicate();
    return ClonedActor;
}

void UMovementComponent::DuplicateSubObjects(const UObject* Source)
{
    UActorComponent::DuplicateSubObjects(Source);
}

void UMovementComponent::PostDuplicate()
{
}

bool UMovementComponent::MoveUpdatedComponent(FVector Delta, FQuat& NewRotation)
{
    if (!UpdatedComponent)
        return false;

    UpdatedComponent->AddLocation(Delta);
    UpdatedComponent->SetRelativeQuat(NewRotation);
    return true;
}
