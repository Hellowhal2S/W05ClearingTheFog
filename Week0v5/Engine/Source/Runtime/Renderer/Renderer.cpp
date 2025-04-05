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

void FRenderer::Initialize(FGraphicsDevice* graphics)
{
    Graphics = graphics;
    CreateShader();
    CreateTextureShader();
    CreateLineShader();
    CreateConstantBuffers();
}

void FRenderer::Release()
{
    ReleaseShader();
    ReleaseTextureShader();
    ReleaseLineShader();
    ReleaseConstantBuffers();
}

void FRenderer::CreateShader()
{
    ID3DBlob* VertexShaderCSO;
    ID3DBlob* PixelShaderCSO;

    D3DCompileFromFile(L"Shaders/StaticMeshVertexShader.hlsl", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "mainVS", "vs_5_0", 0, 0, &VertexShaderCSO, nullptr);
    Graphics->Device->CreateVertexShader(VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), nullptr, &VertexShader);

    D3DCompileFromFile(L"Shaders/StaticMeshPixelShader.hlsl", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "mainPS", "ps_5_0", 0, 0, &PixelShaderCSO, nullptr);
    Graphics->Device->CreatePixelShader(PixelShaderCSO->GetBufferPointer(), PixelShaderCSO->GetBufferSize(), nullptr, &PixelShader);

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"MATERIAL_INDEX", 0, DXGI_FORMAT_R32_UINT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    Graphics->Device->CreateInputLayout(
        layout, ARRAYSIZE(layout), VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), &InputLayout
    );

    Stride = sizeof(FVertexSimple);
    VertexShaderCSO->Release();
    PixelShaderCSO->Release();
}

void FRenderer::ReleaseShader()
{
    if (InputLayout)
    {
        InputLayout->Release();
        InputLayout = nullptr;
    }

    if (PixelShader)
    {
        PixelShader->Release();
        PixelShader = nullptr;
    }

    if (VertexShader)
    {
        VertexShader->Release();
        VertexShader = nullptr;
    }
}

void FRenderer::PrepareShader() const
{
    Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(InputLayout);

    // 추후 필요한 상태만 업데이트로 변경필요
    if (ConstantBuffers.Camera00)
    {
        Graphics->DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffers.Camera00);
        Graphics->DeviceContext->PSSetConstantBuffers(0, 1, &ConstantBuffers.Camera00);
    }
    if (ConstantBuffers.Light01)
    {
        Graphics->DeviceContext->VSSetConstantBuffers(1, 1, &ConstantBuffers.Light01);
        Graphics->DeviceContext->PSSetConstantBuffers(1, 1, &ConstantBuffers.Light01);
    }
    if (ConstantBuffers.Actor03)
    {
        Graphics->DeviceContext->VSSetConstantBuffers(3, 1, &ConstantBuffers.Actor03);
        Graphics->DeviceContext->PSSetConstantBuffers(3, 1, &ConstantBuffers.Actor03);
    }
    if (ConstantBuffers.Texture05)
    {
        Graphics->DeviceContext->PSSetConstantBuffers(5, 1, &ConstantBuffers.Texture05);
    }
    if (ConstantBuffers.Mesh06)
    {
        Graphics->DeviceContext->VSSetConstantBuffers(6, 1, &ConstantBuffers.Mesh06);
        Graphics->DeviceContext->PSSetConstantBuffers(6, 1, &ConstantBuffers.Mesh06);
    }
}

void FRenderer::ResetVertexShader() const
{
    Graphics->DeviceContext->VSSetShader(nullptr, nullptr, 0);
    VertexShader->Release();
}

void FRenderer::ResetPixelShader() const
{
    Graphics->DeviceContext->PSSetShader(nullptr, nullptr, 0);
    PixelShader->Release();
}

