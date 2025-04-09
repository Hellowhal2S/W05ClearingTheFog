#include "RotatingMovementComponent.h"
#include "Math/JungleMath.h"
#include "UObject/ObjectFactory.h"

URotatingMovementComponent::URotatingMovementComponent()
{
}

URotatingMovementComponent::~URotatingMovementComponent()
{
}

URotatingMovementComponent::URotatingMovementComponent(const URotatingMovementComponent& other)
    : UMovementComponent(other),
    RotationRate(other.RotationRate),
    PivotTranslation(other.PivotTranslation),
    bRotationInLocalSpace(other.bRotationInLocalSpace)
{
}

void URotatingMovementComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime); 
    if (!UpdatedComponent)
        return;

    //DeltaTime /= 16.f;

    const FQuat OldRotation = UpdatedComponent->GetComponentQuat();
    const FQuat DeltaRotation = JungleMath::EulerToQuaternion(RotationRate * DeltaTime);
    const FQuat NewRotation = bRotationInLocalSpace ? (OldRotation * DeltaRotation) : (DeltaRotation * OldRotation);

    FVector DeltaLocation = FVector::ZeroVector;
    if (!PivotTranslation.IsZero())
    {
        const FVector OldPivot = OldRotation.RotateVector(PivotTranslation);
        const FVector NewPivot = NewRotation.RotateVector(PivotTranslation);
        DeltaLocation = (OldPivot - NewPivot); // ConstrainDirectionToPlane() not necessary because it's done by MoveUpdatedComponent() below.
    }
    MoveUpdatedComponent(DeltaLocation, NewRotation);
}

UObject* URotatingMovementComponent::Duplicate() const
{
    URotatingMovementComponent* ClonedComponent = FObjectFactory::ConstructObjectFrom<URotatingMovementComponent>(this);
    ClonedComponent->DuplicateSubObjects(this);
    ClonedComponent->PostDuplicate();
    return ClonedComponent;
}

void URotatingMovementComponent::DuplicateSubObjects(const UObject* Source)
{
    UMovementComponent::DuplicateSubObjects(Source);
}

void URotatingMovementComponent::PostDuplicate()
{
}
