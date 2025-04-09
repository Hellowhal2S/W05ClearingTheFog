#pragma once
#include "TextRenderComponent.h"

class UTextUUID : public UTextRenderComponent
{
    DECLARE_CLASS(UTextUUID, UTextRenderComponent)

public:
    UTextUUID();
    virtual ~UTextUUID() override;

    virtual int CheckRayIntersection(
        FVector& rayOrigin,
        FVector& rayDirection, float& pfNearHitDistance
    ) override;
    void SetUUID(uint32 UUID);
};
