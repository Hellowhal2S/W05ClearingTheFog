#include "ProjectileMovementComponent.h"

#include "UObject/ObjectFactory.h"

UProjectileMovementComponent::UProjectileMovementComponent()
{
}

UProjectileMovementComponent::~UProjectileMovementComponent()
{
}

UProjectileMovementComponent::UProjectileMovementComponent(const UProjectileMovementComponent& other) :
    UMovementComponent(other),
    InitialSpeed(other.InitialSpeed),
    MaxSpeed(other.MaxSpeed)
{
}

void UProjectileMovementComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

    if (!UpdatedComponent)
        return;

    FVector MoveDelta = Velocity * DeltaTime;
    FQuat Rotation = UpdatedComponent->GetQuat();

    MoveUpdatedComponent(MoveDelta, Rotation);
}

UObject* UProjectileMovementComponent::Duplicate() const
{
    UProjectileMovementComponent* ClonedActor = FObjectFactory::ConstructObjectFrom<UProjectileMovementComponent>(this);
    ClonedActor->DuplicateSubObjects(this);
    ClonedActor->PostDuplicate();
    return ClonedActor;
}

void UProjectileMovementComponent::DuplicateSubObjects(const UObject* Source)
{
    UMovementComponent::DuplicateSubObjects(Source);
}

void UProjectileMovementComponent::PostDuplicate()
{}

FVector UProjectileMovementComponent::ComputeVelocity(FVector InitialVelocity, float DeltaTime) const
{
    FVector NewVelocity = InitialVelocity * DeltaTime;
    return NewVelocity;
}

void UProjectileMovementComponent::SetVelocityInLocalSpace(FVector NewVelocity)
{
    if (UpdatedComponent)
    {
        FVector Forward = UpdatedComponent->GetForwardVector();
        Velocity = Forward * NewVelocity.x;
    }
}
