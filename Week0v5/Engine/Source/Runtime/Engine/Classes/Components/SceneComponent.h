#pragma once
#include "ActorComponent.h"
#include "Math/Quat.h"
#include "UObject/ObjectMacros.h"

class USceneComponent : public UActorComponent
{
    DECLARE_CLASS(USceneComponent, UActorComponent)

public:
    USceneComponent();
    USceneComponent(const USceneComponent& Other);
    virtual ~USceneComponent() override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance);
    virtual FVector GetForwardVector();
    virtual FVector GetRightVector();
    virtual FVector GetUpVector();
    void AddLocation(FVector _added);
    void AddRotation(FVector _added);
    void AddScale(FVector _added);

private:
    FVector RelativeLocation;
    FVector RelativeRotation;
    FQuat QuatRotation;
    FVector RelativeScale3D;

    USceneComponent* AttachParent = nullptr;
    TArray<USceneComponent*> AttachChildren;

public:
    FVector GetRelativeLocation() const { return RelativeLocation; }
    FVector GetRelativeRotation() const { return RelativeRotation; }
    FQuat GetRelativeQuat() const { return QuatRotation; }
    FVector GetRelativeScale() const { return RelativeScale3D; }
    FMatrix GetRelativeTransform() const;

    FVector GetComponentLocation() const;
    FVector GetComponentRotation() const;
    FQuat GetComponentQuat() const;
    FVector GetComponentScale() const;
    FMatrix GetComponentTransform() const;

    FMatrix GetComponentTranslateMatrix() const;
    FMatrix GetComponentRotationMatrix() const;
    FMatrix GetComponentScaleMatrix() const;


    //virtual FVector GetWorldRotation();
    //FVector GetWorldScale();
    //FVector GetWorldLocation();
    //FVector GetLocalRotation();

    FVector GetLocalScale() const { return RelativeScale3D; }
    FVector GetLocalLocation() const { return RelativeLocation; }

    void SetRelativeLocation(FVector _newLoc);
    void SetRelativeRotation(FVector _newRot);
    void SetRelativeQuat(FQuat _newRot);
    void SetRelativeScale(FVector _newScale);
    void SetupAttachment(USceneComponent* InParent);
public:

    USceneComponent* GetAttachParent() const;
    void SetAttachParent(USceneComponent* InParent);
    TArray<USceneComponent*> GetAttachChildren() const { return AttachChildren; }
    virtual UObject* Duplicate() const override;
    virtual void DuplicateSubObjects(const UObject* Source) override;
    virtual void PostDuplicate();


private:
    class UTextUUID* uuidText = nullptr;

protected:
    // AABB용 더티비트
    bool bIsChangedForAABB : 1 = true;
};
