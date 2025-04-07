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
    FLinearColor(float InR, float InG, float InB, float InA = 1.0f) : R(InR), G(InG), B(InB), A(InA) {}
    explicit FLinearColor(const FVector& Vector);
    explicit FLinearColor(const FVector4& Vector);
    static const FLinearColor White;
    static const FLinearColor Gray;
    static const FLinearColor Black;
    static const FLinearColor Transparent;
    static const FLinearColor Red;
    static const FLinearColor Green;
    static const FLinearColor Blue;
    static const FLinearColor Yellow;
};

struct FColor
{
public:
    union { struct { uint8 R, G, B, A; }; uint32 Bits; };
};