#pragma once
#include "Components/MeshComponent.h"
#include "Mesh/StaticMesh.h"

class UStaticMeshComponent : public UMeshComponent
{
    DECLARE_CLASS(UStaticMeshComponent, UMeshComponent)

public:
    UStaticMeshComponent() = default;
    UStaticMeshComponent(const UStaticMeshComponent& Other);

    virtual UObject* Duplicate() const override;
    virtual void DuplicateSubObjects(const UObject* Source) override;
    virtual void PostDuplicate() override;
    virtual void TickComponent(float DeltaTime) override;

    PROPERTY(int, selectedSubMeshIndex);

    virtual uint32 GetNumMaterials() const override;
    virtual UMaterial* GetMaterial(uint32 ElementIndex) const override;
    virtual uint32 GetMaterialIndex(FName MaterialSlotName) const override;
    virtual TArray<FName> GetMaterialSlotNames() const override;
    virtual void GetUsedMaterials(TArray<UMaterial*>& Out) const override;

    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;
    
    UStaticMesh* GetStaticMesh() const { return staticMesh; }
    void SetStaticMesh(UStaticMesh* value)
    { 
        staticMesh = value;
        OverrideMaterials.SetNum(value->GetMaterials().Num());
        LocalAABB = FBoundingBox(staticMesh->GetRenderData()->BoundingBoxMin, staticMesh->GetRenderData()->BoundingBoxMax);
        WorldAABB = LocalAABB;
    }

    // UScenecomponent에서 setter가 실행되었으면 changed bit를 변경해서
    // AABB 조사 전에 값을 업데이트합니다.
    FBoundingBox GetBoundingBoxWorld() { return WorldAABB; }
    void UpdateWorldAABB();

protected:
    UStaticMesh* staticMesh = nullptr;
    int selectedSubMeshIndex = -1;
private:
    float Timer = 0.0f;

private:
    FBoundingBox WorldAABB;
};