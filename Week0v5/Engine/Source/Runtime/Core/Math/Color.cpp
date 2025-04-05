#include "Color.h"

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
    A(Vector.a)
{
}