void FRenderer::SetVertexShader(const FWString& filename, const FString& funcname, const FString& version)
{
    // ���� �߻��� ���ɼ��� ����
    if (Graphics == nullptr)
        assert(0);
    if (VertexShader != nullptr)
        ResetVertexShader();
    if (InputLayout != nullptr)
        InputLayout->Release();
    ID3DBlob* vertexshaderCSO;

    D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, *funcname, *version, 0, 0, &vertexshaderCSO, nullptr);
    Graphics->Device->CreateVertexShader(vertexshaderCSO->GetBufferPointer(), vertexshaderCSO->GetBufferSize(), nullptr, &VertexShader);
    vertexshaderCSO->Release();
}

void FRenderer::SetPixelShader(const FWString& filename, const FString& funcname, const FString& version)
{
    // ���� �߻��� ���ɼ��� ����
    if (Graphics == nullptr)
        assert(0);
    if (VertexShader != nullptr)
        ResetVertexShader();
    ID3DBlob* pixelshaderCSO;
    D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, *funcname, *version, 0, 0, &pixelshaderCSO, nullptr);
    Graphics->Device->CreatePixelShader(pixelshaderCSO->GetBufferPointer(), pixelshaderCSO->GetBufferSize(), nullptr, &PixelShader);

    pixelshaderCSO->Release();
}

void FRenderer::ChangeViewMode(EViewModeIndex evi) const
{
    switch (evi)
    {
    case EViewModeIndex::VMI_Lit:
    {
        FConstantBufferLights buf;
        buf.isLit = 1;
        UpdateConstantbufferLights(buf);
    }
        break;
    case EViewModeIndex::VMI_Wireframe:
    case EViewModeIndex::VMI_Unlit:
    {
        FConstantBufferLights buf;
        buf.isLit = 0;
        UpdateConstantbufferLights(buf);
        break;
    }
    }
}

void FRenderer::RenderPrimitive(ID3D11Buffer* pBuffer, UINT numVertices) const
{
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pBuffer, &Stride, &offset);
    Graphics->DeviceContext->Draw(numVertices, 0);
}

void FRenderer::RenderPrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11Buffer* pIndexBuffer, UINT numIndices) const
{
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &Stride, &offset);
    Graphics->DeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    Graphics->DeviceContext->DrawIndexed(numIndices, 0, 0);
}

void FRenderer::RenderPrimitive(const FMatrix& ModelMatrix, OBJ::FStaticMeshRenderData* renderData, TArray<FStaticMaterial*> materials, TArray<UMaterial*> overrideMaterial, int selectedSubMeshIndex = -1) const
{
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &renderData->VertexBuffer, &Stride, &offset);

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

void FRenderer::RenderTexturedModelPrimitive(
    ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11Buffer* pIndexBuffer, UINT numIndices, ID3D11ShaderResourceView* InTextureSRV,
    ID3D11SamplerState* InSamplerState
) const
{
    if (!InTextureSRV || !InSamplerState)
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "SRV, Sampler Error");
    }
    if (numIndices <= 0)
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "numIndices Error");
    }
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &Stride, &offset);
    Graphics->DeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    //Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics->DeviceContext->PSSetShaderResources(0, 1, &InTextureSRV);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &InSamplerState);

    Graphics->DeviceContext->DrawIndexed(numIndices, 0, 0);
}

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
    Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &ConstantBuffers.Mesh06);

    ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    ConstantBufferDesc.ByteWidth = sizeof(FConstantBufferTexture);
    Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &ConstantBuffers.Texture05);

    ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    ConstantBufferDesc.ByteWidth = sizeof(FConstantBufferActor);
    Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &ConstantBuffers.Actor03);

    ConstantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    ConstantBufferDesc.ByteWidth = sizeof(FConstantBufferLights);
    Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &ConstantBuffers.Light01);
    
    ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    ConstantBufferDesc.ByteWidth = sizeof(FConstantBufferCamera);
    Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &ConstantBuffers.Camera00);

}

void FRenderer::ReleaseConstantBuffers()
{
    ReleaseBuffer(ConstantBuffers.Camera00);
    ReleaseBuffer(ConstantBuffers.Light01);
    ReleaseBuffer(ConstantBuffers.Actor03);
    ReleaseBuffer(ConstantBuffers.Texture05);
    ReleaseBuffer(ConstantBuffers.Mesh06);
}

