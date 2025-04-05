#include "PostEffect.h"
#include <iostream>
#include <d3dcompiler.h>
#include "Math/Matrix.h"
#define SAFE_RELEASE(x) if (x) { x->Release(); x = nullptr; }

struct FogConstants
{
    float heightStart;
    float heightFalloff;
    float fogDensity;
    int mode; // 1: Rendered image, 2: DepthOnly
    float fogColor[4];
};
struct GlobalConstants
{
    FMatrix invProj; // 4x4 행렬, 총 64바이트 (16의 배수)
};


namespace PostEffect
{
    inline void ThrowIfFailed(HRESULT hr) 
    {
        if (FAILED(hr)) 
        {
            throw std::exception();
        }
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
    // Vertex Buffer는 Vertex Shader에서 초기화

    // PostEffectsConstants에 대한 constant buffer 생성
    D3D11_BUFFER_DESC cbfogDesc;
    ZeroMemory(&cbfogDesc, sizeof(cbfogDesc));
    cbfogDesc.ByteWidth = sizeof(FogConstants);
    cbfogDesc.Usage = D3D11_USAGE_DEFAULT;
    cbfogDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbfogDesc.CPUAccessFlags = 0;
    cbfogDesc.MiscFlags = 0;
    
    HRESULT hr = Device->CreateBuffer(&cbfogDesc, nullptr, &FogConstantBuffer);
    if (FAILED(hr))
    {
        OutputDebugString(L"Failed to create PostEffectsConstantBuffer\n");
    }
    
    D3D11_BUFFER_DESC cbGlobalDesc;
    ZeroMemory(&cbGlobalDesc, sizeof(cbGlobalDesc));
    cbGlobalDesc.ByteWidth = sizeof(GlobalConstants); // 64바이트
    cbGlobalDesc.Usage = D3D11_USAGE_DEFAULT;
    cbGlobalDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbGlobalDesc.CPUAccessFlags = 0;
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
    // desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    // Device->CreateDepthStencilView(DepthOnlyTexture, NULL, &DepthOnlyDSV);

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
    SAFE_RELEASE(FogConstantBuffer);
    SAFE_RELEASE(GlobalConstantBuffer);

    
    SAFE_RELEASE(PostEffectSampler);
    SAFE_RELEASE(PostEffectPS);
    SAFE_RELEASE(PostEffectVS);
}