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

int ULightComponentBase::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    FVector center = { 0,0,0 };
    float radius = 1.0f;

    FVector L = rayOrigin - center;

    float a = rayDirection.Dot(rayDirection);
    float b = 2.f * rayDirection.Dot(L);
    float c = L.Dot(L) - radius * radius;

    float discriminant = b * b - 4 * a * c;

    if (discriminant < 0.0f)
        return 0; // 교차 없음

    // 근 접점 t 계산 (두 개의 해 중 더 작은 값)
    float sqrtDiscriminant = sqrtf(discriminant);
    float t1 = (-b - sqrtDiscriminant) / (2.0f * a);
    float t2 = (-b + sqrtDiscriminant) / (2.0f * a);

    float t = (t1 >= 0.0f) ? t1 : ((t2 >= 0.0f) ? t2 : -1.0f);

    if (t >= 0.0f)
    {
        pfNearHitDistance = t;
        return 1;
    }

    return 0;
}


//////////////////////////////////
// PointLight

UPointlightComponent::UPointlightComponent()
{
}

UPointlightComponent::~UPointlightComponent()
{
}

UPointlightComponent::UPointlightComponent(const UPointlightComponent& Other)
    : ULightComponentBase(Other),
    Radius(Other.Radius),
    RadiusFallOff(Other.RadiusFallOff)
{
}

void UPointlightComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UPointlightComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

UObject* UPointlightComponent::Duplicate() const
{
    UPointlightComponent* ClonedActor = FObjectFactory::ConstructObjectFrom<UPointlightComponent>(this);
    ClonedActor->DuplicateSubObjects(this);
    ClonedActor->PostDuplicate();
    return ClonedActor;
}

void UPointlightComponent::DuplicateSubObjects(const UObject* Source)
{
    Super::DuplicateSubObjects(Source);
}

void UPointlightComponent::PostDuplicate()
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