void FRenderer::UpdateLightBuffer() const
{
    if (!ConstantBuffers.Light01) return;
    FConstantBufferLights buf;
    buf.DirLights[0].Color.Ambient = FVector(1.0f, 1.0f, 1.0f) * 0.06f;
    buf.DirLights[0].Color.Diffuse = FVector(1.0f, 1.0f, 1.0f);
    buf.DirLights[0].Color.Specular = FVector(1.0f, 1.0f, 1.0f);
    buf.DirLights[0].Direction = FVector(1.0f, 1.0f, 1.0f);

    UpdateConstantbufferLights(buf);
}

void FRenderer::UpdateConstantbufferMesh(FConstantBufferMesh Buffer) const
{
    if (ConstantBuffers.Mesh06)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����

        Graphics->DeviceContext->Map(ConstantBuffers.Mesh06, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        memcpy(ConstantBufferMSR.pData, &Buffer, sizeof(Buffer));
        Graphics->DeviceContext->Unmap(ConstantBuffers.Mesh06, 0); // GPU�� �ٽ� ��밡���ϰ� �����
    }
}

void FRenderer::UpdateConstantbufferTexture(FConstantBufferTexture Buffer) const
{
    if (ConstantBuffers.Texture05)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����

        Graphics->DeviceContext->Map(ConstantBuffers.Texture05, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        memcpy(ConstantBufferMSR.pData, &Buffer, sizeof(Buffer));
        Graphics->DeviceContext->Unmap(ConstantBuffers.Texture05, 0); // GPU�� �ٽ� ��밡���ϰ� �����
    }
}

void FRenderer::UpdateConstantbufferActor(FConstantBufferActor Buffer) const
{
    if (ConstantBuffers.Actor03)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����

        Graphics->DeviceContext->Map(ConstantBuffers.Actor03, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        memcpy(ConstantBufferMSR.pData, &Buffer, sizeof(Buffer));
        Graphics->DeviceContext->Unmap(ConstantBuffers.Actor03, 0); // GPU�� �ٽ� ��밡���ϰ� �����
    }
}

void FRenderer::UpdateConstantbufferLights(FConstantBufferLights Buffer) const
{
    if (ConstantBuffers.Light01)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����

        Graphics->DeviceContext->Map(ConstantBuffers.Light01, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        memcpy(ConstantBufferMSR.pData, &Buffer, sizeof(Buffer));
        Graphics->DeviceContext->Unmap(ConstantBuffers.Light01, 0); // GPU�� �ٽ� ��밡���ϰ� �����
    }
}

void FRenderer::UpdateConstantbufferCamera(FConstantBufferCamera Buffer) const
{
    if (ConstantBuffers.Camera00)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����

        Graphics->DeviceContext->Map(ConstantBuffers.Camera00, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        memcpy(ConstantBufferMSR.pData, &Buffer, sizeof(Buffer));
        Graphics->DeviceContext->Unmap(ConstantBuffers.Camera00, 0); // GPU�� �ٽ� ��밡���ϰ� �����
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
    hr = D3DCompileFromFile(L"Shaders/VertexTextureShader.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, &vertextextureshaderCSO, nullptr);
    if (FAILED(hr))
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "VertexShader Error");
    }
    Graphics->Device->CreateVertexShader(
        vertextextureshaderCSO->GetBufferPointer(), vertextextureshaderCSO->GetBufferSize(), nullptr, &VertexTextureShader
    );

    hr = D3DCompileFromFile(L"Shaders/PixelTextureShader.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, &pixeltextureshaderCSO, nullptr);
    if (FAILED(hr))
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "PixelShader Error");
    }
    Graphics->Device->CreatePixelShader(
        pixeltextureshaderCSO->GetBufferPointer(), pixeltextureshaderCSO->GetBufferSize(), nullptr, &PixelTextureShader
    );

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    Graphics->Device->CreateInputLayout(
        layout, ARRAYSIZE(layout), vertextextureshaderCSO->GetBufferPointer(), vertextextureshaderCSO->GetBufferSize(), &TextureInputLayout
    );

    //�ڷᱸ�� ���� �ʿ�
    TextureStride = sizeof(FVertexTexture);
    vertextextureshaderCSO->Release();
    pixeltextureshaderCSO->Release();
}

