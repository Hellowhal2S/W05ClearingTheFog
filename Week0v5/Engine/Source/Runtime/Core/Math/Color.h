#pragma once
#include "JungleMath.h"

struct FLinearColor
{
    union
    {
        struct
        {
            float R, G, B, A;
        };
        float RGBA[4];
    };

    FLinearColor() = default;
    explicit FLinearColor(const FVector& Vector);
    explicit FLinearColor(const FVector4& Vector);
};

struct FColor
{
public:
    union { struct { uint8 R, G, B, A; }; uint32 Bits; };
};