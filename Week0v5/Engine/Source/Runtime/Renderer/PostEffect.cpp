#define _TCHAR_DEFINED  // TCHAR 재정의 에러 때문

#include "PostEffect.h"
#include <iostream>
#include <d3dcompiler.h>

#include "EditorEngine.h"
#include "Actors/AExponentialHeightFog.h"
#include "Components/HeightFogComponent.h"
#include "D3D11RHI/GraphicDevice.h"
#include "Engine/World.h"
#include "LevelEditor/SLevelEditor.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UObject/Casts.h"

#define SAFE_RELEASE(x) if (x) { x->Release(); x = nullptr; }

extern UEditorEngine* GEngine;

namespace PostEffect
{
    inline void ThrowIfFailed(HRESULT hr) 
    {
        if (FAILED(hr)) 
        {
            throw std::exception();
        }
    }
    template <typename T_DATA>
    static void UpdateBuffer(ID3D11Device*& Device, ID3D11DeviceContext*& DeviceContext, const T_DATA& bufferData, ID3D11Buffer*& Buffer)
    {
        if (!Buffer) {
            //UE_LOG(LogLevel::Display, "UpdateBuffer(): buffer was not initialized");
        }

        D3D11_MAPPED_SUBRESOURCE ms;
        DeviceContext->Map(Buffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
        memcpy(ms.pData, &bufferData, sizeof(bufferData));
        DeviceContext->Unmap(Buffer, NULL);
    }

    template <typename T_CONSTANT>
    static void CreateConstBuffer(ID3D11Device*& Device, const T_CONSTANT& ConstantBufferData, ID3D11Buffer*& ConstantBuffer) {

        static_assert((sizeof(T_CONSTANT) % 16) == 0, "Constant Buffer size must be 16-byte aligned");

        D3D11_BUFFER_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.ByteWidth = sizeof(ConstantBufferData);
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;
        desc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA initData;
        ZeroMemory(&initData, sizeof(initData));
        initData.pSysMem = &ConstantBufferData;
        initData.SysMemPitch = 0;
        initData.SysMemSlicePitch = 0;

        ThrowIfFailed(Device->CreateBuffer(&desc, &initData, &ConstantBuffer));
    }

    ID3D11RenderTargetView* DepthOnlyRTV;
    ID3D11Texture2D* DepthOnlyTexture;
    ID3D11ShaderResourceView* DepthOnlySRV;
    ID3D11DepthStencilView* DepthOnlyDSV;
    ID3D11RenderTargetView* WorldPosRTV;
    ID3D11Texture2D* WorldPosTexture;
    ID3D11ShaderResourceView* WorldPosSRV;
    ID3D11RenderTargetView* WorldNormalRTV;
    ID3D11Texture2D* WorldNormalTexture;
    ID3D11ShaderResourceView* WorldNormalSRV;
    ID3D11RenderTargetView* AlbedoRTV;
    ID3D11Texture2D* AlbedoTexture;
    ID3D11ShaderResourceView* AlbedoSRV;
    ID3D11RenderTargetView* SpecularRTV;
    ID3D11Texture2D* SpecularTexture;
    ID3D11ShaderResourceView* SpecularSRV;
    
    ID3D11InputLayout* PostEffectInputLayout;
    ID3D11ShaderResourceView* PostEffectSRV;
    ID3D11SamplerState* PostEffectSampler;
    ID3D11VertexShader* PostEffectVS;
    ID3D11PixelShader* PostEffectPS;
    ID3D11Buffer* ViewportConstantBuffer;
    ID3D11Buffer* FogConstantBuffer = nullptr;
    ID3D11Buffer* CameraConstantBuffer = nullptr;
    ID3D11Buffer* SettingConstantBuffer;
    ID3D11Buffer* LightConstantBuffer;
    
    ID3D11RenderTargetView* finalRTV;
    ID3D11Texture2D* finalTexture;
    int renderMode = 0;
} // namespace PostEffect

void PostEffect::InitCommonStates(FGraphicsDevice*& Graphics)
{
    InitBuffers(Graphics->Device);
    InitShaders(Graphics->Device);                
    //InitTextures(Graphics);
    InitDepthTextures(Graphics);
    //InitDepthStencilStates(Device);
    InitRenderTargetViews(Graphics);
}
void PostEffect::InitBuffers(ID3D11Device*& Device)
{
    // Vertex Buffer / Constant Buffer 초기화 및 생성
    // Vertex Buffer는 Vertex Shader에서 초기화

    // PostEffectsConstants에 대한 constant buffer 생성
    D3D11_BUFFER_DESC cbviewportDesc;
    ZeroMemory(&cbviewportDesc, sizeof(cbviewportDesc));
    cbviewportDesc.ByteWidth = sizeof(ViewportConstants);
    cbviewportDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbviewportDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbviewportDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbviewportDesc.MiscFlags = 0;
    
    HRESULT hr = Device->CreateBuffer(&cbviewportDesc, nullptr, &ViewportConstantBuffer);
    if (FAILED(hr))
    {
        OutputDebugString(L"Failed to create ViewportConstantBuffer\n");
    }
    
    
    D3D11_BUFFER_DESC cbfogDesc;
    ZeroMemory(&cbfogDesc, sizeof(cbfogDesc));
    cbfogDesc.ByteWidth = sizeof(FFogConstants);
    cbfogDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbfogDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbfogDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbfogDesc.MiscFlags = 0;
    
    hr = Device->CreateBuffer(&cbfogDesc, nullptr, &FogConstantBuffer);
    if (FAILED(hr))
    {
        OutputDebugString(L"Failed to create PostEffectsConstantBuffer\n");
    }
    
    D3D11_BUFFER_DESC cbCameraDesc;
    ZeroMemory(&cbCameraDesc, sizeof(cbCameraDesc));
    cbCameraDesc.ByteWidth = sizeof(FCameraConstants); // 64바이트
    cbCameraDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbCameraDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbCameraDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbCameraDesc.MiscFlags = 0;
    
    hr = Device->CreateBuffer(&cbCameraDesc, nullptr, &CameraConstantBuffer);
    if (FAILED(hr))
    {
        OutputDebugString(L"Failed to create CameraConstantBuffer\n");
    }
    
    D3D11_BUFFER_DESC cbSettingDesc;
    ZeroMemory(&cbSettingDesc, sizeof(cbSettingDesc));
    cbSettingDesc.ByteWidth = sizeof(FPostEffectSettingConstants);
    cbSettingDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbSettingDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbSettingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbSettingDesc.MiscFlags = 0;
    
    hr = Device->CreateBuffer(&cbSettingDesc, nullptr, &SettingConstantBuffer);
    if (FAILED(hr))
    {
        OutputDebugString(L"Failed to create PostSettingConstantBuffer\n");
    }
    
}
void PostEffect::InitShaders(ID3D11Device*& Device)
{
    // Quad 사각형 버텍스를 그리기 위한 Vertex Shader                                  : SamplingVS.hlsl
    // 원본 색상, Depth, World Position을 SRV로 받아 오물딱 조물딱 잘 섞을 Pixel Shader : PostEffect.hlsl


    // DEBUG 모드에서 Shader Compile 시 그래픽스 디버거에서 셰이더 디버깅 가능
#if _DEBUG
    DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
    shaderFlags |= D3DCOMPILE_DEBUG;
    shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    DWORD shaderFlags = 0;
#endif

    ID3DBlob* blob = nullptr;
    D3DCompileFromFile(L"Shaders/SamplingVS.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", shaderFlags, 0, &blob, nullptr);
    Device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &PostEffectVS);
    blob->Release();
    D3DCompileFromFile(L"Shaders/PostEffectPS.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", shaderFlags, 0, &blob, nullptr);
    Device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &PostEffectPS);
    blob->Release();

    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    Device->CreateSamplerState(&samplerDesc, &PostEffectSampler);
}
void PostEffect::InitTextures(FGraphicsDevice*& Graphics)
{
}
void PostEffect::InitDepthStencilStates(ID3D11Device*& Device)
{
    
}

