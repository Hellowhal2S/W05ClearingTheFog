#include "MovementComponent.h"

#include "UObject/ObjectFactory.h"
#include "UObject/Casts.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "Math/JungleMath.h"

UMovementComponent::UMovementComponent()
{
}

UMovementComponent::UMovementComponent(const UMovementComponent& other)
    : UActorComponent(other),
    Velocity(other.Velocity)
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
    UMovementComponent* ClonedComponent = FObjectFactory::ConstructObjectFrom<UMovementComponent>(this);
    ClonedComponent->DuplicateSubObjects(this);
    ClonedComponent->PostDuplicate();
    return ClonedComponent;
}

void UMovementComponent::DuplicateSubObjects(const UObject* Source)
{
}

void UMovementComponent::PostDuplicate()
{
}

bool UMovementComponent::MoveUpdatedComponent(const FVector Delta, const FQuat& NewRotation)
{
    if (!UpdatedComponent)
        return false;

    UpdatedComponent->AddLocation(Delta);
    UpdatedComponent->SetRelativeQuat(NewRotation);
    return true;
}
