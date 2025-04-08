#pragma once
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#define _TCHAR_DEFINED
#include <d3d11.h>
#include "EngineBaseTypes.h"
#include "Define.h"
#include "Container/Set.h"
#include "RenderResources.h"

class ULightComponentBase;
class UWorld;
class FGraphicsDevice;
class UMaterial;
struct FStaticMaterial;
class UObject;
class FEditorViewportClient;
class UBillboardComponent;
class UStaticMeshComponent;
class UGizmoBaseComponent;
class FRenderer 
{
    friend class UPrimitiveBatch;
    friend class FEditorRenderer;
public:
    void Initialize(FGraphicsDevice* graphics);
    void Release();
    void Render(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport);
    void ChangeViewMode(EViewModeIndex evi);


    // 이거는 추후 RenderResources에 함수로 넣기
    ID3D11Buffer* CreateVertexBuffer(FVertexSimple* vertices, UINT byteWidth) const;
    ID3D11Buffer* CreateVertexBuffer(const TArray<FVertexSimple>& vertices, UINT byteWidth) const;
    ID3D11Buffer* CreateVertexBuffer(FVertexTexture* vertices, UINT byteWidth) const;
    ID3D11Buffer* CreateIndexBuffer(uint32* indices, UINT byteWidth) const;
    ID3D11Buffer* CreateIndexBuffer(const TArray<uint32>& indices, UINT byteWidth) const;

    ID3D11Buffer* GetLightConstantBuffer() const { return RenderResources.ConstantBuffers.StaticMesh.Light01; }
private:
    // Resources
    FRenderResources RenderResources;

    float litFlag =1;
    FGraphicsDevice* Graphics;

    //uint32 Stride;

    void PrepareShader(FShaderResource ShaderResource) const;
    void PrepareConstantbufferStaticMesh(/*FConstantBuffersStaticMesh Constantbuffers*/);
    //Render
    void RenderPrimitive(const FMatrix& ModelMatrix, OBJ::FStaticMeshRenderData* renderData, TArray<FStaticMaterial*> materials, TArray<UMaterial*> overrideMaterial, int selectedSubMeshIndex) const;
    void RenderPrimitive(ID3D11Buffer* pBuffer, UINT numVertices) const;
    void RenderPrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11Buffer* pIndexBuffer, UINT numIndices) const;
   
    void RenderTexturedModelPrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11Buffer* pIndexBuffer, UINT numIndices, ID3D11ShaderResourceView* InTextureSRV, ID3D11SamplerState* InSamplerState) const;
    //Release
    void ReleaseShaders();
    void ReleaseBuffer(ID3D11Buffer*& Buffer) const;
    void ReleaseConstantBuffers();

    void CreateShaders();

    void CreateMeshShader();
    
    //  Constant Buffers
    void CreateConstantBuffers();
    void UpdateConstantbufferMesh(FConstantBufferMesh Buffer) const;
    void UpdateConstantbufferTexture(FConstantBufferTexture Buffer) const;
    void UpdateConstantbufferActor(FConstantBufferActor Buffer) const;
    void UpdateConstantbufferLights(FConstantBufferLights Buffer) const;
    void UpdateConstantbufferCamera(FConstantBufferCamera Buffer) const;

    uint32 TextureStride;


    void CreateTextureShader();
    void PrepareTextureShader() const;
    ID3D11Buffer* CreateVertexTextureBuffer(FVertexTexture* vertices, UINT byteWidth) const;
    ID3D11Buffer* CreateIndexTextureBuffer(uint32* indices, UINT byteWidth) const;
    void RenderTexturePrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices,
        ID3D11Buffer* pIndexBuffer, UINT numIndices,
        ID3D11ShaderResourceView* _TextureSRV,
        ID3D11SamplerState* _SamplerState) const;
    void RenderTextPrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices,
        ID3D11ShaderResourceView* _TextureSRV,
        ID3D11SamplerState* _SamplerState) const;

    void UpdateSubUVConstant(float _indexU, float _indexV) const;
    void PrepareSubUVConstant() const;

    //Render Pass Demo
    void PreparePrimitives();
    void ClearRenderArr();
    void RenderStaticMeshes(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport);
    void RenderBillboards(UWorld* World,std::shared_ptr<FEditorViewportClient> ActiveViewport);
    void RenderPostProcess();
};

