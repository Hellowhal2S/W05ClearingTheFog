#include "LightComponent.h"
#include "UBillboardComponent.h"
#include "Math/JungleMath.h"
#include "UnrealEd/PrimitiveBatch.h"
#include "UObject/ObjectFactory.h"


ULightComponentBase::~ULightComponentBase()
{
}

ULightComponentBase::ULightComponentBase(const ULightComponentBase& Other) :
    USceneComponent(Other),
    Color(Other.Color),
    Intensity(Other.Intensity)
{
}

void ULightComponentBase::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}




//////////////////////////////////
// PointLight

UPointLightComponent::UPointLightComponent()
{
}

UPointLightComponent::~UPointLightComponent()
{
}

UPointLightComponent::UPointLightComponent(const UPointLightComponent& Other)
    : ULightComponentBase(Other),
    Radius(Other.Radius),
    RadiusFallOff(Other.RadiusFallOff)
{
}

void UPointLightComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UPointLightComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

UObject* UPointLightComponent::Duplicate() const
{
    UPointLightComponent* ClonedActor = FObjectFactory::ConstructObjectFrom<UPointLightComponent>(this);
    ClonedActor->DuplicateSubObjects(this);
    ClonedActor->PostDuplicate();
    return ClonedActor;
}

void UPointLightComponent::DuplicateSubObjects(const UObject* Source)
{
    Super::DuplicateSubObjects(Source);
}

void UPointLightComponent::PostDuplicate()
{
    Super::PostDuplicate();
}


//////////////////////////////////
// DirectionalLight

UDirectionalLightComponent::UDirectionalLightComponent()
{
}

UDirectionalLightComponent::~UDirectionalLightComponent()
{
}

UDirectionalLightComponent::UDirectionalLightComponent(const UDirectionalLightComponent& Other)
    : ULightComponentBase(Other)
{
}

void UDirectionalLightComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UDirectionalLightComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

UObject* UDirectionalLightComponent::Duplicate() const
{
    UDirectionalLightComponent* ClonedActor = FObjectFactory::ConstructObjectFrom<UDirectionalLightComponent>(this);
    ClonedActor->DuplicateSubObjects(this);
    ClonedActor->PostDuplicate();
    return ClonedActor;
}

void UDirectionalLightComponent::DuplicateSubObjects(const UObject* Source)
{
    Super::DuplicateSubObjects(Source);
}

void UDirectionalLightComponent::PostDuplicate()
{
    Super::PostDuplicate();
}


//////////////////////////////////
// SpotLight

USpotLightComponent::USpotLightComponent()
{
}

USpotLightComponent::~USpotLightComponent()
{
}

USpotLightComponent::USpotLightComponent(const USpotLightComponent& Other)
    : ULightComponentBase(Other), InnerRadius(Other.InnerRadius), OuterRadius(Other.OuterRadius)
{
}

void USpotLightComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void USpotLightComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

UObject* USpotLightComponent::Duplicate() const
{
    USpotLightComponent* ClonedActor = FObjectFactory::ConstructObjectFrom<USpotLightComponent>(this);
    ClonedActor->DuplicateSubObjects(this);
    ClonedActor->PostDuplicate();
    return ClonedActor;
}

void USpotLightComponent::DuplicateSubObjects(const UObject* Source)
{
    Super::DuplicateSubObjects(Source);
}

void USpotLightComponent::PostDuplicate()
{
    Super::PostDuplicate();
}
