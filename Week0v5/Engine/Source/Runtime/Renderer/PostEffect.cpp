#include "PostEffect.h"
#include <iostream>
#include <d3dcompiler.h>

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
            UE_LOG(LogLevel::Display, "UpdateBuffer(): buffer was not initialized");
        }

        D3D11_MAPPED_SUBRESOURCE ms;
        context->Map(buffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
        memcpy(ms.pData, &bufferData, sizeof(bufferData));
        context->Unmap(buffer.Get(), NULL);
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

        ThrowIfFailed(Device->CreateBuffer(&desc, &initData, &ConstantBuffer);
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
    ID3D11Buffer* PostEffectConstantBuffer;
    ID3D11Buffer* VertexBuffer;

} // namespace PostEffect

void PostEffect::InitCommonStates(ID3D11Device*& Device)
{
    InitBuffers(Device);
    InitShaders(Device);
    InitTextures(Device);
    //InitDepthStencilStates(Device);
    InitRenderTargetViews(Device);
}
void PostEffect::InitBuffers(ID3D11Device*& Device)
{
    // Vertex Buffer / Constant Buffer 초기화 및 생성 
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
}
void PostEffect::InitDepthStencilStates(ID3D11Device*& Device)
{
    
}

void PostEffect::InitTextures(ID3D11Device*& Device)
{
    // Depth 전용
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = 1920;
    desc.Height = 1080;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    Device->CreateDepthStencilView(DepthOnlyTexture, NULL, &DepthOnlyDSV);

    desc.Format = DXGI_FORMAT_R32_TYPELESS;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;                            // Depth Texture는 SRV로 사용이 가능해야 함
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    ThrowIfFailed(Device->CreateTexture2D(&desc, NULL, &DepthOnlyTexture));                            // Depth Pass를 수행할 Depth Texture 생성    

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    ZeroMemory(&dsvDesc, sizeof(dsvDesc));
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    ThrowIfFailed(Device->CreateDepthStencilView(DepthOnlyTexture, &dsvDesc, &DepthOnlyDSV));          // Depth Pass를 수행할 Depth Stencil View 생성

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    ThrowIfFailed(Device->CreateShaderResourceView(DepthOnlyTexture, &srvDesc, &DepthOnlySRV));        // Depth Texture를 셰이더로 보내기 위해 SRV로 제작
}

void PostEffect::InitRenderTargetViews(ID3D11Device*& Device)
{
}

void PostEffect::Release()
{
    SAFE_RELEASE(DepthOnlyRTV);
    SAFE_RELEASE(DepthOnlyTexture);
    SAFE_RELEASE(DepthOnlySRV);
    SAFE_RELEASE(DepthOnlyDSV);
    
    SAFE_RELEASE(WorldPosRTV);
    SAFE_RELEASE(WorldPosSRV);
    SAFE_RELEASE(WorldPosTexture);  

    SAFE_RELEASE(PostEffectSRV);

    SAFE_RELEASE(PostEffectInputLayout);
    SAFE_RELEASE(VertexBuffer);
    SAFE_RELEASE(PostEffectConstantBuffer);
    
    SAFE_RELEASE(PostEffectSampler);
    SAFE_RELEASE(PostEffectPS);
    SAFE_RELEASE(PostEffectVS);
}