void FRenderer::ReleaseTextureShader()
{
    if (TextureInputLayout)
    {
        TextureInputLayout->Release();
        TextureInputLayout = nullptr;
    }

    if (PixelTextureShader)
    {
        PixelTextureShader->Release();
        PixelTextureShader = nullptr;
    }

    if (VertexTextureShader)
    {
        VertexTextureShader->Release();
        VertexTextureShader = nullptr;
    }
}

void FRenderer::PrepareTextureShader() const
{
    Graphics->DeviceContext->VSSetShader(VertexTextureShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelTextureShader, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(TextureInputLayout);

    //�ؽ��Ŀ� ConstantBuffer �߰��ʿ��Ҽ���
    if (ConstantBuffers.Texture05)
    {
        Graphics->DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffers.Texture05);
    }
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
    if (ConstantBuffers.Texture05)
    {
        FConstantBufferTexture buf;
        buf.UVOffset = { _indexU, _indexV };
        UpdateConstantbufferTexture(buf);
    }
}

void FRenderer::PrepareSubUVConstant() const
{
    if (ConstantBuffers.Texture05)
    {
        Graphics->DeviceContext->VSSetConstantBuffers(1, 1, &ConstantBuffers.Texture05);
        Graphics->DeviceContext->PSSetConstantBuffers(1, 1, &ConstantBuffers.Texture05);
    }
}

void FRenderer::PrepareLineShader() const
{
    // ���̴��� �Է� ���̾ƿ� ����
    Graphics->DeviceContext->VSSetShader(VertexLineShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelLineShader, nullptr, 0);

    // ��� ���� ���ε�: 
    // - MatrixBuffer�� register(b0)��, Vertex Shader�� ���ε�
    // - GridConstantBuffer�� register(b1)��, Vertex�� Pixel Shader�� ���ε� (�ȼ� ���̴��� �ʿ信 ����)
    if (ConstantBuffers.Mesh06 && GridConstantBuffer)
    {
        Graphics->DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffers.Mesh06);     // MatrixBuffer (b0)
        Graphics->DeviceContext->VSSetConstantBuffers(1, 1, &GridConstantBuffer); // GridParameters (b1)
        Graphics->DeviceContext->PSSetConstantBuffers(1, 1, &GridConstantBuffer);
        Graphics->DeviceContext->VSSetConstantBuffers(3, 1, &LinePrimitiveBuffer);
        Graphics->DeviceContext->VSSetShaderResources(2, 1, &pBBSRV);
        Graphics->DeviceContext->VSSetShaderResources(3, 1, &pConeSRV);
        Graphics->DeviceContext->VSSetShaderResources(4, 1, &pOBBSRV);
    }
}

void FRenderer::CreateLineShader()
{
    ID3DBlob* VertexShaderLine;
    ID3DBlob* PixelShaderLine;

    HRESULT hr;
    hr = D3DCompileFromFile(L"Shaders/ShaderLine.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", 0, 0, &VertexShaderLine, nullptr);
    if (FAILED(hr))
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "VertexShader Error");
    }
    Graphics->Device->CreateVertexShader(VertexShaderLine->GetBufferPointer(), VertexShaderLine->GetBufferSize(), nullptr, &VertexLineShader);

    hr = D3DCompileFromFile(L"Shaders/ShaderLine.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", 0, 0, &PixelShaderLine, nullptr);
    if (FAILED(hr))
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "PixelShader Error");
    }
    Graphics->Device->CreatePixelShader(PixelShaderLine->GetBufferPointer(), PixelShaderLine->GetBufferSize(), nullptr, &PixelLineShader);


    VertexShaderLine->Release();
    PixelShaderLine->Release();
}

void FRenderer::ReleaseLineShader() const
{
    if (GridConstantBuffer) GridConstantBuffer->Release();
    if (LinePrimitiveBuffer) LinePrimitiveBuffer->Release();
    if (VertexLineShader) VertexLineShader->Release();
    if (PixelLineShader) PixelLineShader->Release();
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
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN; // ����ü ������ ��� UNKNOWN
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementOffset = 0;
    srvDesc.Buffer.NumElements = numBoundingBoxes;


    Graphics->Device->CreateShaderResourceView(pBoundingBoxBuffer, &srvDesc, &pBBSRV);
    return pBBSRV;
}