void PostEffect::InitDepthTextures(FGraphicsDevice* Graphics)
{
    // Depth 전용
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = Graphics->screenWidth;
    desc.Height = Graphics->screenHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    desc.Format = DXGI_FORMAT_R32_TYPELESS;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;                            // Depth Texture는 SRV로 사용이 가능해야 함
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    ThrowIfFailed(Graphics->Device->CreateTexture2D(&desc, NULL, &DepthOnlyTexture));                            // Depth Pass를 수행할 Depth Texture 생성    

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    ZeroMemory(&dsvDesc, sizeof(dsvDesc));
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    ThrowIfFailed(Graphics->Device->CreateDepthStencilView(DepthOnlyTexture, &dsvDesc, &DepthOnlyDSV));          // Depth Pass를 수행할 Depth Stencil View 생성

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    ThrowIfFailed(Graphics->Device->CreateShaderResourceView(DepthOnlyTexture, &srvDesc, &DepthOnlySRV));        // Depth Texture를 셰이더로 보내기 위해 SRV로 제작
}

void PostEffect::InitRenderTargetViews(FGraphicsDevice* Graphics)
{
    Graphics->SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&finalTexture);

    D3D11_RENDER_TARGET_VIEW_DESC finalRTVDesc = {};
    finalRTVDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    finalRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

    HRESULT hr = Graphics->Device->CreateRenderTargetView(finalTexture, &finalRTVDesc, &finalRTV);
    if (FAILED(hr))
    {
        OutputDebugString(L"Failed to create finalRTV in InitRenderTargetViews\n");
        finalTexture->Release();
        finalTexture = nullptr;
        return;
    }

    // World Position Render Target 생성
    if (!CreateRenderTargetResources(Graphics->Device, Graphics->screenWidth, Graphics->screenHeight,
        DXGI_FORMAT_R32G32B32A32_FLOAT, &WorldPosTexture, &WorldPosRTV, &WorldPosSRV))
    {
        OutputDebugString(L"Failed to create World Position render target resources\n");
        return;
    }

    // World Normal Render Target 생성
    if (!CreateRenderTargetResources(Graphics->Device, Graphics->screenWidth, Graphics->screenHeight,
        DXGI_FORMAT_R32G32B32A32_FLOAT, &WorldNormalTexture, &WorldNormalRTV, &WorldNormalSRV))
    {
        OutputDebugString(L"Failed to create World Normal render target resources\n");
        return;
    }

    // Albedo Render Target 생성
    if (!CreateRenderTargetResources(Graphics->Device, Graphics->screenWidth, Graphics->screenHeight,
        DXGI_FORMAT_R32G32B32A32_FLOAT, &AlbedoTexture, &AlbedoRTV, &AlbedoSRV))
    {
        OutputDebugString(L"Failed to create Albedo render target resources\n");
        return;
    }

    // Specular Render Target 생성
    if (!CreateRenderTargetResources(Graphics->Device, Graphics->screenWidth, Graphics->screenHeight,
        DXGI_FORMAT_R32G32B32A32_FLOAT, &SpecularTexture, &SpecularRTV, &SpecularSRV))
    {
        OutputDebugString(L"Failed to create Specular render target resources\n");
        return;
    }
}

