#include "Renderer.h"
#include <d3dcompiler.h>

#include "Engine/World.h"
#include "Actors/Player.h"
#include "BaseGizmos/GizmoBaseComponent.h"
#include "Components/LightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/UBillboardComponent.h"
#include "Components/UParticleSubUVComp.h"
#include "Components/UText.h"
#include "Components/Material/Material.h"
#include "Components/FireBallComponent.h"
#include "D3D11RHI/GraphicDevice.h"
#include "Launch/EditorEngine.h"
#include "Math/JungleMath.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UnrealEd/PrimitiveBatch.h"
#include "UObject/Casts.h"
#include "UObject/Object.h"
#include "PropertyEditor/ShowFlags.h"
#include "UObject/UObjectIterator.h"
#include "Components/SkySphereComponent.h"
#include "PostEffect.h"

#define SAFE_RELEASE(x) if (x) { x->Release(); x = nullptr; }

void FRenderer::Initialize(FGraphicsDevice* graphics)
{
    Graphics = graphics;
    CreateShaders();

    CreateConstantBuffers();
    PostEffect::InitCommonStates(Graphics);
}

void FRenderer::Release()
{
    ReleaseShaders();
    ReleaseConstantBuffers();

    PostEffect::Release();
}

void FRenderer::CreateMeshShader()
{
    ID3DBlob* VertexShaderCSO;
    ID3DBlob* PixelShaderCSO;

    D3DCompileFromFile(L"Shaders/StaticMeshVertexShader.hlsl", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "mainVS", "vs_5_0", 0, 0, &VertexShaderCSO, nullptr);
    Graphics->Device->CreateVertexShader(VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), nullptr, &RenderResources.Shaders.StaticMesh.Vertex);

    D3DCompileFromFile(L"Shaders/StaticMeshPixelShader.hlsl", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "mainPS", "ps_5_0", 0, 0, &PixelShaderCSO, nullptr);
    Graphics->Device->CreatePixelShader(PixelShaderCSO->GetBufferPointer(), PixelShaderCSO->GetBufferSize(), nullptr, &RenderResources.Shaders.StaticMesh.Pixel);

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"MATERIAL_INDEX", 0, DXGI_FORMAT_R32_UINT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    Graphics->Device->CreateInputLayout(
        layout, ARRAYSIZE(layout), VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), &RenderResources.Shaders.StaticMesh.Layout
    );

    VertexShaderCSO->Release();
    PixelShaderCSO->Release();
}

void FRenderer::ReleaseShaders()
{
    SAFE_RELEASE(RenderResources.Shaders.StaticMesh.Pixel);
    SAFE_RELEASE(RenderResources.Shaders.StaticMesh.Vertex);
    SAFE_RELEASE(RenderResources.Shaders.StaticMesh.Layout);

    SAFE_RELEASE(RenderResources.Shaders.Text.Pixel);
    SAFE_RELEASE(RenderResources.Shaders.Text.Vertex);
    SAFE_RELEASE(RenderResources.Shaders.Text.Layout);

    SAFE_RELEASE(RenderResources.Shaders.Texture.Pixel);
    SAFE_RELEASE(RenderResources.Shaders.Texture.Vertex);
    SAFE_RELEASE(RenderResources.Shaders.Texture.Layout);  
}

