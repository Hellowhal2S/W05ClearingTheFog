#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Math/JungleMath.h"
#include "UObject/ObjectFactory.h"
#include "UTextUUID.h"
#include "Math/Quat.h"
USceneComponent::USceneComponent() :RelativeLocation(FVector(0.f, 0.f, 0.f)), RelativeRotation(FVector(0.f, 0.f, 0.f)), RelativeScale3D(FVector(1.f, 1.f, 1.f))
{
}
USceneComponent::USceneComponent(const USceneComponent& Other)
    : UActorComponent(Other),
      AttachParent(nullptr), // 복제 시 복원
      RelativeLocation(Other.RelativeLocation),
      RelativeRotation(Other.RelativeRotation),
      QuatRotation(Other.QuatRotation),
      RelativeScale3D(Other.RelativeScale3D)
{
}
USceneComponent::~USceneComponent()
{
	if (uuidText) delete uuidText;
}
void USceneComponent::InitializeComponent()
{
    Super::InitializeComponent();

}

void USceneComponent::TickComponent(float DeltaTime)
{
	Super::TickComponent(DeltaTime);
}


int USceneComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    FVector center = { 0,0,0 };
    // FEditorRenderer::RenderIcons에서도 수정 필요
    float radius = 5.0f;

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

FVector USceneComponent::GetForwardVector()
{
    FVector4 Forward4 = FVector4(1.f, 0.f, 0.0f, 0.0f);
    Forward4 = FMatrix::TransformVector(Forward4, GetComponentTransform());
    FVector Forward;
    Forward = FVector(Forward4.x, Forward4.y, Forward4.z);
    Forward = Forward.Normalize();

    return Forward;
}

FVector USceneComponent::GetRightVector()
{
    FVector4 Right4 = FVector4(0.f, 0.f, 1.0f, 0.0f);
    Right4 = FMatrix::TransformVector(Right4, GetComponentTransform());
    FVector Right;
    Right = FVector(Right4.x, Right4.y, Right4.z);
    Right = Right.Normalize();

    return Right;
}

FVector USceneComponent::GetUpVector()
{
	FVector4 Up4 = FVector4(0.f, 0.f, 1.0f, 0.0f);
    Up4 = FMatrix::TransformVector(Up4, GetComponentTransform());
    FVector Up;
    Up = FVector(Up4.x, Up4.y, Up4.z);
    Up = Up.Normalize();

    return Up;
}


void USceneComponent::AddLocation(FVector _added)
{
	RelativeLocation = RelativeLocation + _added;
    bIsChangedForAABB = true;
}

void USceneComponent::AddRotation(FVector _added)
{
	RelativeRotation = RelativeRotation + _added;
    bIsChangedForAABB = true;

}

void USceneComponent::AddScale(FVector _added)
{
	RelativeScale3D = RelativeScale3D + _added;
    bIsChangedForAABB = true;

}

//FVector USceneComponent::GetWorldRotation()
//{
//	if (AttachParent)
//	{
//		return FVector(AttachParent->GetLocalRotation() + GetLocalRotation());
//	}
//	else
//		return GetLocalRotation();
//}
//
//FVector USceneComponent::GetWorldScale()
//{
//	return GetLocalScale();
//}
//
//FVector USceneComponent::GetWorldLocation()
//{
//		return GetLocalLocation();
//}
//
//FVector USceneComponent::GetLocalRotation()
//{
//	return JungleMath::QuaternionToEuler(QuatRotation);
//}

FMatrix USceneComponent::GetRelativeTransform() const
{
    return JungleMath::CreateModelMatrix(RelativeLocation, RelativeRotation, RelativeScale3D);
}

FVector USceneComponent::GetComponentLocation() const
{
    if (AttachParent)
    {
        FVector4 CompLoc = AttachParent->GetComponentTransform().TransformFVector4(FVector4(RelativeLocation, 1.0f));
        return FVector(CompLoc.xyz() / CompLoc.w);
    }
    else
    {
        return GetRelativeLocation();
    }
}

FVector USceneComponent::GetComponentRotation() const
{
    return JungleMath::QuaternionToEuler(GetComponentQuat());
}

