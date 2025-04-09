#pragma once

#include "MovementComponent.h"

class UProjectileMovementComponent : public UMovementComponent
{
    DECLARE_CLASS(UProjectileMovementComponent, UMovementComponent)

public:
    UProjectileMovementComponent();
    virtual ~UProjectileMovementComponent() override;
    UProjectileMovementComponent(const UProjectileMovementComponent& other);
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual UObject* Duplicate() const override;
    virtual void DuplicateSubObjects(const UObject* Source) override;
    virtual void PostDuplicate() override;
    virtual FVector ComputeVelocity(FVector InitialVelocity, float DeltaTime) const;
    void SetVelocityInLocalSpace(FVector NewVelocity);
    FVector Acceleration;
    float InitialSpeed;
    float MaxSpeed;
};