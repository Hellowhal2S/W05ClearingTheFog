#include "PostEffect.h"
#include <iostream>
#include <d3dcompiler.h>
#include "D3D11RHI/GraphicDevice.h"

#define SAFE_RELEASE(x) if (x) { x->Release(); x = nullptr; }

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
    
    ID3D11InputLayout* PostEffectInputLayout;
    ID3D11ShaderResourceView* PostEffectSRV;
    ID3D11SamplerState* PostEffectSampler;
    ID3D11VertexShader* PostEffectVS;
    ID3D11PixelShader* PostEffectPS;
    ID3D11Buffer* FogConstantBuffer = nullptr;
    ID3D11Buffer* GlobalConstantBuffer = nullptr;


    ID3D11RenderTargetView* finalRTV;
    ID3D11Texture2D* finalTexture;
    FFogConstants Fog;
} // namespace PostEffect

void PostEffect::InitCommonStates(FGraphicsDevice*& Graphics)
{
    InitBuffers(Graphics->Device);
    InitShaders(Graphics->Device);                
    InitTextures(Graphics);
    //InitDepthStencilStates(Device);
    InitRenderTargetViews(Graphics);
}
void PostEffect::InitBuffers(ID3D11Device*& Device)
{
    // Vertex Buffer / Constant Buffer 초기화 및 생성
    // Vertex Buffer는 Vertex Shader에서 초기화

    // PostEffectsConstants에 대한 constant buffer 생성
    D3D11_BUFFER_DESC cbfogDesc;
    ZeroMemory(&cbfogDesc, sizeof(cbfogDesc));
    cbfogDesc.ByteWidth = sizeof(FFogConstants);
    cbfogDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbfogDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbfogDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbfogDesc.MiscFlags = 0;
    
    HRESULT hr = Device->CreateBuffer(&cbfogDesc, nullptr, &FogConstantBuffer);
    if (FAILED(hr))
    {
        OutputDebugString(L"Failed to create PostEffectsConstantBuffer\n");
    }
    
    D3D11_BUFFER_DESC cbGlobalDesc;
    ZeroMemory(&cbGlobalDesc, sizeof(cbGlobalDesc));
    cbGlobalDesc.ByteWidth = sizeof(GlobalConstants); // 64바이트
    cbGlobalDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbGlobalDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbGlobalDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbGlobalDesc.MiscFlags = 0;
    
    hr = Device->CreateBuffer(&cbGlobalDesc, nullptr, &GlobalConstantBuffer);
    if (FAILED(hr))
    {
        OutputDebugString(L"Failed to create GlobalConstantBuffer\n");
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
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    Device->CreateSamplerState(&samplerDesc, &PostEffectSampler);
}
void PostEffect::InitDepthStencilStates(ID3D11Device*& Device)
{
    
}

void PostEffect::InitTextures(FGraphicsDevice*& Graphics)
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
    // desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    // Device->CreateDepthStencilView(DepthOnlyTexture, NULL, &DepthOnlyDSV);

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
    
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    
    HRESULT hr = Graphics->Device->CreateRenderTargetView(finalTexture, &rtvDesc, &finalRTV);
    if (FAILED(hr))
    {
        OutputDebugString(L"Failed to create finalRTV in InitRenderTargetViews\n");
        // 실패시 생성한 텍스처 릴리즈 필요
        finalTexture->Release();
        finalTexture = nullptr;
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
    FLOAT ClearColor[4] = { 1.0f, 0.025f, 0.025f, 1.0f };
    //DeviceContext->ClearRenderTargetView(finalRTV, ClearColor); // Clear Color
    DeviceContext->OMSetRenderTargets(1, &finalRTV, nullptr);
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    DeviceContext->VSSetShader(PostEffectVS, nullptr, 0);
    DeviceContext->PSSetShader(PostEffectPS, nullptr, 0);
    DeviceContext->IASetInputLayout(PostEffectInputLayout);

    
    DeviceContext->PSSetConstantBuffers(0, 1, &GlobalConstantBuffer);       // 상수 버퍼
    DeviceContext->PSSetConstantBuffers(1, 1, &FogConstantBuffer);

    DeviceContext->PSSetShaderResources(10, 1, &ColorSRV);                   // SRV
    DeviceContext->PSSetShaderResources(11, 1, &DepthOnlySRV);  

    UpdateFogConstantBuffer(DeviceContext, Fog);
    DeviceContext->PSSetSamplers(0, 1, &PostEffectSampler);                 // Sampler      
    DeviceContext->Draw(6, 0);
}

void PostEffect::Release()
{
    SAFE_RELEASE(DepthOnlyRTV);                     // Depth Texture RTV    
    SAFE_RELEASE(DepthOnlyTexture);                 // Depth Texture
    SAFE_RELEASE(DepthOnlySRV);                     // Depth Only Texture
    SAFE_RELEASE(DepthOnlyDSV);                     // Depth Only Stencil View
    
    SAFE_RELEASE(WorldPosRTV);                      // World Position RTV     
    SAFE_RELEASE(WorldPosSRV);                      // World Position SRV
    SAFE_RELEASE(WorldPosTexture);                  // World Position Texture

    SAFE_RELEASE(PostEffectSRV);                    // 원본 Color SRV

    SAFE_RELEASE(PostEffectInputLayout);            // Input Layout
    SAFE_RELEASE(FogConstantBuffer);                // Vertex Buffer
    SAFE_RELEASE(GlobalConstantBuffer);             // 역투영 등의 Constant Buffer

    
    SAFE_RELEASE(PostEffectSampler);                // Sampler
    SAFE_RELEASE(PostEffectPS);                     // Pixel Shader
    SAFE_RELEASE(PostEffectVS);                     // Vertex Shader     

    ReleaseFinalRTV();
}

void PostEffect::ReleaseFinalRTV()
{
    SAFE_RELEASE(finalRTV);
    SAFE_RELEASE(finalTexture);
}

void PostEffect::UpdateFogConstantBuffer(ID3D11DeviceContext*& DeviceContext, FFogConstants newFog)
{
    if (!FogConstantBuffer) return;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    DeviceContext->Map(FogConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    {
        FFogConstants* constants = static_cast<FFogConstants*>(mappedResource.pData);
        constants->heightStart = newFog.heightStart;
        constants->heightFalloff = newFog.heightFalloff;
        constants->fogDensity = newFog.fogDensity;
        constants->mode = newFog.mode;
        constants->fogColor = newFog.fogColor;
        constants->depthScale = newFog.depthScale;
    }
    DeviceContext->Unmap(FogConstantBuffer, 0);
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