void FRenderer::PrepareShader(FShaderResource ShaderResource) const
{
    Graphics->DeviceContext->VSSetShader(ShaderResource.Vertex, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(ShaderResource.Pixel, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(ShaderResource.Layout);
    Graphics->DeviceContext->IASetPrimitiveTopology(ShaderResource.Topology);
}

void FRenderer::PrepareConstantbufferStaticMesh()
{
    if (RenderResources.ConstantBuffers.StaticMesh.Camera00)
    {
        Graphics->DeviceContext->VSSetConstantBuffers(0, 1, &RenderResources.ConstantBuffers.StaticMesh.Camera00);
    }
    if (RenderResources.ConstantBuffers.StaticMesh.Light01)
    {
        Graphics->DeviceContext->PSSetConstantBuffers(1, 1, &RenderResources.ConstantBuffers.StaticMesh.Light01);
    }
    if (RenderResources.ConstantBuffers.StaticMesh.Actor03)
    {
        Graphics->DeviceContext->VSSetConstantBuffers(3, 1, &RenderResources.ConstantBuffers.StaticMesh.Actor03);
        Graphics->DeviceContext->PSSetConstantBuffers(3, 1, &RenderResources.ConstantBuffers.StaticMesh.Actor03);
    }
    if (RenderResources.ConstantBuffers.StaticMesh.Texture05)
    {
        Graphics->DeviceContext->PSSetConstantBuffers(5, 1, &RenderResources.ConstantBuffers.StaticMesh.Texture05);
    }
    if (RenderResources.ConstantBuffers.StaticMesh.Mesh06)
    {
        Graphics->DeviceContext->VSSetConstantBuffers(6, 1, &RenderResources.ConstantBuffers.StaticMesh.Mesh06);
        Graphics->DeviceContext->PSSetConstantBuffers(6, 1, &RenderResources.ConstantBuffers.StaticMesh.Mesh06);
    }
}

void FRenderer::ChangeViewMode(EViewModeIndex evi)
{
    //static EViewModeIndex prev = evi;
    //if (prev == evi) return;
    switch (evi)
    {
    case EViewModeIndex::VMI_Lit:
        litFlag = 1;
        break;
    case EViewModeIndex::VMI_Wireframe:
    case EViewModeIndex::VMI_Unlit:
        litFlag = 0;
    }
}

// legacy 지원용.
//void FRenderer::RenderPrimitive(ID3D11Buffer* pBuffer, UINT numVertices) const
//{
//    UINT offset = 0;
//    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pBuffer, &Stride, &offset);
//    Graphics->DeviceContext->Draw(numVertices, 0);
//}
//
//// legacy 지원용.
//void FRenderer::RenderPrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11Buffer* pIndexBuffer, UINT numIndices) const
//{
//    UINT offset = 0;
//    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &Stride, &offset);
//    Graphics->DeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
//
//    Graphics->DeviceContext->DrawIndexed(numIndices, 0, 0);
//}

void FRenderer::RenderPrimitive(const FMatrix& ModelMatrix, OBJ::FStaticMeshRenderData* renderData, TArray<FStaticMaterial*> materials, TArray<UMaterial*> overrideMaterial, int selectedSubMeshIndex = -1) const
{
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &renderData->VertexBuffer, &renderData->Stride, &offset);

    if (renderData->IndexBuffer)
        Graphics->DeviceContext->IASetIndexBuffer(renderData->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    if (renderData->MaterialSubsets.Num() == 0)
    {
        // no submesh
        Graphics->DeviceContext->DrawIndexed(renderData->Indices.Num(), 0, 0);
    }

    for (int subMeshIndex = 0; subMeshIndex < renderData->MaterialSubsets.Num(); subMeshIndex++)
    {
        int materialIndex = renderData->MaterialSubsets[subMeshIndex].MaterialIndex;

        FConstantBufferMesh buf;
        FMatrix ModelInvTransMatrix = FMatrix::Transpose(FMatrix::Inverse(ModelMatrix));
        buf.ModelMatrix = ModelMatrix;
        buf.ModelInvTransMatrix = ModelInvTransMatrix;

        if (subMeshIndex == selectedSubMeshIndex)
        {
            buf.IsSelectedMesh = 1;
        }
        else
        {
            buf.IsSelectedMesh = 0;
        }

        FObjMaterialInfo MaterialInfo;
        if (overrideMaterial[materialIndex] != nullptr)
        {
            MaterialInfo = overrideMaterial[materialIndex]->GetMaterialInfo();
        }
        else
        {
            MaterialInfo = materials[materialIndex]->Material->GetMaterialInfo();
        }

        buf.Material.DiffuseColor = MaterialInfo.Diffuse;
        buf.Material.TransparencyScalar = MaterialInfo.TransparencyScalar;
        buf.Material.AmbientColor = MaterialInfo.Ambient;
        buf.Material.DensityScalar = MaterialInfo.DensityScalar;
        buf.Material.SpecularColor = MaterialInfo.Specular;
        buf.Material.SpecularScalar = MaterialInfo.SpecularScalar;
        buf.Material.EmmisiveColor = MaterialInfo.Emissive;

        if (MaterialInfo.bHasTexture)
        {
            std::shared_ptr<FTexture> texture = UEditorEngine::resourceMgr.GetTexture(MaterialInfo.DiffuseTexturePath);
            Graphics->DeviceContext->PSSetShaderResources(0, 1, &texture->TextureSRV);
            Graphics->DeviceContext->PSSetSamplers(0, 1, &texture->SamplerState);
        }
        else
        {
            ID3D11ShaderResourceView* nullSRV[1] = {nullptr};
            ID3D11SamplerState* nullSampler[1] = {nullptr};

            Graphics->DeviceContext->PSSetShaderResources(0, 1, nullSRV);
            Graphics->DeviceContext->PSSetSamplers(0, 1, nullSampler);
        }

        UpdateConstantbufferMesh(buf);

        if (renderData->IndexBuffer)
        {
            // index draw
            uint64 startIndex = renderData->MaterialSubsets[subMeshIndex].IndexStart;
            uint64 indexCount = renderData->MaterialSubsets[subMeshIndex].IndexCount;
            Graphics->DeviceContext->DrawIndexed(indexCount, startIndex, 0);
        }
    }
}

//void FRenderer::RenderTexturedModelPrimitive(
//    ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11Buffer* pIndexBuffer, UINT numIndices, ID3D11ShaderResourceView* InTextureSRV,
//    ID3D11SamplerState* InSamplerState
//) const
//{
//    if (!InTextureSRV || !InSamplerState)
//    {
//        Console::GetInstance().AddLog(LogLevel::Warning, "SRV, Sampler Error");
//    }
//    if (numIndices <= 0)
//    {
//        Console::GetInstance().AddLog(LogLevel::Warning, "numIndices Error");
//    }
//    UINT offset = 0;
//    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &Stride, &offset);
//    Graphics->DeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
//
//    //Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//    Graphics->DeviceContext->PSSetShaderResources(0, 1, &InTextureSRV);
//    Graphics->DeviceContext->PSSetSamplers(0, 1, &InSamplerState);
//
//    Graphics->DeviceContext->DrawIndexed(numIndices, 0, 0);
//}

ID3D11Buffer* FRenderer::CreateVertexBuffer(FVertexSimple* vertices, UINT byteWidth) const
{
    // 2. Create a vertex buffer
    D3D11_BUFFER_DESC vertexbufferdesc = {};
    vertexbufferdesc.ByteWidth = byteWidth;
    vertexbufferdesc.Usage = D3D11_USAGE_IMMUTABLE; // will never be updated 
    vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexbufferSRD = {vertices};

    ID3D11Buffer* vertexBuffer;

    HRESULT hr = Graphics->Device->CreateBuffer(&vertexbufferdesc, &vertexbufferSRD, &vertexBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "VertexBuffer Creation faild");
    }
    return vertexBuffer;
}

ID3D11Buffer* FRenderer::CreateVertexBuffer(const TArray<FVertexSimple>& vertices, UINT byteWidth) const
{
    D3D11_BUFFER_DESC vertexbufferdesc = {};
    vertexbufferdesc.ByteWidth = byteWidth;
    vertexbufferdesc.Usage = D3D11_USAGE_IMMUTABLE; // will never be updated 
    vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexbufferSRD;
    vertexbufferSRD.pSysMem = vertices.GetData();

    ID3D11Buffer* vertexBuffer;

    HRESULT hr = Graphics->Device->CreateBuffer(&vertexbufferdesc, &vertexbufferSRD, &vertexBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "VertexBuffer Creation faild");
    }
    return vertexBuffer;
}

ID3D11Buffer* FRenderer::CreateIndexBuffer(uint32* indices, UINT byteWidth) const
{
    D3D11_BUFFER_DESC indexbufferdesc = {};              // buffer�� ����, �뵵 ���� ����
    indexbufferdesc.Usage = D3D11_USAGE_IMMUTABLE;       // immutable: gpu�� �б� �������� ������ �� �ִ�.
    indexbufferdesc.BindFlags = D3D11_BIND_INDEX_BUFFER; // index buffer�� ����ϰڴ�.
    indexbufferdesc.ByteWidth = byteWidth;               // buffer ũ�� ����

    D3D11_SUBRESOURCE_DATA indexbufferSRD = {indices};

    ID3D11Buffer* indexBuffer;

    HRESULT hr = Graphics->Device->CreateBuffer(&indexbufferdesc, &indexbufferSRD, &indexBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "IndexBuffer Creation faild");
    }
    return indexBuffer;
}

