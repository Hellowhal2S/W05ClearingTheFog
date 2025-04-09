struct AABBData
{
    float3 Position;
    float3 Extent;
};
cbuffer ConstantBufferDebugAABB : register(b13)
{
    AABBData DataAABB[8];
}

struct SphereData
{
    float3 Position;
    float Radius;
};
cbuffer ConstantBufferDebugSphere : register(b13)
{
    SphereData DataSphere[8];
}

struct ConeData
{
    float3 ApexPosiiton;
    float Radius;
    float3 Direction;
    float Height;
};
cbuffer ConstantBufferDebugCone : register(b13)
{
    ConeData DataCone[8];
}

cbuffer ConstantBufferDebugGrid : register(b13)
{
    row_major matrix InverseViewProj;
}

cbuffer ConstantBufferDebugIcon : register(b13)
{
    float3 IconPosition;
    float IconScale;
}

cbuffer ConstantBufferDebugArrow : register(b13)
{
    float3 ArrowPosition;
    float ArrowScaleXYZ;
    float3 ArrowDirection;
    float ArrowScaleZ;
}