void PostEffect::Render(ID3D11DeviceContext*& DeviceContext, ID3D11ShaderResourceView*& ColorSRV)
{
    // VS
    // PS
    // SRV
    // Draw
    // Sampler
    //ClearCommBreak
    GEngine->graphicDevice.ChangeRasterizer(EViewModeIndex::VMI_Lit);
    DeviceContext->OMSetRenderTargets(1, &finalRTV, nullptr);
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    DeviceContext->VSSetShader(PostEffectVS, nullptr, 0);
    DeviceContext->PSSetShader(PostEffectPS, nullptr, 0);
    DeviceContext->IASetInputLayout(PostEffectInputLayout);

    ID3D11ShaderResourceView* ppSRV[6] = { 
        ColorSRV, DepthOnlySRV, WorldPosSRV, WorldNormalSRV, AlbedoSRV, SpecularSRV
    };
    DeviceContext->PSSetShaderResources(10, 6, ppSRV);                      // SRV
    DeviceContext->VSSetConstantBuffers(4, 1, &ViewportConstantBuffer);       // 상수 버퍼

    DeviceContext->PSSetConstantBuffers(0, 1, &CameraConstantBuffer);       // 상수 버퍼
    DeviceContext->PSSetConstantBuffers(1, 1, &FogConstantBuffer);
    DeviceContext->PSSetConstantBuffers(2, 1, &SettingConstantBuffer);

    UpdateViewportConstantBuffer(DeviceContext);
    UpdateFogConstantBuffer(DeviceContext, GEngine->GetWorld()->Fog);
    UpdateCameraConstantBuffer(DeviceContext);
    UpdateSettingConstantBuffer(DeviceContext);
    
    DeviceContext->PSSetSamplers(0, 1, &PostEffectSampler);                 // Sampler      
    DeviceContext->Draw(6, 0);
}