ID3D11Buffer* FRenderer::CreateIndexBuffer(const TArray<uint32>& indices, UINT byteWidth) const
{
    D3D11_BUFFER_DESC indexbufferdesc = {};              // buffer�� ����, �뵵 ���� ����
    indexbufferdesc.Usage = D3D11_USAGE_IMMUTABLE;       // immutable: gpu�� �б� �������� ������ �� �ִ�.
    indexbufferdesc.BindFlags = D3D11_BIND_INDEX_BUFFER; // index buffer�� ����ϰڴ�.
    indexbufferdesc.ByteWidth = byteWidth;               // buffer ũ�� ����

    D3D11_SUBRESOURCE_DATA indexbufferSRD;
    indexbufferSRD.pSysMem = indices.GetData();

    ID3D11Buffer* indexBuffer;

    HRESULT hr = Graphics->Device->CreateBuffer(&indexbufferdesc, &indexbufferSRD, &indexBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "IndexBuffer Creation faild");
    }
    return indexBuffer;
}

void FRenderer::ReleaseBuffer(ID3D11Buffer*& Buffer) const
{
    if (Buffer)
    {
        Buffer->Release();
        Buffer = nullptr;
    }
}

void FRenderer::CreateConstantBuffers()
{
    D3D11_BUFFER_DESC ConstantBufferDesc = {};
    ConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    ConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    ConstantBufferDesc.ByteWidth = sizeof(FConstantBufferMesh);
    Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &RenderResources.ConstantBuffers.StaticMesh.Mesh06);

    ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    ConstantBufferDesc.ByteWidth = sizeof(FConstantBufferTexture);
    Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &RenderResources.ConstantBuffers.StaticMesh.Texture05);

    ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    ConstantBufferDesc.ByteWidth = sizeof(FConstantBufferActor);
    Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &RenderResources.ConstantBuffers.StaticMesh.Actor03);

    ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    ConstantBufferDesc.ByteWidth = sizeof(FConstantBufferLights);
    Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &RenderResources.ConstantBuffers.StaticMesh.Light01);
    
    ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    ConstantBufferDesc.ByteWidth = sizeof(FConstantBufferCamera);
    Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &RenderResources.ConstantBuffers.StaticMesh.Camera00);

    // Debug로 넘겨야함
    //// grid line 용
    //ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    //ConstantBufferDesc.ByteWidth = sizeof(FGridParameters) + 0xf & 0xfffffff0;
    //Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &RenderResources.ConstantBuffers.BatchLine.Grid01);

    //ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    //ConstantBufferDesc.ByteWidth = sizeof(FPrimitiveCounts) + 0xf & 0xfffffff0;
    //Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &RenderResources.ConstantBuffers.BatchLine.Counts03);


}

void FRenderer::ReleaseConstantBuffers()
{
    ReleaseBuffer(RenderResources.ConstantBuffers.StaticMesh.Camera00);
    ReleaseBuffer(RenderResources.ConstantBuffers.StaticMesh.Light01);
    ReleaseBuffer(RenderResources.ConstantBuffers.StaticMesh.Actor03);
    ReleaseBuffer(RenderResources.ConstantBuffers.StaticMesh.Texture05);
    ReleaseBuffer(RenderResources.ConstantBuffers.StaticMesh.Mesh06);
}

void FRenderer::CreateShaders()
{
    CreateMeshShader();
    CreateTextureShader();
    CreateLineShader();
}


void FRenderer::UpdateConstantbufferMesh(FConstantBufferMesh Buffer) const
{
    if (RenderResources.ConstantBuffers.StaticMesh.Mesh06)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����

        Graphics->DeviceContext->Map(RenderResources.ConstantBuffers.StaticMesh.Mesh06, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        memcpy(ConstantBufferMSR.pData, &Buffer, sizeof(Buffer));
        Graphics->DeviceContext->Unmap(RenderResources.ConstantBuffers.StaticMesh.Mesh06, 0); // GPU�� �ٽ� ��밡���ϰ� �����
    }
}

void FRenderer::UpdateConstantbufferTexture(FConstantBufferTexture Buffer) const
{
    if (RenderResources.ConstantBuffers.StaticMesh.Texture05)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����

        Graphics->DeviceContext->Map(RenderResources.ConstantBuffers.StaticMesh.Texture05, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        memcpy(ConstantBufferMSR.pData, &Buffer, sizeof(Buffer));
        Graphics->DeviceContext->Unmap(RenderResources.ConstantBuffers.StaticMesh.Texture05, 0); // GPU�� �ٽ� ��밡���ϰ� �����
    }
}

void FRenderer::UpdateConstantbufferActor(FConstantBufferActor Buffer) const
{
    if (RenderResources.ConstantBuffers.StaticMesh.Actor03)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����

        Graphics->DeviceContext->Map(RenderResources.ConstantBuffers.StaticMesh.Actor03, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        memcpy(ConstantBufferMSR.pData, &Buffer, sizeof(Buffer));
        Graphics->DeviceContext->Unmap(RenderResources.ConstantBuffers.StaticMesh.Actor03, 0); // GPU�� �ٽ� ��밡���ϰ� �����
    }
}

void FRenderer::UpdateConstantbufferLights(FConstantBufferLights Buffer) const
{
    if (RenderResources.ConstantBuffers.StaticMesh.Light01)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����

        Graphics->DeviceContext->Map(RenderResources.ConstantBuffers.StaticMesh.Light01, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        memcpy(ConstantBufferMSR.pData, &Buffer, sizeof(Buffer));
        Graphics->DeviceContext->Unmap(RenderResources.ConstantBuffers.StaticMesh.Light01, 0); // GPU�� �ٽ� ��밡���ϰ� �����
    }
}

