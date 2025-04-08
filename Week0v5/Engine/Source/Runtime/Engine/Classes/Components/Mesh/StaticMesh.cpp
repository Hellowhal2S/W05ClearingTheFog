#include "StaticMesh.h"
#include "Engine/FLoaderOBJ.h"
#include "UObject/ObjectFactory.h"

UStaticMesh::UStaticMesh()
{

}

UStaticMesh::~UStaticMesh()
{
    if (staticMeshRenderData == nullptr) return;

    if (staticMeshRenderData->VertexBuffer) {
        staticMeshRenderData->VertexBuffer->Release();
        staticMeshRenderData->VertexBuffer = nullptr;
    }

    if (staticMeshRenderData->IndexBuffer) {
        staticMeshRenderData->IndexBuffer->Release();
        staticMeshRenderData->IndexBuffer = nullptr;
    }
}

uint32 UStaticMesh::GetMaterialIndex(FName MaterialSlotName) const
{
    for (uint32 materialIndex = 0; materialIndex < materials.Num(); materialIndex++) {
        if (materials[materialIndex]->MaterialSlotName == MaterialSlotName)
            return materialIndex;
    }

    return -1;
}

void UStaticMesh::GetUsedMaterials(TArray<UMaterial*>& Out) const
{
    for (const FStaticMaterial* Material : materials)
    {
        Out.Emplace(Material->Material);
    }
}

void UStaticMesh::SetData(OBJ::FStaticMeshRenderData* renderData)
{
    staticMeshRenderData = renderData;

    uint32 verticeNum = staticMeshRenderData->Vertices.Num();
    if (verticeNum <= 0) return;
    staticMeshRenderData->VertexBuffer = GetEngine()->RenderEngine.Renderer.CreateVertexBuffer(staticMeshRenderData->Vertices, verticeNum * sizeof(FVertexSimple));

    uint32 indexNum = staticMeshRenderData->Indices.Num();
    if (indexNum > 0)
        staticMeshRenderData->IndexBuffer = GetEngine()->RenderEngine.Renderer.CreateIndexBuffer(staticMeshRenderData->Indices, indexNum * sizeof(uint32));

    for (int materialIndex = 0; materialIndex < staticMeshRenderData->Materials.Num(); materialIndex++) {
        FStaticMaterial* newMaterialSlot = new FStaticMaterial();
        UMaterial* newMaterial = FManagerOBJ::CreateMaterial(staticMeshRenderData->Materials[materialIndex]);

        newMaterialSlot->Material = newMaterial;
        newMaterialSlot->MaterialSlotName = staticMeshRenderData->Materials[materialIndex].MTLName;

        materials.Add(newMaterialSlot);
    }
}