FQuat USceneComponent::GetComponentQuat() const
{
    if (AttachParent)
    {
        return AttachParent->GetComponentQuat() * QuatRotation;
    }
    else
    {
        return QuatRotation;
    }
}

FVector USceneComponent::GetComponentScale() const
{
    if (AttachParent)
    {
        return GetComponentScale() * RelativeScale3D;
    }
    else
    {
        return RelativeScale3D;
    }
}

FMatrix USceneComponent::GetComponentTransform() const
{
    if (AttachParent)
    {
        return GetRelativeTransform() * AttachParent->GetComponentTransform();
    }
    else
    {
        return GetRelativeTransform();
    }
}

FMatrix USceneComponent::GetComponentTranslateMatrix() const
{
    FMatrix LocMat = FMatrix::CreateTranslationMatrix(RelativeLocation);
    if (AttachParent)
    {
        FMatrix ParentLocMat = AttachParent->GetComponentTranslateMatrix();
        LocMat = LocMat * ParentLocMat;
    }
    return LocMat;
}

FMatrix USceneComponent::GetComponentRotationMatrix() const
{
    FMatrix RotMat = FMatrix::CreateRotation(RelativeRotation.x, RelativeRotation.y, RelativeRotation.z);
    if (AttachParent)
    {
        FMatrix ParentRotMat = AttachParent->GetComponentRotationMatrix();
        RotMat = RotMat * ParentRotMat;
    }
    return RotMat;
}

FMatrix USceneComponent::GetComponentScaleMatrix() const
{
    FMatrix ScaleMat = FMatrix::CreateScale(RelativeScale3D.x, RelativeScale3D.y, RelativeScale3D.z);
    if (AttachParent)
    {
        FMatrix ParentScaleMat = AttachParent->GetComponentScaleMatrix();
        ScaleMat = ScaleMat * ParentScaleMat;
    }
    return ScaleMat;
}

void USceneComponent::SetRelativeLocation(FVector _newLoc)
{
    RelativeLocation = _newLoc; 
    bIsChangedForAABB = true;
}

void USceneComponent::SetRelativeRotation(FVector _newRot)
{
 	RelativeRotation = _newRot;
	QuatRotation = JungleMath::EulerToQuaternion(_newRot);
    bIsChangedForAABB = true;
}

void USceneComponent::SetRelativeQuat(FQuat _newRot)
{ 
    QuatRotation = _newRot; 
    RelativeRotation = JungleMath::QuaternionToEuler(_newRot);
    bIsChangedForAABB = true;
}

void USceneComponent::SetRelativeScale(FVector _newScale)
{ 
    RelativeScale3D = _newScale; 
    bIsChangedForAABB = true;
}

void USceneComponent::SetupAttachment(USceneComponent* InParent)
{
    if (
        InParent != AttachParent                                  // 설정하려는 Parent가 기존의 Parent와 다르거나
        && InParent != this                                       // InParent가 본인이 아니고
        && InParent != nullptr                                    // InParent가 유효한 포인터 이며
        && (
            AttachParent == nullptr                               // AttachParent도 유효하며
            || !AttachParent->AttachChildren.Contains(this)  // 이미 AttachParent의 자식이 아닌 경우
        ) 
    ) {
        SetAttachParent(InParent);
        InParent->AttachChildren.AddUnique(this);
    }
}

void USceneComponent::DetachFromParent()
{
    if (AttachParent)
    {
        AttachParent->AttachChildren.Remove(this);
        AttachParent = nullptr;
    }
}

USceneComponent* USceneComponent::GetAttachParent() const
{
    return AttachParent;
}

void USceneComponent::SetAttachParent(USceneComponent* InParent)
{
    AttachParent = InParent;
}

UObject* USceneComponent::Duplicate() const
{
    USceneComponent* NewComp = FObjectFactory::ConstructObjectFrom<USceneComponent>(this);
    NewComp->DuplicateSubObjects(this);
    NewComp->PostDuplicate();
    return NewComp;
}

void USceneComponent::DuplicateSubObjects(const UObject* Source)
{
}

void USceneComponent::PostDuplicate() {}