void FRenderer::UpdateConstantbufferCamera(FConstantBufferCamera Buffer) const
{
    if (RenderResources.ConstantBuffers.StaticMesh.Camera00)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����

        Graphics->DeviceContext->Map(RenderResources.ConstantBuffers.StaticMesh.Camera00, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        memcpy(ConstantBufferMSR.pData, &Buffer, sizeof(Buffer));
        Graphics->DeviceContext->Unmap(RenderResources.ConstantBuffers.StaticMesh.Camera00, 0); // GPU�� �ٽ� ��밡���ϰ� �����
    }
}
//
//void FRenderer::UpdateMaterial(const FObjMaterialInfo& MaterialInfo) const
//{
//    if (MaterialConstantBuffer)
//    {
//        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����
//
//        Graphics->DeviceContext->Map(MaterialConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
//        {
//            FMaterialConstants* constants = static_cast<FMaterialConstants*>(ConstantBufferMSR.pData);
//            constants->DiffuseColor = MaterialInfo.Diffuse;
//            constants->TransparencyScalar = MaterialInfo.TransparencyScalar;
//            constants->AmbientColor = MaterialInfo.Ambient;
//            constants->DensityScalar = MaterialInfo.DensityScalar;
//            constants->SpecularColor = MaterialInfo.Specular;
//            constants->SpecularScalar = MaterialInfo.SpecularScalar;
//            constants->EmmisiveColor = MaterialInfo.Emissive;
//        }
//        Graphics->DeviceContext->Unmap(MaterialConstantBuffer, 0); // GPU�� �ٽ� ��밡���ϰ� �����
//    }
//
//    if (MaterialInfo.bHasTexture == true)
//    {
//        std::shared_ptr<FTexture> texture = UEditorEngine::resourceMgr.GetTexture(MaterialInfo.DiffuseTexturePath);
//        Graphics->DeviceContext->PSSetShaderResources(0, 1, &texture->TextureSRV);
//        Graphics->DeviceContext->PSSetSamplers(0, 1, &texture->SamplerState);
//    }
//    else
//    {
//        ID3D11ShaderResourceView* nullSRV[1] = {nullptr};
//        ID3D11SamplerState* nullSampler[1] = {nullptr};
//
//        Graphics->DeviceContext->PSSetShaderResources(0, 1, nullSRV);
//        Graphics->DeviceContext->PSSetSamplers(0, 1, nullSampler);
//    }
//}
//
//void FRenderer::UpdateLitUnlitConstant(int isLit) const
//{
//    if (FlagBuffer)
//    {
//        D3D11_MAPPED_SUBRESOURCE constantbufferMSR; // GPU �� �޸� �ּ� ����
//        Graphics->DeviceContext->Map(FlagBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR);
//        auto constants = static_cast<FLitUnlitConstants*>(constantbufferMSR.pData); //GPU �޸� ���� ����
//        {
//            constants->isLit = isLit;
//        }
//        Graphics->DeviceContext->Unmap(FlagBuffer, 0);
//    }
//}
//
//void FRenderer::UpdateSubMeshConstant(bool isSelected) const
//{
//    if (SubMeshConstantBuffer) {
//        D3D11_MAPPED_SUBRESOURCE constantbufferMSR; // GPU �� �޸� �ּ� ����
//        Graphics->DeviceContext->Map(SubMeshConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR);
//        FSubMeshConstants* constants = (FSubMeshConstants*)constantbufferMSR.pData; //GPU �޸� ���� ����
//        {
//            constants->isSelectedSubMesh = isSelected;
//        }
//        Graphics->DeviceContext->Unmap(SubMeshConstantBuffer, 0);
//    }
//}
//
//void FRenderer::UpdateTextureConstant(float UOffset, float VOffset)
//{
//    if (TextureConstantBufer) {
//        D3D11_MAPPED_SUBRESOURCE constantbufferMSR; // GPU �� �޸� �ּ� ����
//        Graphics->DeviceContext->Map(TextureConstantBufer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR);
//        FTextureConstants* constants = (FTextureConstants*)constantbufferMSR.pData; //GPU �޸� ���� ����
//        {
//            constants->UOffset = UOffset;
//            constants->VOffset = VOffset;
//        }
//        Graphics->DeviceContext->Unmap(TextureConstantBufer, 0);
//    }
//}

void FRenderer::CreateTextureShader()
{
    ID3DBlob* vertextextureshaderCSO;
    ID3DBlob* pixeltextureshaderCSO;

    HRESULT hr;
    hr = D3DCompileFromFile(L"Shaders/VertexTextureShader.hlsl", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", 0, 0, &vertextextureshaderCSO, nullptr);
    if (FAILED(hr))
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "VertexShader Error");
    }
    Graphics->Device->CreateVertexShader(
        vertextextureshaderCSO->GetBufferPointer(), vertextextureshaderCSO->GetBufferSize(), nullptr, &RenderResources.Shaders.Texture.Vertex
    );

    hr = D3DCompileFromFile(L"Shaders/PixelTextureShader.hlsl", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, &pixeltextureshaderCSO, nullptr);
    if (FAILED(hr))
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "PixelShader Error");
    }
    Graphics->Device->CreatePixelShader(
        pixeltextureshaderCSO->GetBufferPointer(), pixeltextureshaderCSO->GetBufferSize(), nullptr, &RenderResources.Shaders.Texture.Pixel
    );

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    Graphics->Device->CreateInputLayout(
        layout, ARRAYSIZE(layout), vertextextureshaderCSO->GetBufferPointer(), vertextextureshaderCSO->GetBufferSize(), &RenderResources.Shaders.Texture.Layout
    );

    //�ڷᱸ�� ���� �ʿ�
    TextureStride = sizeof(FVertexTexture);
    vertextextureshaderCSO->Release();
    pixeltextureshaderCSO->Release();
}


void FRenderer::PrepareTextureShader() const
{
    Graphics->DeviceContext->VSSetShader(RenderResources.Shaders.Texture.Vertex, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(RenderResources.Shaders.Texture.Pixel, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(RenderResources.Shaders.Texture.Layout);
}

ID3D11Buffer* FRenderer::CreateVertexTextureBuffer(FVertexTexture* vertices, UINT byteWidth) const
{
    // 2. Create a vertex buffer
    D3D11_BUFFER_DESC vertexbufferdesc = {};
    vertexbufferdesc.ByteWidth = byteWidth;
    vertexbufferdesc.Usage = D3D11_USAGE_DYNAMIC; // will never be updated 
    vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    //D3D11_SUBRESOURCE_DATA vertexbufferSRD = { vertices };

    ID3D11Buffer* vertexBuffer;

    HRESULT hr = Graphics->Device->CreateBuffer(&vertexbufferdesc, nullptr, &vertexBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "VertexBuffer Creation faild");
    }
    return vertexBuffer;
}

ID3D11Buffer* FRenderer::CreateIndexTextureBuffer(uint32* indices, UINT byteWidth) const
{
    D3D11_BUFFER_DESC indexbufferdesc = {};
    indexbufferdesc.Usage = D3D11_USAGE_DYNAMIC;
    indexbufferdesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexbufferdesc.ByteWidth = byteWidth;
    indexbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    ID3D11Buffer* indexBuffer;

    HRESULT hr = Graphics->Device->CreateBuffer(&indexbufferdesc, nullptr, &indexBuffer);
    if (FAILED(hr))
    {
        return nullptr;
    }
    return indexBuffer;
}

void FRenderer::RenderTexturePrimitive(
    ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11Buffer* pIndexBuffer, UINT numIndices, ID3D11ShaderResourceView* _TextureSRV,
    ID3D11SamplerState* _SamplerState
) const
{
    if (!_TextureSRV || !_SamplerState)
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "SRV, Sampler Error");
    }
    if (numIndices <= 0)
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "numIndices Error");
    }
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &TextureStride, &offset);
    Graphics->DeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics->DeviceContext->PSSetShaderResources(0, 1, &_TextureSRV);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &_SamplerState);

    Graphics->DeviceContext->DrawIndexed(numIndices, 0, 0);
}

