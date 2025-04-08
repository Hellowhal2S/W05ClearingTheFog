#include "Color.h"

const FLinearColor FLinearColor::White(1.f, 1.f, 1.f);
const FLinearColor FLinearColor::Gray(0.5f, 0.5f, 0.5f);
const FLinearColor FLinearColor::Black(0, 0, 0);
const FLinearColor FLinearColor::Transparent(0, 0, 0, 0);
const FLinearColor FLinearColor::Red(1.f, 0, 0);
const FLinearColor FLinearColor::Green(0, 1.f, 0);
const FLinearColor FLinearColor::Blue(0, 0, 1.f);
const FLinearColor FLinearColor::Yellow(1.f, 1.f, 0);

FLinearColor::FLinearColor(const FVector& Vector) :
    R(Vector.x),
    G(Vector.y),
    B(Vector.z),
    A(1.0f)
{
}

FLinearColor::FLinearColor(const FVector4& Vector) :
    R(Vector.x),
    G(Vector.y),
    B(Vector.z),
    A(Vector.w)
{
}
