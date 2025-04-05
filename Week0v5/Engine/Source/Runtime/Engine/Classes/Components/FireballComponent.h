#pragma once
#include "PrimitiveComponent.h"
#include "Math/Color.h"

class UFireBallComponent : public UPrimitiveComponent
{
    DECLARE_CLASS(UFireBallComponent, UPrimitiveComponent);

public:
    UFireBallComponent();
    virtual ~UFireBallComponent() override;
    UFireBallComponent(const UFireBallComponent& other);

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;

    virtual UObject* Duplicate() const override;
    virtual void DuplicateSubObjects(const UObject* Source) override;
    virtual void PostDuplicate() override;

private:
    float Intensity;
    float Radius;
    float RadiusFallOff;
    FLinearColor Color;
};