//��Ʈ ��ġ������
void FRenderer::RenderTextPrimitive(
    ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11ShaderResourceView* _TextureSRV, ID3D11SamplerState* _SamplerState
) const
{
    if (!_TextureSRV || !_SamplerState)
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "SRV, Sampler Error");
    }
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &TextureStride, &offset);

    // �Է� ���̾ƿ� �� �⺻ ����
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics->DeviceContext->PSSetShaderResources(0, 1, &_TextureSRV);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &_SamplerState);

    // ��ο� ȣ�� (6���� �ε��� ���)
    Graphics->DeviceContext->Draw(numVertices, 0);
}


ID3D11Buffer* FRenderer::CreateVertexBuffer(FVertexTexture* vertices, UINT byteWidth) const
{
    // 2. Create a vertex buffer
    D3D11_BUFFER_DESC vertexbufferdesc = {};
    vertexbufferdesc.ByteWidth = byteWidth;
    vertexbufferdesc.Usage = D3D11_USAGE_IMMUTABLE; // will never be updated 
    vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexbufferSRD = {vertices};

    ID3D11Buffer* vertexBuffer;

    HRESULT hr = Graphics->Device->CreateBuffer(&vertexbufferdesc, &vertexbufferSRD, &vertexBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "VertexBuffer Creation faild");
    }
    return vertexBuffer;
}

void FRenderer::UpdateSubUVConstant(float _indexU, float _indexV) const
{
    if (RenderResources.ConstantBuffers.StaticMesh.Texture05)
    {
        FConstantBufferTexture buf;
        buf.UVOffset = { _indexU, _indexV };
        UpdateConstantbufferTexture(buf);
    }
}

void FRenderer::PrepareSubUVConstant() const
{
    if (RenderResources.ConstantBuffers.StaticMesh.Texture05)
    {
        //Graphics->DeviceContext->VSSetConstantBuffers(5, 1, &RenderResources.ConstantBuffers.StaticMesh.Texture05);
        Graphics->DeviceContext->PSSetConstantBuffers(5, 1, &RenderResources.ConstantBuffers.StaticMesh.Texture05);
    }
}

//void FRenderer::PrepareLineShader() const
//{
//    // ���̴��� �Է� ���̾ƿ� ����
//    Graphics->DeviceContext->VSSetShader(VertexLineShader, nullptr, 0);
//    Graphics->DeviceContext->PSSetShader(PixelLineShader, nullptr, 0);
//
//    // ��� ���� ���ε�: 
//    // - MatrixBuffer�� register(b0)��, Vertex Shader�� ���ε�
//    // - GridConstantBuffer�� register(b1)��, Vertex�� Pixel Shader�� ���ε� (�ȼ� ���̴��� �ʿ信 ����)
//    if (RenderResources.ConstantBuffers.StaticMesh.Mesh06 && GridConstantBuffer)
//    {
//        //Graphics->DeviceContext->VSSetConstantBuffers(0, 1, &RenderResources.ConstantBuffers.StaticMesh.Mesh06);     // MatrixBuffer (b0)
//        Graphics->DeviceContext->VSSetConstantBuffers(1, 1, &GridConstantBuffer); // GridParameters (b1)
//        Graphics->DeviceContext->PSSetConstantBuffers(1, 1, &GridConstantBuffer);
//        Graphics->DeviceContext->VSSetConstantBuffers(3, 1, &LinePrimitiveBuffer);
//        Graphics->DeviceContext->VSSetShaderResources(2, 1, &pBBSRV);
//        Graphics->DeviceContext->VSSetShaderResources(3, 1, &pConeSRV);
//        Graphics->DeviceContext->VSSetShaderResources(4, 1, &pOBBSRV);
//    }
//}

void FRenderer::CreateLineShader()
{
    //ID3DBlob* VertexShaderLine;
    //ID3DBlob* PixelShaderLine;

    //HRESULT hr;
    //hr = D3DCompileFromFile(L"Shaders/ShaderLine.hlsl", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "mainVS", "vs_5_0", 0, 0, &VertexShaderLine, nullptr);
    //if (FAILED(hr))
    //{
    //    Console::GetInstance().AddLog(LogLevel::Warning, "VertexShader Error");
    //}
    //Graphics->Device->CreateVertexShader(VertexShaderLine->GetBufferPointer(), VertexShaderLine->GetBufferSize(), nullptr, &RenderResources.Shaders.Line.Vertex);

    //hr = D3DCompileFromFile(L"Shaders/ShaderLine.hlsl", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "mainPS", "ps_5_0", 0, 0, &PixelShaderLine, nullptr);
    //if (FAILED(hr))
    //{
    //    Console::GetInstance().AddLog(LogLevel::Warning, "PixelShader Error");
    //}
    //Graphics->Device->CreatePixelShader(PixelShaderLine->GetBufferPointer(), PixelShaderLine->GetBufferSize(), nullptr, &RenderResources.Shaders.Line.Pixel);


    //VertexShaderLine->Release();
    //PixelShaderLine->Release();
}

ID3D11Buffer* FRenderer::CreateStaticVerticesBuffer() const
{
    FSimpleVertex vertices[2]{{0}, {0}};

    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.ByteWidth = sizeof(vertices);
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA vbInitData = {};
    vbInitData.pSysMem = vertices;
    ID3D11Buffer* pVertexBuffer = nullptr;
    HRESULT hr = Graphics->Device->CreateBuffer(&vbDesc, &vbInitData, &pVertexBuffer);
    return pVertexBuffer;
}

ID3D11Buffer* FRenderer::CreateBoundingBoxBuffer(UINT numBoundingBoxes) const
{
    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC; // ���� ������Ʈ�� ��� DYNAMIC, �׷��� ������ DEFAULT
    bufferDesc.ByteWidth = sizeof(FBoundingBox) * numBoundingBoxes;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(FBoundingBox);

    ID3D11Buffer* BoundingBoxBuffer = nullptr;
    Graphics->Device->CreateBuffer(&bufferDesc, nullptr, &BoundingBoxBuffer);
    return BoundingBoxBuffer;
}

