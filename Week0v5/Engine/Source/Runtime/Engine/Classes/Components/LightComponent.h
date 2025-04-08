#pragma once
#include "SceneComponent.h"
#include "Define.h"
#include "UObject/ObjectMacros.h"
#include "Math/Color.h"

class ULightComponentBase : public USceneComponent
{
    DECLARE_CLASS(ULightComponentBase, USceneComponent)

public:
    ULightComponentBase() {}
    virtual ~ULightComponentBase() override;

    virtual void TickComponent(float DeltaTime) override;
    PROPERTY(FLinearColor, Color)
    PROPERTY(float, Intensity)

protected:
    FLinearColor Color = FLinearColor::Red;
    float Intensity = 1000.0f;

    //texture2D->SetTexture(L"Assets/Texture/spotLight.png");
};

//////////////////////////////////
// PointLight

class UPointlightComponent : public ULightComponentBase
{
    DECLARE_CLASS(UPointlightComponent, ULightComponentBase)

public:
    UPointlightComponent();
    virtual ~UPointlightComponent() override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;

    virtual UObject* Duplicate() const override;
    virtual void DuplicateSubObjects(const UObject* Source) override;
    virtual void PostDuplicate() override;

    PROPERTY(float, Radius)
    PROPERTY(float, RadiusFallOff)
protected:
    float Radius = 500.f;
    float RadiusFallOff = 2.f;
};

//////////////////////////////////
// DirectionalLight

class UDirectionalLightComponent : public ULightComponentBase
{
    DECLARE_CLASS(UDirectionalLightComponent, ULightComponentBase)

public:
    UDirectionalLightComponent();
    virtual ~UDirectionalLightComponent() override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;

    virtual UObject* Duplicate() const override;
    virtual void DuplicateSubObjects(const UObject* Source) override;
    virtual void PostDuplicate() override;

    PROPERTY(FVector, Direction)
protected:
    FVector Direction = (0, 0, -1);
};

//////////////////////////////////
// SpotLight

class USpotLightComponent : public ULightComponentBase
{
    DECLARE_CLASS(USpotLightComponent, ULightComponentBase)

public:
    USpotLightComponent();
    virtual ~USpotLightComponent() override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;

    virtual UObject* Duplicate() const override;
    virtual void DuplicateSubObjects(const UObject* Source) override;
    virtual void PostDuplicate() override;

    PROPERTY(FVector, Direction)
    PROPERTY(float, Range)
    PROPERTY(float, FallOff)
protected:
    FVector Direction = (0, 0, -1);
    float Range = 1.f;
    float FallOff = 1.f;
};

