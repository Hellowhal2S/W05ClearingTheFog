#pragma once

#include <DirectXMath.h>

struct FVector2D
{
	float x,y;
	FVector2D(float _x = 0, float _y = 0) : x(_x), y(_y) {}

	FVector2D operator+(const FVector2D& rhs) const
	{
		return FVector2D(x + rhs.x, y + rhs.y);
	}
	FVector2D operator-(const FVector2D& rhs) const
	{
		return FVector2D(x - rhs.x, y - rhs.y);
	}
	FVector2D operator*(float rhs) const
	{
		return FVector2D(x * rhs, y * rhs);
	}
	FVector2D operator/(float rhs) const
	{
		return FVector2D(x / rhs, y / rhs);
	}
	FVector2D& operator+=(const FVector2D& rhs)
	{
		x += rhs.x;
		y += rhs.y;
		return *this;
	}
};

// 3D 벡터
struct FVector
{
    float x, y, z;
    FVector(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}

    FVector operator+(const FVector& Other) const;
    FVector& operator+=(const FVector& Other);

    FVector operator-(const FVector& Other) const;
    FVector& operator-=(const FVector& Other);

    FVector operator*(const FVector& Other) const;
    FVector operator*(float Scalar) const;
    FVector& operator*=(float Scalar);

    FVector operator/(const FVector& Other) const;
    FVector operator/(float Scalar) const;
    FVector& operator/=(float Scalar);

    FVector operator-() const;

    // 벡터 내적
    float Dot(const FVector& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    // 벡터 크기
    float Magnitude() const {
        return sqrt(x * x + y * y + z * z);
    }

    // 벡터 정규화
    FVector Normalize() const {
        float mag = Magnitude();
        return (mag > 0) ? FVector(x / mag, y / mag, z / mag) : FVector(0, 0, 0);
    }
    FVector Cross(const FVector& Other) const
    {
        return FVector{
            y * Other.z - z * Other.y,
            z * Other.x - x * Other.z,
            x * Other.y - y * Other.x
        };
    }
    bool IsZero() const
    {
        return x == 0.f && y == 0.f && z == 0.f;
    }
    bool operator==(const FVector& other) const {
        return (x == other.x && y == other.y && z == other.z);
    }

    float Distance(const FVector& other) const {
        // 두 벡터의 차 벡터의 크기를 계산
        return ((*this - other).Magnitude());
    }
    DirectX::XMFLOAT3 ToXMFLOAT3() const
    {
        return DirectX::XMFLOAT3(x, y, z);
    }

    static const FVector ZeroVector;
    static const FVector OneVector;
    static const FVector UpVector;
    static const FVector ForwardVector;
    static const FVector RightVector;
};

inline FVector FVector::operator+(const FVector& Other) const
{
    return { x + Other.x, y + Other.y, z + Other.z };
}

inline FVector& FVector::operator+=(const FVector& Other)
{
    x += Other.x; y += Other.y; z += Other.z;
    return *this;
}

inline FVector FVector::operator-(const FVector& Other) const
{
    return { x - Other.x, y - Other.y, z - Other.z };
}

inline FVector& FVector::operator-=(const FVector& Other)
{
    x -= Other.x; y -= Other.y; z -= Other.z;
    return *this;
}

inline FVector FVector::operator*(const FVector& Other) const
{
    return { x * Other.x, y * Other.y, z * Other.z };
}

inline FVector FVector::operator*(float Scalar) const
{
    return { x * Scalar, y * Scalar, z * Scalar };
}

inline FVector& FVector::operator*=(float Scalar)
{
    x *= Scalar; y *= Scalar; z *= Scalar;
    return *this;
}

inline FVector FVector::operator/(const FVector& Other) const
{
    return { x / Other.x, y / Other.y, z / Other.z };
}

inline FVector FVector::operator/(float Scalar) const
{
    return { x / Scalar, y / Scalar, z / Scalar };
}

inline FVector& FVector::operator/=(float Scalar)
{
    x /= Scalar; y /= Scalar; z /= Scalar;
    return *this;
}

inline FVector FVector::operator-() const
{
    return { -x, -y, -z };
}