ID3D11Buffer* FRenderer::CreateOBBBuffer(UINT numBoundingBoxes) const
{
    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC; // ���� ������Ʈ�� ��� DYNAMIC, �׷��� ������ DEFAULT
    bufferDesc.ByteWidth = sizeof(FOBB) * numBoundingBoxes;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(FOBB);

    ID3D11Buffer* BoundingBoxBuffer = nullptr;
    Graphics->Device->CreateBuffer(&bufferDesc, nullptr, &BoundingBoxBuffer);
    return BoundingBoxBuffer;
}

ID3D11Buffer* FRenderer::CreateConeBuffer(UINT numCones) const
{
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = sizeof(FCone) * numCones;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(FCone);

    ID3D11Buffer* ConeBuffer = nullptr;
    Graphics->Device->CreateBuffer(&bufferDesc, nullptr, &ConeBuffer);
    return ConeBuffer;
}

ID3D11ShaderResourceView* FRenderer::CreateBoundingBoxSRV(ID3D11Buffer* pBoundingBoxBuffer, UINT numBoundingBoxes)
{
    //D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    //srvDesc.Format = DXGI_FORMAT_UNKNOWN; // ����ü ������ ��� UNKNOWN
    //srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    //srvDesc.Buffer.ElementOffset = 0;
    //srvDesc.Buffer.NumElements = numBoundingBoxes;


    //Graphics->Device->CreateShaderResourceView(pBoundingBoxBuffer, &srvDesc, &RenderResources.ShaderResourceView.AABB);
    //return RenderResources.ShaderResourceView.AABB;
    return nullptr;
}

ID3D11ShaderResourceView* FRenderer::CreateOBBSRV(ID3D11Buffer* pBoundingBoxBuffer, UINT numBoundingBoxes)
{
    //D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    //srvDesc.Format = DXGI_FORMAT_UNKNOWN; // ����ü ������ ��� UNKNOWN
    //srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    //srvDesc.Buffer.ElementOffset = 0;
    //srvDesc.Buffer.NumElements = numBoundingBoxes;
    //Graphics->Device->CreateShaderResourceView(pBoundingBoxBuffer, &srvDesc, &RenderResources.ShaderResourceView.OBB);
    //return RenderResources.ShaderResourceView.OBB;
    return nullptr;
}

ID3D11ShaderResourceView* FRenderer::CreateConeSRV(ID3D11Buffer* pConeBuffer, UINT numCones)
{
    //D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    //srvDesc.Format = DXGI_FORMAT_UNKNOWN; // ����ü ������ ��� UNKNOWN
    //srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    //srvDesc.Buffer.ElementOffset = 0;
    //srvDesc.Buffer.NumElements = numCones;


    //Graphics->Device->CreateShaderResourceView(pConeBuffer, &srvDesc, &RenderResources.ShaderResourceView.Cone);
    //return RenderResources.ShaderResourceView.Cone;
    return nullptr;
}

