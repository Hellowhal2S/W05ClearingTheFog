#pragma once
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")
#include <d3d11.h>

struct FPostEffectConstant
{
    // fog strength, fog color, inverse projection matrix... 등 추가 요망
};

namespace PostEffect
{

    // Depth만 담는 Texture
    extern ID3D11RenderTargetView* DepthOnlyRTV;
    extern ID3D11Texture2D* DepthOnlyTexture;
    extern ID3D11ShaderResourceView* DepthOnlySRV;
    extern ID3D11DepthStencilView* DepthOnlyDSV;

    // World Position을 담을 Texture
    extern ID3D11RenderTargetView* WorldPosRTV;
    extern ID3D11Texture2D* WorldPosTexture;
    extern ID3D11ShaderResourceView* WorldPosSRV;

    // 원본 컬러 값 (Texture)를 변환시켜 SRV로 사용
    extern ID3D11ShaderResourceView* PostEffectSRV;

    // 샘플러 / Vertex Shader / Pixel Shader / Input Layout / Vertex Buffer / Constant Buffer
    extern ID3D11SamplerState* PostEffectSampler;
    extern ID3D11VertexShader* PostEffectVS;
    extern ID3D11PixelShader* PostEffectPS;
    extern ID3D11InputLayout* PostEffectInputLayout;
    extern ID3D11Buffer* PostEffectConstantBuffer;
    extern ID3D11Buffer* VertexBuffer;

    
    void InitCommonStates(ID3D11Device*& Device);
    void InitBuffers(ID3D11Device*& Device);
    void InitShaders(ID3D11Device*& Device);
    void InitTextures(ID3D11Device*& Device);
    void InitDepthStencilStates(ID3D11Device*& Device);
    void InitRenderTargetViews(ID3D11Device*& Device);
    void Release();
}