void PostEffect::ClearRTV(ID3D11DeviceContext*& DeviceContext)
{
    FLOAT ClearColor[4] = { 1.f, 1.f, 1.f, 1.0f };
    DeviceContext->ClearRenderTargetView(finalRTV, ClearColor); // Clear Color
}

void PostEffect::Release()
{
    ReleaseRTVDepth();

    SAFE_RELEASE(PostEffectSRV);                    // 원본 Color SRV

    SAFE_RELEASE(PostEffectInputLayout);            // Input Layout
    SAFE_RELEASE(FogConstantBuffer);                // Vertex Buffer
    SAFE_RELEASE(CameraConstantBuffer);             // 역투영 등의 Constant Buffer
    SAFE_RELEASE(SettingConstantBuffer);
    SAFE_RELEASE(LightConstantBuffer);
    
    SAFE_RELEASE(PostEffectSampler);                // Sampler
    SAFE_RELEASE(PostEffectPS);                     // Pixel Shader
    SAFE_RELEASE(PostEffectVS);                     // Vertex Shader     
}

void PostEffect::ReleaseRTVDepth()
{
    SAFE_RELEASE(finalRTV);                         // Final Render Target View
    SAFE_RELEASE(finalTexture);

    SAFE_RELEASE(WorldPosRTV);                      // World Position Texture RTV  
    SAFE_RELEASE(WorldPosTexture);
    SAFE_RELEASE(WorldPosSRV);

    SAFE_RELEASE(WorldNormalRTV);                   // World Normal Texture RTV
    SAFE_RELEASE(WorldNormalTexture);
    SAFE_RELEASE(WorldNormalSRV);

    SAFE_RELEASE(AlbedoRTV);                        // Albedo Texture RTV
    SAFE_RELEASE(AlbedoTexture);    
    SAFE_RELEASE(AlbedoSRV);

    SAFE_RELEASE(SpecularRTV);                      // Specular Texture RTV  
    SAFE_RELEASE(SpecularTexture);
    SAFE_RELEASE(SpecularSRV);

    SAFE_RELEASE(DepthOnlyRTV);                     // Depth Texture RTV    
    SAFE_RELEASE(DepthOnlyTexture);                  // Depth Texture
    SAFE_RELEASE(DepthOnlySRV);                     // Depth Only Texture
    SAFE_RELEASE(DepthOnlyDSV);                     // Depth Only Stencil View
}

void PostEffect::UpdateViewportConstantBuffer(ID3D11DeviceContext*& DeviceContext)
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    DeviceContext->Map(ViewportConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    {
        ViewportConstants* constants = static_cast<ViewportConstants*>(mappedResource.pData);
        constants->screenWidth = GEngine->graphicDevice.screenWidth;
        constants->screenHeight = GEngine->graphicDevice.screenHeight;
        constants->topLeftX = GEngine->GetLevelEditor()->GetActiveViewportClient()->GetD3DViewport().TopLeftX;
        constants->topLeftY = GEngine->GetLevelEditor()->GetActiveViewportClient()->GetD3DViewport().TopLeftY;
        constants->width = GEngine->GetLevelEditor()->GetActiveViewportClient()->GetD3DViewport().Width;
        constants->height = GEngine->GetLevelEditor()->GetActiveViewportClient()->GetD3DViewport().Height;
    }
    DeviceContext->Unmap(ViewportConstantBuffer, 0);
}

