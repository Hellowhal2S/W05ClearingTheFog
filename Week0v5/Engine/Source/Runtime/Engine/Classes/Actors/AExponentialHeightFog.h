#pragma once
#include "GameFramework/Actor.h"

class UHeightFogComponent;

class AExponentialHeightFog : public AActor
{
    DECLARE_CLASS(AExponentialHeightFog, AActor)
public:
    AExponentialHeightFog();
    ~AExponentialHeightFog();
    AExponentialHeightFog(const AExponentialHeightFog& other);

    UObject* Duplicate() const override;
    void DuplicateSubObjects(const UObject* Source) override;
    void Destroyed() override;
private:
    UHeightFogComponent* fogComponent;

public:
    UHeightFogComponent* GetFogComponent() const { return fogComponent; }
};