ID3D11ShaderResourceView* FRenderer::CreateOBBSRV(ID3D11Buffer* pBoundingBoxBuffer, UINT numBoundingBoxes)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN; // ����ü ������ ��� UNKNOWN
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementOffset = 0;
    srvDesc.Buffer.NumElements = numBoundingBoxes;
    Graphics->Device->CreateShaderResourceView(pBoundingBoxBuffer, &srvDesc, &pOBBSRV);
    return pOBBSRV;
}

ID3D11ShaderResourceView* FRenderer::CreateConeSRV(ID3D11Buffer* pConeBuffer, UINT numCones)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN; // ����ü ������ ��� UNKNOWN
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementOffset = 0;
    srvDesc.Buffer.NumElements = numCones;


    Graphics->Device->CreateShaderResourceView(pConeBuffer, &srvDesc, &pConeSRV);
    return pConeSRV;
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
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = Graphics->DeviceContext->Map(GridConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (SUCCEEDED(hr))
    {
        memcpy(mappedResource.pData, &gridParams, sizeof(FGridParameters));
        Graphics->DeviceContext->Unmap(GridConstantBuffer, 0);
    }
    else
    {
        UE_LOG(LogLevel::Warning, "gridParams ���� ����");
    }
}

void FRenderer::UpdateLinePrimitveCountBuffer(int numBoundingBoxes, int numCones) const
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = Graphics->DeviceContext->Map(LinePrimitiveBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    auto pData = static_cast<FPrimitiveCounts*>(mappedResource.pData);
    pData->BoundingBoxCount = numBoundingBoxes;
    pData->ConeCount = numCones;
    Graphics->DeviceContext->Unmap(LinePrimitiveBuffer, 0);
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

void FRenderer::PrepareRender()
{
    if (GEngine->GetWorld()->WorldType == EWorldType::Editor)
    {
        for (const auto iter : TObjectRange<USceneComponent>())
        {
                UE_LOG(LogLevel::Display, "%d", GUObjectArray.GetObjectItemArrayUnsafe().Num());
                if (UStaticMeshComponent* pStaticMeshComp = Cast<UStaticMeshComponent>(iter))
                {
                    if (!Cast<UGizmoBaseComponent>(iter))
                        StaticMeshObjs.Add(pStaticMeshComp);
                }
                if (UGizmoBaseComponent* pGizmoComp = Cast<UGizmoBaseComponent>(iter))
                {
                    GizmoObjs.Add(pGizmoComp);
                }
                if (UBillboardComponent* pBillboardComp = Cast<UBillboardComponent>(iter))
                {
                    BillboardObjs.Add(pBillboardComp);
                }
                if (ULightComponentBase* pLightComp = Cast<ULightComponentBase>(iter))
                {
                    LightObjs.Add(pLightComp);
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
                if (UStaticMeshComponent* pStaticMeshComp = Cast<UStaticMeshComponent>(iter2))
                {
                    if (!Cast<UGizmoBaseComponent>(iter2))
                        StaticMeshObjs.Add(pStaticMeshComp);
                }
                if (UBillboardComponent* pBillboardComp = Cast<UBillboardComponent>(iter2))
                {
                    BillboardObjs.Add(pBillboardComp);
                }
                if (ULightComponentBase* pLightComp = Cast<ULightComponentBase>(iter2))
                {
                    LightObjs.Add(pLightComp);
                }
            }
        }
    }
}

void FRenderer::ClearRenderArr()
{
    StaticMeshObjs.Empty();
    GizmoObjs.Empty();
    BillboardObjs.Empty();
    LightObjs.Empty();
}

void FRenderer::Render(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    Graphics->DeviceContext->RSSetViewports(1, &ActiveViewport->GetD3DViewport());
    Graphics->ChangeRasterizer(ActiveViewport->GetViewMode());
    ChangeViewMode(ActiveViewport->GetViewMode());
    UpdateLightBuffer();
    // Scene Update
    {
        FConstantBufferCamera buf;
        buf.ViewMatrix = ActiveViewport->GetViewMatrix();
        buf.ProjMatrix = ActiveViewport->GetProjectionMatrix();
        UpdateConstantbufferCamera(buf);
    }
    //UPrimitiveBatch::GetInstance().RenderBatch(ActiveViewport->GetViewMatrix(), ActiveViewport->GetProjectionMatrix());

    if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_Primitives))
        RenderStaticMeshes(World, ActiveViewport);
    RenderGizmos(World, ActiveViewport);
    if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_BillboardText))
        RenderBillboards(World, ActiveViewport);
    RenderLight(World, ActiveViewport);
    
    ClearRenderArr();
}

