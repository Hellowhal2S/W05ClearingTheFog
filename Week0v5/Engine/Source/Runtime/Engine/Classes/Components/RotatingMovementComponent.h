#pragma once

#include "MovementComponent.h"

class URotatingMovementComponent : public UMovementComponent
{
    DECLARE_CLASS(URotatingMovementComponent, UMovementComponent)

public:
    URotatingMovementComponent();
    virtual ~URotatingMovementComponent() override;
    URotatingMovementComponent(const URotatingMovementComponent& other);
    virtual void TickComponent(float DeltaTime) override;
    virtual UObject* Duplicate() const override;
    virtual void DuplicateSubObjects(const UObject* Source) override;
    virtual void PostDuplicate() override;

    FVector RotationRate;
    FVector PivotTranslation;
    uint32 bRotationInLocalSpace = 1;
};