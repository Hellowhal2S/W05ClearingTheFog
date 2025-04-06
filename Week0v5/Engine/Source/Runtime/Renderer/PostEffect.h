#pragma once
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")
#include <d3d11.h>
#include "Math/Matrix.h"

class FGraphicsDevice;

struct FPostEffectConstant
{
    // fog strength, fog color, inverse projection matrix... 등 추가 요망
};

namespace PostEffect
{
    struct FFogConstants
    {
        float heightStart = 0.0f;
        float heightFalloff = 50.0f;
        float fogDensity = 5.0f;
        int mode; // 1: Rendered image, 2: DepthOnly
        FVector4 fogColor  = { 1.0f,1.0f,1.0f,1.0f};
        float depthScale = 1.0f;
    };
    struct GlobalConstants
    {
        FMatrix invProj; // 4x4 행렬, 총 64바이트 (16의 배수)
    };
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
    extern ID3D11Buffer* FogConstantBuffer;
    extern ID3D11Buffer* GlobalConstantBuffer;

    extern ID3D11RenderTargetView* finalRTV;
    extern ID3D11Texture2D* finalTexture;
    
    extern FFogConstants Fog;
    
    void InitCommonStates(FGraphicsDevice*& Graphics);
    void InitBuffers(ID3D11Device*& Device);
    void InitShaders(ID3D11Device*& Device);
    void InitTextures(FGraphicsDevice*& Graphics);
    void InitDepthStencilStates(ID3D11Device*& Device);
    void InitRenderTargetViews(FGraphicsDevice * Graphics);
    void Render(ID3D11DeviceContext*& DeviceContext, ID3D11ShaderResourceView*& ColorSRV);
    void Release();
    void ReleaseFinalRTV();
    void UpdateFogConstantBuffer(ID3D11DeviceContext*& DeviceContext, FFogConstants newFog);
    void CopyBackBufferToColorSRV(ID3D11DeviceContext*& DeviceContext, ID3D11Texture2D*& ColorTexture, ID3D11Texture2D*& FrameBuffer);

    // Depth Stencil Buffer를 Depth Map Texture에 복사
    void CopyDepthBufferToDepthOnlySRV(ID3D11DeviceContext*& DeviceContext, ID3D11Texture2D*& SrcDepthTexture);

    
}