void FRenderer::RenderStaticMeshes(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    PrepareShader();
    for (UStaticMeshComponent* StaticMeshComp : StaticMeshObjs)
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

        if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_AABB))
        {
            FMatrix Model = JungleMath::CreateModelMatrix(
                StaticMeshComp->GetWorldLocation(),
                StaticMeshComp->GetWorldRotation(),
                StaticMeshComp->GetWorldScale()
            );
            UPrimitiveBatch::GetInstance().RenderAABB(
                StaticMeshComp->GetBoundingBox(),
                StaticMeshComp->GetWorldLocation(),
                Model);
        }
        
        if (!StaticMeshComp->GetStaticMesh()) continue;

        OBJ::FStaticMeshRenderData* renderData = StaticMeshComp->GetStaticMesh()->GetRenderData();
        if (renderData == nullptr) continue;

        FMatrix Model = JungleMath::CreateModelMatrix(
            StaticMeshComp->GetWorldLocation(),
            StaticMeshComp->GetWorldRotation(),
            StaticMeshComp->GetWorldScale()
        );

        RenderPrimitive(Model, renderData, StaticMeshComp->GetStaticMesh()->GetMaterials(), StaticMeshComp->GetOverrideMaterials(), StaticMeshComp->GetselectedSubMeshIndex());
    }
}

void FRenderer::RenderGizmos(const UWorld* World, const std::shared_ptr<FEditorViewportClient>& ActiveViewport)
{
    if (!World->GetSelectedActor())
    {
        return;
    }

    #pragma region GizmoDepth
        ID3D11DepthStencilState* DepthStateDisable = Graphics->DepthStateDisable;
        Graphics->DeviceContext->OMSetDepthStencilState(DepthStateDisable, 0);
    #pragma endregion GizmoDepth

    //  fill solid,  Wirframe 에서도 제대로 렌더링되기 위함
    Graphics->DeviceContext->RSSetState(UEditorEngine::graphicDevice.RasterizerStateSOLID);
    
    for (auto GizmoComp : GizmoObjs)
    {
        
        if ((GizmoComp->GetGizmoType()==UGizmoBaseComponent::ArrowX ||
            GizmoComp->GetGizmoType()==UGizmoBaseComponent::ArrowY ||
            GizmoComp->GetGizmoType()==UGizmoBaseComponent::ArrowZ)
            && World->GetEditorPlayer()->GetControlMode() != CM_TRANSLATION)
            continue;
        else if ((GizmoComp->GetGizmoType()==UGizmoBaseComponent::ScaleX ||
            GizmoComp->GetGizmoType()==UGizmoBaseComponent::ScaleY ||
            GizmoComp->GetGizmoType()==UGizmoBaseComponent::ScaleZ)
            && World->GetEditorPlayer()->GetControlMode() != CM_SCALE)
            continue;
        else if ((GizmoComp->GetGizmoType()==UGizmoBaseComponent::CircleX ||
            GizmoComp->GetGizmoType()==UGizmoBaseComponent::CircleY ||
            GizmoComp->GetGizmoType()==UGizmoBaseComponent::CircleZ)
            && World->GetEditorPlayer()->GetControlMode() != CM_ROTATION)
            continue;
        FMatrix Model = JungleMath::CreateModelMatrix(GizmoComp->GetWorldLocation(),
            GizmoComp->GetWorldRotation(),
            GizmoComp->GetWorldScale()
        );

        {
            FConstantBufferActor buf;
            buf.IsSelectedActor = 0;
            buf.UUID = GizmoComp->EncodeUUID() / 255.0f;
            UpdateConstantbufferActor(buf);
        }
        // TODO: 기즈모 선택효과 안보임
        //if (GizmoComp == World->GetPickingGizmo())
        //    UpdateConstantbufferMesh(MVP, NormalMatrix, UUIDColor, true);
        //else
        //    UpdateConstantbufferMesh(MVP, NormalMatrix, UUIDColor, false);

        if (!GizmoComp->GetStaticMesh()) continue;

        OBJ::FStaticMeshRenderData* renderData = GizmoComp->GetStaticMesh()->GetRenderData();
        if (renderData == nullptr) continue;

        RenderPrimitive(Model, renderData, GizmoComp->GetStaticMesh()->GetMaterials(), GizmoComp->GetOverrideMaterials());
    }

    Graphics->DeviceContext->RSSetState(Graphics->GetCurrentRasterizer());

#pragma region GizmoDepth
    ID3D11DepthStencilState* originalDepthState = Graphics->DepthStencilState;
    Graphics->DeviceContext->OMSetDepthStencilState(originalDepthState, 0);
#pragma endregion GizmoDepth
}

