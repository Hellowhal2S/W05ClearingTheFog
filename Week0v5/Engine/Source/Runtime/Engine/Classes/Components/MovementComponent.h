#pragma once

#include "ActorComponent.h"
#include "SceneComponent.h"
#include "PrimitiveComponent.h"

class UMovementComponent : public UActorComponent
{
    DECLARE_CLASS(UMovementComponent, UActorComponent)

public:
    UMovementComponent();
    UMovementComponent(const UMovementComponent& other);
    virtual ~UMovementComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual UObject* Duplicate() const override;
    virtual void DuplicateSubObjects(const UObject* Source) override;
    virtual void PostDuplicate() override;

    virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) {
        UpdatedComponent = NewUpdatedComponent;
    }

    bool MoveUpdatedComponent(FVector Delta, FQuat& NewRotation);
    FVector Velocity;
    USceneComponent* UpdatedComponent;
};