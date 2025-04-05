#include "MovementComponent.h"

#include "UObject/ObjectFactory.h"
#include "UObject/Casts.h"
#include "Components/SceneComponent.h"

UMovementComponent::UMovementComponent()
{
}

UMovementComponent::UMovementComponent(const UMovementComponent& other)
    : UActorComponent(other),
    Velocity(other.Velocity)
{
    UpdatedComponent = Cast<USceneComponent>(other.UpdatedComponent->Duplicate());
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

void UMovementComponent::DuplicateSubObjects(const UObject* SourceObj)
{
    const UMovementComponent* Source = Cast<UMovementComponent>(SourceObj);
    
    if (Source)
    {
        UpdatedComponent = Source->UpdatedComponent;
        Velocity = Source->Velocity;
    }
}

void UMovementComponent::PostDuplicate()
{
}

bool UMovementComponent::MoveUpdatedComponent(FVector Delta, FQuat& NewRotation)
{
    if (!UpdatedComponent)
        return false;

    UpdatedComponent->AddLocation(Delta);
    UpdatedComponent->SetRotation(NewRotation);
    return true;
}