void PostEffect::UpdateFogConstantBuffer(ID3D11DeviceContext*& DeviceContext, AExponentialHeightFog* newFog)
{
    if (!FogConstantBuffer) return;
    if (!newFog) return;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    DeviceContext->Map(FogConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    {
        FFogConstants* constants = static_cast<FFogConstants*>(mappedResource.pData);
        constants->depthStart = newFog->GetFogComponent()->GetDepthStart();
        constants->depthFalloff =  newFog->GetFogComponent()->GetDepthFalloff();
        constants->heightStart =  newFog->GetFogComponent()->GetHeightStart();
        constants->heightFalloff =  newFog->GetFogComponent()->GetHeightFalloff();
        constants->heightDensity =  newFog->GetFogComponent()->GetHeightDensity();
        constants->fogDensity =  newFog->GetFogComponent()->GetFogDensity();
        constants->fogColor =  newFog->GetFogComponent()->GetFogColor();
    }
    DeviceContext->Unmap(FogConstantBuffer, 0);
}

void PostEffect::UpdateCameraConstantBuffer(ID3D11DeviceContext*& DeviceContext)
{
    if (!CameraConstantBuffer) return;
    std::shared_ptr<FEditorViewportClient> EditorViewport = GEngine->GetLevelEditor()->GetActiveViewportClient();
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    DeviceContext->Map(CameraConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    {
        FCameraConstants* constants = static_cast<FCameraConstants*>(mappedResource.pData);
        constants->invProj = FMatrix::Transpose(FMatrix::Inverse(EditorViewport->GetProjectionMatrix()));
        constants->invView = FMatrix::Transpose(FMatrix::Inverse(EditorViewport->GetViewMatrix()));
        constants->eyeWorld = EditorViewport->ViewTransformPerspective.GetLocation();
        constants->camNear = EditorViewport->nearPlane;
        constants->camFar = EditorViewport->farPlane;
    }
    DeviceContext->Unmap(CameraConstantBuffer, 0);
}
void PostEffect::UpdateSettingConstantBuffer(ID3D11DeviceContext*& DeviceContext)
{
    if (!SettingConstantBuffer) return;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    DeviceContext->Map(SettingConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    {
        FPostEffectSettingConstants* constants = static_cast<FPostEffectSettingConstants*>(mappedResource.pData);
        constants->renderMode = static_cast<int>(GEngine->GetLevelEditor()->GetActiveViewportClient()->GetViewMode());
        constants->fogEnabled = static_cast<bool>(GEngine->GetWorld()->Fog);
    }
    DeviceContext->Unmap(SettingConstantBuffer, 0);
}

void PostEffect::CopyBackBufferToColorSRV(ID3D11DeviceContext*& DeviceContext, ID3D11Texture2D*& ColorTexture, ID3D11Texture2D*& FrameBuffer)
{
    // 백버퍼가 멀티샘플이 아니므로 CopyResource를 사용하여 SRV로 쓰일 텍스처에 복사 
    DeviceContext->CopyResource(ColorTexture, FrameBuffer);
    //DeviceContext->ResolveSubresource(ColorTexture, 0, FrameBuffer, 0, DXGI_FORMAT_R8G8B8A8_UNORM);
}

void PostEffect::CopyDepthBufferToDepthOnlySRV(ID3D11DeviceContext*& DeviceContext, ID3D11Texture2D*& SrcDepthTexture)
{
    DeviceContext->CopyResource(DepthOnlyTexture, SrcDepthTexture);
}

static bool PostEffect::CreateRenderTargetResources(
    ID3D11Device* Device,
    UINT Width,
    UINT Height,
    DXGI_FORMAT Format,
    ID3D11Texture2D** OutTexture,
    ID3D11RenderTargetView** OutRTV,
    ID3D11ShaderResourceView** OutSRV)
{
    D3D11_TEXTURE2D_DESC TexDesc = {};
    TexDesc.Width = Width;
    TexDesc.Height = Height;
    TexDesc.MipLevels = 1;
    TexDesc.ArraySize = 1;
    TexDesc.Format = Format;
    TexDesc.SampleDesc.Count = 1;
    TexDesc.Usage = D3D11_USAGE_DEFAULT;
    TexDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    HRESULT Hr = Device->CreateTexture2D(&TexDesc, nullptr, OutTexture);
    if (FAILED(Hr))
    {
        OutputDebugString(L"Failed to create texture in CreateRenderTargetResources\n");
        return false;
    }

    D3D11_RENDER_TARGET_VIEW_DESC RtvDesc = {};
    RtvDesc.Format = Format;
    RtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    Hr = Device->CreateRenderTargetView(*OutTexture, &RtvDesc, OutRTV);
    if (FAILED(Hr))
    {
        OutputDebugString(L"Failed to create RTV in CreateRenderTargetResources\n");
        (*OutTexture)->Release();
        *OutTexture = nullptr;
        return false;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
    SrvDesc.Format = Format;
    SrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    SrvDesc.Texture2D.MostDetailedMip = 0;
    SrvDesc.Texture2D.MipLevels = 1;
    Hr = Device->CreateShaderResourceView(*OutTexture, &SrvDesc, OutSRV);
    if (FAILED(Hr))
    {
        OutputDebugString(L"Failed to create SRV in CreateRenderTargetResources\n");
        (*OutRTV)->Release();
        (*OutTexture)->Release();
        *OutTexture = nullptr;
        *OutRTV = nullptr;
        return false;
    }

    return true;
}
