#pragma once
#include "UBillboardComponent.h"

class UParticleSubUVComp : public UBillboardComponent
{
    DECLARE_CLASS(UParticleSubUVComp, UBillboardComponent)

public:
    UParticleSubUVComp();
    UParticleSubUVComp(const UParticleSubUVComp& other);

    virtual ~UParticleSubUVComp() override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;

    void SetRowColumnCount(int _cellsPerRow, int _cellsPerColumn);

    UObject* Duplicate() const override;
    void DuplicateSubObjects(const UObject* Source) override;
    ID3D11Buffer* vertexSubUVBuffer;
    UINT numTextVertices;

protected:
    bool bIsLoop = true;

private:
    int indexU = 0;
    int indexV = 0;
    float second = 0;

    int CellsPerRow;
    int CellsPerColumn;

    void UpdateVertexBuffer(const TArray<FVertexTexture>& vertices);
    void CreateSubUVVertexBuffer();
};