void FRenderer::UpdateBoundingBoxBuffer(ID3D11Buffer* pBoundingBoxBuffer, const TArray<FBoundingBox>& BoundingBoxes, int numBoundingBoxes) const
{
    if (!pBoundingBoxBuffer) return;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    Graphics->DeviceContext->Map(pBoundingBoxBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    auto pData = reinterpret_cast<FBoundingBox*>(mappedResource.pData);
    for (int i = 0; i < BoundingBoxes.Num(); ++i)
    {
        pData[i] = BoundingBoxes[i];
    }
    Graphics->DeviceContext->Unmap(pBoundingBoxBuffer, 0);
}

void FRenderer::UpdateOBBBuffer(ID3D11Buffer* pBoundingBoxBuffer, const TArray<FOBB>& BoundingBoxes, int numBoundingBoxes) const
{
    if (!pBoundingBoxBuffer) return;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    Graphics->DeviceContext->Map(pBoundingBoxBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    auto pData = reinterpret_cast<FOBB*>(mappedResource.pData);
    for (int i = 0; i < BoundingBoxes.Num(); ++i)
    {
        pData[i] = BoundingBoxes[i];
    }
    Graphics->DeviceContext->Unmap(pBoundingBoxBuffer, 0);
}

void FRenderer::UpdateConesBuffer(ID3D11Buffer* pConeBuffer, const TArray<FCone>& Cones, int numCones) const
{
    if (!pConeBuffer) return;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    Graphics->DeviceContext->Map(pConeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    auto pData = reinterpret_cast<FCone*>(mappedResource.pData);
    for (int i = 0; i < Cones.Num(); ++i)
    {
        pData[i] = Cones[i];
    }
    Graphics->DeviceContext->Unmap(pConeBuffer, 0);
}

void FRenderer::UpdateGridConstantBuffer(const FGridParameters& gridParams) const
{
    //D3D11_MAPPED_SUBRESOURCE mappedResource;
    //HRESULT hr = Graphics->DeviceContext->Map(RenderResources.ConstantBuffers.BatchLine.Grid01, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    //if (SUCCEEDED(hr))
    //{
    //    memcpy(mappedResource.pData, &gridParams, sizeof(FGridParameters));
    //    Graphics->DeviceContext->Unmap(RenderResources.ConstantBuffers.BatchLine.Grid01, 0);
    //}
    //else
    //{
    //    UE_LOG(LogLevel::Warning, "gridParams ���� ����");
    //}
}

void FRenderer::UpdateLinePrimitveCountBuffer(int numBoundingBoxes, int numCones) const
{
    //D3D11_MAPPED_SUBRESOURCE mappedResource;
    //HRESULT hr = Graphics->DeviceContext->Map(RenderResources.ConstantBuffers.BatchLine.Counts03, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    //auto pData = static_cast<FPrimitiveCounts*>(mappedResource.pData);
    //pData->BoundingBoxCount = numBoundingBoxes;
    //pData->ConeCount = numCones;
    //Graphics->DeviceContext->Unmap(RenderResources.ConstantBuffers.BatchLine.Counts03, 0);
}

void FRenderer::RenderBatch(
    const FGridParameters& gridParam, ID3D11Buffer* pVertexBuffer, int boundingBoxCount, int coneCount, int coneSegmentCount, int obbCount
) const
{
    UINT stride = sizeof(FSimpleVertex);
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    UINT vertexCountPerInstance = 2;
    UINT instanceCount = gridParam.numGridLines + 3 + (boundingBoxCount * 12) + (coneCount * (2 * coneSegmentCount)) + (12 * obbCount);
    Graphics->DeviceContext->DrawInstanced(vertexCountPerInstance, instanceCount, 0, 0);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void FRenderer::PreparePrimitives()
{
    if (GEngine->GetWorld()->WorldType == EWorldType::Editor)
    {
        for (const auto iter : TObjectRange<USceneComponent>())
        {
                UE_LOG(LogLevel::Display, "%d", GUObjectArray.GetObjectItemArrayUnsafe().Num());
                if (UFireBallComponent* pFireBallComp = Cast<UFireBallComponent>(iter))
                {
                    RenderResources.Components.FireBallObjs.Add(pFireBallComp);
                }
                if (UStaticMeshComponent* pStaticMeshComp = Cast<UStaticMeshComponent>(iter))
                {
                    if (!Cast<UGizmoBaseComponent>(iter))
                        RenderResources.Components.StaticMeshObjs.Add(pStaticMeshComp);
                }
                //if (UGizmoBaseComponent* pGizmoComp = Cast<UGizmoBaseComponent>(iter))
                //{
                //    RenderResources.Primitives.GizmoObjs.Add(pGizmoComp);
                //}
                if (UBillboardComponent* pBillboardComp = Cast<UBillboardComponent>(iter))
                {
                    RenderResources.Components.BillboardObjs.Add(pBillboardComp);
                }
                if (ULightComponentBase* pLightComp = Cast<ULightComponentBase>(iter))
                {
                    RenderResources.Components.LightObjs.Add(pLightComp);
                }
        }
        
    }
    else if (GEngine->GetWorld()->WorldType == EWorldType::PIE)
    {
        // UE_LOG(LogLevel::Display, "%d", GEngine->GetWorld()->GetActors().Num() );
        for (const auto iter : GEngine->GetWorld()->GetActors())
        {
            
            for (const auto iter2 : iter->GetComponents())
            {
                if (UFireBallComponent* pFireBallComp = Cast<UFireBallComponent>(iter2))
                {
                    RenderResources.Components.FireBallObjs.Add(pFireBallComp);
                }
                if (UStaticMeshComponent* pStaticMeshComp = Cast<UStaticMeshComponent>(iter2))
                {
                    if (!Cast<UGizmoBaseComponent>(iter2))
                        RenderResources.Components.StaticMeshObjs.Add(pStaticMeshComp);
                }
                if (UBillboardComponent* pBillboardComp = Cast<UBillboardComponent>(iter2))
                {
                    RenderResources.Components.BillboardObjs.Add(pBillboardComp);
                }
                if (ULightComponentBase* pLightComp = Cast<ULightComponentBase>(iter2))
                {
                    RenderResources.Components.LightObjs.Add(pLightComp);
                }
            }
        }
    }
}

void FRenderer::ClearRenderArr()
{
    RenderResources.Components.StaticMeshObjs.Empty();
    //RenderResources.Primitives.GizmoObjs.Empty();
    RenderResources.Components.BillboardObjs.Empty();
    RenderResources.Components.LightObjs.Empty();
    RenderResources.Components.FireBallObjs.Empty();
}

void FRenderer::Render(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    PreparePrimitives();

    Graphics->DeviceContext->RSSetViewports(1, &ActiveViewport->GetD3DViewport());
    Graphics->ChangeRasterizer(ActiveViewport->GetViewMode());
    // Scene Update
    {
        FConstantBufferCamera buf;
        buf.ViewMatrix = ActiveViewport->GetViewMatrix();
        buf.ProjMatrix = ActiveViewport->GetProjectionMatrix();
        UpdateConstantbufferCamera(buf);
    }

    if (!RenderResources.ConstantBuffers.StaticMesh.Light01) return;
    FConstantBufferLights buf;
    buf.DirLights[0].Color.Ambient = FVector(1.0f, 1.0f, 1.0f) * 0.06f;
    buf.DirLights[0].Color.Diffuse = FVector(1.0f, 1.0f, 1.0f) * 0.06f;
    buf.DirLights[0].Color.Specular = FVector(1.0f, 1.0f, 1.0f) * 0.06f;
    static float time = 0;
    time += 0.1;
    buf.DirLights[0].Direction = FVector(sin(time), cos(time), -10.0f).Normalize();
    buf.isLit = litFlag;

    int32 LightIndex = 0;
    for (UFireBallComponent* FireBallComp : RenderResources.Components.FireBallObjs)
    {
        if (LightIndex >= MACRO_FCONSTANT_NUM_MAX_POINTLIGHT)
            break;

        buf.PointLights[LightIndex].Color = FireBallComp->Color;
        buf.PointLights[LightIndex].Position = FireBallComp->GetComponentLocation();
        buf.PointLights[LightIndex].Intensity = FireBallComp->Intensity;
        buf.PointLights[LightIndex].Radius = FireBallComp->Radius;
        buf.PointLights[LightIndex].RadiusFallOff = FireBallComp->RadiusFallOff;
        buf.NumPointLights = ++LightIndex;
    }
    UpdateConstantbufferLights(buf);



    if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_Primitives))
    {
        RenderStaticMeshes(World, ActiveViewport);
    }
    //RenderGizmos(World, ActiveViewport);
    if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_BillboardText))
    {
        RenderBillboards(World, ActiveViewport);
    }
    if (RenderResources.Components.LightObjs.Num() > 0)
    {
        RenderLight(World, ActiveViewport);
    }
    UPrimitiveBatch::GetInstance().RenderBatch(ActiveViewport->GetViewMatrix(), ActiveViewport->GetProjectionMatrix());
    
    ClearRenderArr();
}

void FRenderer::RenderStaticMeshes(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    PrepareShader(RenderResources.Shaders.StaticMesh);
    PrepareConstantbufferStaticMesh();
    for (UStaticMeshComponent* StaticMeshComp : RenderResources.Components.StaticMeshObjs)
    {
        // Actor별 constantbuffer
        {
            FVector4 UUIDColor = StaticMeshComp->EncodeUUID() / 255.0f;
            FConstantBufferActor buf;
            buf.UUID = UUIDColor;
            buf.IsSelectedActor = World->GetSelectedActor() == StaticMeshComp->GetOwner() ? 1 : 0;
            UpdateConstantbufferActor(buf);
        }

        if (USkySphereComponent* skysphere = Cast<USkySphereComponent>(StaticMeshComp))
        {
            FConstantBufferTexture buf;
            buf.UVOffset = { skysphere->UOffset, skysphere->VOffset };
            UpdateConstantbufferTexture(buf);
        }
        else
        {
            UpdateConstantbufferTexture({ {0,0} });
        }

        FMatrix Model = StaticMeshComp->GetComponentTransform();
        if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_AABB))
        {
            UPrimitiveBatch::GetInstance().RenderAABB(
                StaticMeshComp->GetBoundingBoxLocal(),
                StaticMeshComp->GetComponentLocation(),
                Model);
        }
        
        if (!StaticMeshComp->GetStaticMesh()) continue;

        OBJ::FStaticMeshRenderData* renderData = StaticMeshComp->GetStaticMesh()->GetRenderData();
        if (renderData == nullptr) continue;

        RenderPrimitive(Model, renderData, StaticMeshComp->GetStaticMesh()->GetMaterials(), StaticMeshComp->GetOverrideMaterials(), StaticMeshComp->GetselectedSubMeshIndex());
    }
}

