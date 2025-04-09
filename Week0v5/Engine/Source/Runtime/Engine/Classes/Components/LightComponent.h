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
    ULightComponentBase(const ULightComponentBase& Other);

    virtual void TickComponent(float DeltaTime) override;
    PROPERTY(FLinearColor, Color)
    PROPERTY(float, Intensity)

protected:
    FLinearColor Color = FLinearColor::White;
    float Intensity = 1000.0f;

    //texture2D->SetTexture(L"Assets/Texture/spotLight.png");
};

//////////////////////////////////
// PointLight

class UPointLightComponent : public ULightComponentBase
{
    DECLARE_CLASS(UPointLightComponent, ULightComponentBase)

public:
    UPointLightComponent();
    virtual ~UPointLightComponent() override;
    UPointLightComponent(const UPointLightComponent& Other);

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;

    virtual UObject* Duplicate() const override;
    virtual void DuplicateSubObjects(const UObject* Source) override;
    virtual void PostDuplicate() override;

    PROPERTY(float, Radius)
    PROPERTY(float, RadiusFallOff)
protected:
    float Radius = 5.f;
    float RadiusFallOff = 0.1f;
};

//////////////////////////////////
// DirectionalLight

class UDirectionalLightComponent : public ULightComponentBase
{
    DECLARE_CLASS(UDirectionalLightComponent, ULightComponentBase)

public:
    UDirectionalLightComponent();
    virtual ~UDirectionalLightComponent() override;
    UDirectionalLightComponent(const UDirectionalLightComponent& Other);

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;

    virtual UObject* Duplicate() const override;
    virtual void DuplicateSubObjects(const UObject* Source) override;
    virtual void PostDuplicate() override;

    FVector GetLightDirection() { return -GetUpVector(); }
};

//////////////////////////////////
// SpotLight

class USpotLightComponent : public ULightComponentBase
{
    DECLARE_CLASS(USpotLightComponent, ULightComponentBase)

public:
    USpotLightComponent();
    virtual ~USpotLightComponent() override;
    USpotLightComponent(const USpotLightComponent& Other);

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;

    virtual UObject* Duplicate() const override;
    virtual void DuplicateSubObjects(const UObject* Source) override;
    virtual void PostDuplicate() override;

    FVector GetLightDirection() { return -GetUpVector(); }
    PROPERTY(float, InnerRadius)
    PROPERTY(float, OuterRadius)
protected:
    float InnerRadius = 10.f;
    float OuterRadius = 20.f;
};