void FRenderer::RenderBillboards(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    //PrepareTextureShader();
    //PrepareSubUVConstant();
    //for (auto BillboardComp : BillboardObjs)
    //{
    //    UpdateSubUVConstant(BillboardComp->finalIndexU, BillboardComp->finalIndexV);

    //    FMatrix Model = BillboardComp->CreateBillboardMatrix();

    //    // 최종 MVP 행렬
    //    FMatrix MVP = Model * ActiveViewport->GetViewMatrix() * ActiveViewport->GetProjectionMatrix();
    //    FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
    //    FVector4 UUIDColor = BillboardComp->EncodeUUID() / 255.0f;
    //    if (BillboardComp == World->GetPickingGizmo())
    //        UpdateConstantbufferMesh(MVP, NormalMatrix, UUIDColor, true);
    //    else
    //        UpdateConstantbufferMesh(MVP, NormalMatrix, UUIDColor, false);

    //    if (UParticleSubUVComp* SubUVParticle = Cast<UParticleSubUVComp>(BillboardComp))
    //    {
    //        RenderTexturePrimitive(
    //            SubUVParticle->vertexSubUVBuffer, SubUVParticle->numTextVertices,
    //            SubUVParticle->indexTextureBuffer, SubUVParticle->numIndices, SubUVParticle->Texture->TextureSRV, SubUVParticle->Texture->SamplerState
    //        );
    //    }
    //    else if (UText* Text = Cast<UText>(BillboardComp))
    //    {
    //        UEditorEngine::renderer.RenderTextPrimitive(
    //            Text->vertexTextBuffer, Text->numTextVertices,
    //            Text->Texture->TextureSRV, Text->Texture->SamplerState
    //        );
    //    }
    //    else
    //    {
    //        RenderTexturePrimitive(
    //            BillboardComp->vertexTextureBuffer, BillboardComp->numVertices,
    //            BillboardComp->indexTextureBuffer, BillboardComp->numIndices, BillboardComp->Texture->TextureSRV, BillboardComp->Texture->SamplerState
    //        );
    //    }
    //}
    //PrepareShader();
}

void FRenderer::RenderLight(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    for (auto Light : LightObjs)
    {
        FMatrix Model = JungleMath::CreateModelMatrix(Light->GetWorldLocation(), Light->GetWorldRotation(), {1, 1, 1});
        UPrimitiveBatch::GetInstance().AddCone(Light->GetWorldLocation(), Light->GetRadius(), 15, 140, Light->GetColor(), Model);
        UPrimitiveBatch::GetInstance().RenderOBB(Light->GetBoundingBox(), Light->GetWorldLocation(), Model);
    }
}