//void FRenderer::RenderGizmos(const UWorld* World, const std::shared_ptr<FEditorViewportClient>& ActiveViewport)
//{
//    if (!World->GetSelectedActor())
//    {
//        return;
//    }
//
//    #pragma region GizmoDepth
//        ID3D11DepthStencilState* DepthStateDisable = Graphics->DepthStateDisable;
//        Graphics->DeviceContext->OMSetDepthStencilState(DepthStateDisable, 0);
//    #pragma endregion GizmoDepth
//
//    //  fill solid,  Wirframe 에서도 제대로 렌더링되기 위함
//    Graphics->DeviceContext->RSSetState(UEditorEngine::graphicDevice.RasterizerStateSOLID);
//    
//    for (auto GizmoComp : RenderResources.Primitives.GizmoObjs)
//    {
//        
//        if ((GizmoComp->GetGizmoType()==UGizmoBaseComponent::ArrowX ||
//            GizmoComp->GetGizmoType()==UGizmoBaseComponent::ArrowY ||
//            GizmoComp->GetGizmoType()==UGizmoBaseComponent::ArrowZ)
//            && World->GetEditorPlayer()->GetControlMode() != CM_TRANSLATION)
//            continue;
//        else if ((GizmoComp->GetGizmoType()==UGizmoBaseComponent::ScaleX ||
//            GizmoComp->GetGizmoType()==UGizmoBaseComponent::ScaleY ||
//            GizmoComp->GetGizmoType()==UGizmoBaseComponent::ScaleZ)
//            && World->GetEditorPlayer()->GetControlMode() != CM_SCALE)
//            continue;
//        else if ((GizmoComp->GetGizmoType()==UGizmoBaseComponent::CircleX ||
//            GizmoComp->GetGizmoType()==UGizmoBaseComponent::CircleY ||
//            GizmoComp->GetGizmoType()==UGizmoBaseComponent::CircleZ)
//            && World->GetEditorPlayer()->GetControlMode() != CM_ROTATION)
//            continue;
//        FMatrix Model = GizmoComp->GetComponentTransform();
//        {
//            FConstantBufferActor buf;
//            buf.IsSelectedActor = 0;
//            buf.UUID = GizmoComp->EncodeUUID() / 255.0f;
//            UpdateConstantbufferActor(buf);
//        }
//        {
//            FConstantBufferMesh buf;
//            buf.ModelMatrix = Model;
//            if (GizmoComp == World->GetPickingGizmo())
//            {
//                buf.IsSelectedMesh = 1;
//            }
//            else
//            {
//                buf.IsSelectedMesh = 0;
//            }
//            UpdateConstantbufferMesh(buf);
//        }
//        // TODO: 기즈모 선택효과 안보임
//        // 현재 RenderPrimitive()에서 다시 SelectedMesh를 갱신하는데, 안에서 기즈모를 인식시킬 수 없음.
//
//        if (!GizmoComp->GetStaticMesh()) continue;
//
//        OBJ::FStaticMeshRenderData* renderData = GizmoComp->GetStaticMesh()->GetRenderData();
//        if (renderData == nullptr) continue;
//
//        RenderPrimitive(Model, renderData, GizmoComp->GetStaticMesh()->GetMaterials(), GizmoComp->GetOverrideMaterials());
//    }
//
//    Graphics->DeviceContext->RSSetState(Graphics->GetCurrentRasterizer());
//
//#pragma region GizmoDepth
//    ID3D11DepthStencilState* originalDepthState = Graphics->DepthStencilState;
//    Graphics->DeviceContext->OMSetDepthStencilState(originalDepthState, 0);
//#pragma endregion GizmoDepth
//}

void FRenderer::RenderBillboards(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    PrepareTextureShader();
    PrepareSubUVConstant();
    for (auto BillboardComp : RenderResources.Components.BillboardObjs)
    {
        UpdateSubUVConstant(BillboardComp->finalIndexU, BillboardComp->finalIndexV);
        
        {
            FMatrix Model = BillboardComp->CreateBillboardMatrix();
            FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
            FVector4 UUIDColor = BillboardComp->EncodeUUID() / 255.0f;
            FConstantBufferMesh buf;
            buf.ModelMatrix = Model;
            buf.ModelInvTransMatrix = NormalMatrix;
            if (BillboardComp == World->GetPickingGizmo())
            {
                buf.IsSelectedMesh = true;
            }
            else
            {
                buf.IsSelectedMesh = false;
            }
            UpdateConstantbufferMesh(buf);
        }

        if (UParticleSubUVComp* SubUVParticle = Cast<UParticleSubUVComp>(BillboardComp))
        {
            RenderTexturePrimitive(
                SubUVParticle->vertexSubUVBuffer, SubUVParticle->numTextVertices,
                SubUVParticle->indexTextureBuffer, SubUVParticle->numIndices,
                SubUVParticle->Texture->TextureSRV, SubUVParticle->Texture->SamplerState
            );
        }
        else if (UText* Text = Cast<UText>(BillboardComp))
        {
            UEditorEngine::RenderEngine.Renderer.RenderTextPrimitive(
                Text->vertexTextBuffer, Text->numTextVertices,
                Text->Texture->TextureSRV, Text->Texture->SamplerState
            );
        }
        else
        {
            RenderTexturePrimitive(
                BillboardComp->vertexTextureBuffer, BillboardComp->numVertices,
                BillboardComp->indexTextureBuffer, BillboardComp->numIndices,
                BillboardComp->Texture->TextureSRV, BillboardComp->Texture->SamplerState
            );
        }
    }
}

void FRenderer::RenderPostProcess()
{
    
}

void FRenderer::RenderLight(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    for (auto Light : RenderResources.Components.LightObjs)
    {
        FMatrix Model = JungleMath::CreateModelMatrix(Light->GetComponentLocation(), Light->GetComponentRotation(), {1, 1, 1});
        UPrimitiveBatch::GetInstance().AddCone(Light->GetComponentLocation(), Light->GetRadius(), 15, 140, Light->GetColor(), Model);
        UPrimitiveBatch::GetInstance().RenderOBB(Light->GetBoundingBox(), Light->GetComponentLocation(), Model);
    }
}
