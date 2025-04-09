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
    MaxSpeed(other.MaxSpeed),
    Acceleration(other.Acceleration)
{
}

void UProjectileMovementComponent::BeginPlay()
{
    Super::BeginPlay();
    if (InitialSpeed != 0)
    {
        if (Velocity.x == 0 && Velocity.y == 0 && Velocity.z == 0)
        {
            return;
        }
        else
        {
            Velocity = Velocity.Normalize();
            Velocity *= InitialSpeed;
        }
    }
}

void UProjectileMovementComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

    if (!UpdatedComponent)
        return;

    Velocity += Acceleration * DeltaTime; // Gravity

    FVector MoveDelta = Velocity * DeltaTime;
    FQuat Rotation = UpdatedComponent->GetRelativeQuat();

    MoveUpdatedComponent(MoveDelta, Rotation);
}

UObject* UProjectileMovementComponent::Duplicate() const
{
    UProjectileMovementComponent* ClonedComponent = FObjectFactory::ConstructObjectFrom<UProjectileMovementComponent>(this);
    ClonedComponent->DuplicateSubObjects(this);
    ClonedComponent->PostDuplicate();
    return ClonedComponent;
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
