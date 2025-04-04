#pragma once
#include "Define.h"
#include <d3d11.h>
class UPrimitiveBatch
{
public:
    UPrimitiveBatch();
    ~UPrimitiveBatch();
    static UPrimitiveBatch& GetInstance() {
        static UPrimitiveBatch instance;
        return instance;
    }

public:
    void Release();
    void ClearGrid() {};
    float GetSpacing() { return GridParam.gridSpacing; }
    void GenerateGrid(float spacing, int gridCount);
    void RenderBatch(const FMatrix& View, const FMatrix& Projection);
    void InitializeVertexBuffer();
    void UpdateBoundingBoxResources();
    void ReleaseBoundingBoxResources();
    void UpdateConeResources();
    void ReleaseConeResources();
    void UpdateOBBResources();
    void ReleaseOBBResources();
    void RenderAABB(const FBoundingBox& localAABB, const FVector& center, const FMatrix& modelMatrix);
    void RenderOBB(const FBoundingBox& localAABB, const FVector& center, const FMatrix& modelMatrix);
	
    void AddCone(const FVector& center, float radius, float height, int segments,const FVector4& color, const FMatrix& modelMatri);
	
    // 복사 생성자 및 대입 연산자 삭제
    UPrimitiveBatch(const UPrimitiveBatch&) = delete;
    UPrimitiveBatch& operator=(const UPrimitiveBatch&) = delete;
private:
    ID3D11Buffer* pVertexBuffer;
    ID3D11Buffer* pBoundingBoxBuffer;
    ID3D11ShaderResourceView* pBoundingBoxSRV;
    ID3D11Buffer* pConesBuffer;
    ID3D11Buffer* pOBBBuffer;
    ID3D11ShaderResourceView* pConesSRV;
    ID3D11ShaderResourceView* pOBBSRV;

    size_t allocatedBoundingBoxCapacity;
    size_t allocatedConeCapacity;
    size_t allocatedOBBCapacity;
    TArray<FBoundingBox> BoundingBoxes;
    TArray<FOBB> OrientedBoundingBoxes;
    TArray<FCone> Cones;
    FGridParameters GridParam;
    int ConeSegmentCount = 0;

};