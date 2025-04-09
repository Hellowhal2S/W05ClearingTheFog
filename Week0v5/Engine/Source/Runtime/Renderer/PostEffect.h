#pragma once
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")
#include <d3d11.h>

#include "HAL/PlatformType.h"
#include "Math/Matrix.h"

class FGraphicsDevice;
class AExponentialHeightFog;
namespace PostEffect
{
    struct ViewportConstants
    {
        float screenWidth;
        float screenHeight;
        float topLeftX;
        float topLeftY;
        float width;
        float height;
        FVector2D viewportPadding;
    };
    struct FFogConstants
    {
        float depthStart = 0.0f;
        float depthFalloff = 50.0f;
        float heightStart = 0.0f;
        float heightFalloff = 50.0f;
        float fogDensity = 0.1f;
        float heightDensity =0.3f;
        FVector2D fogPadding;
        FVector4 fogColor  = { 1.0f,1.0f,1.0f,1.0f};
    };
    struct FCameraConstants
    {
        FMatrix invProj; // 4x4 행렬, 총 64바이트 (16의 배수)
        FMatrix invView;
        FVector eyeWorld;
        float camNear = 10.0f;
        float camFar = 50.0f;
        FVector padding;
    };
    struct FLightConstants
    {
        FVector radiance; // Strength
        float fallOffStart;
        FVector direction;
        float fallOffEnd;
        FVector position;
        float spotPower;
        uint32 type;
        FVector dummy;
    };
    struct FPostEffectSettingConstants
    {
        int renderMode =0;// 0: Rendered image, 1: Normal, 2: DepthOnly, 3: WorldPos
        bool fogEnabled =true;
        FVector2D padding;
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

    // World Normal을 담을 Texture
    extern ID3D11RenderTargetView* WorldNormalRTV;
    extern ID3D11Texture2D* WorldNormalTexture;
    extern ID3D11ShaderResourceView* WorldNormalSRV;

    // Light 관련 Texture / RTV / SRV
    extern ID3D11RenderTargetView* AlbedoRTV;
    extern ID3D11Texture2D* AlbedoTexture;
    extern ID3D11ShaderResourceView* AlbedoSRV;

    extern ID3D11RenderTargetView* SpecularRTV;
    extern ID3D11Texture2D* SpecularTexture;
    extern ID3D11ShaderResourceView* SpecularSRV;


    // 원본 컬러 값 (Texture)를 변환시켜 SRV로 사용
    extern ID3D11ShaderResourceView* PostEffectSRV;

    // 샘플러 / Vertex Shader / Pixel Shader / Input Layout / Vertex Buffer / Constant Buffer
    extern ID3D11SamplerState* PostEffectSampler;
    extern ID3D11VertexShader* PostEffectVS;
    extern ID3D11PixelShader* PostEffectPS;
    extern ID3D11InputLayout* PostEffectInputLayout;
    extern ID3D11Buffer* ViewportConstantBuffer;
    extern ID3D11Buffer* FogConstantBuffer;
    extern ID3D11Buffer* CameraConstantBuffer;
    extern ID3D11Buffer* SettingConstantBuffer;
    extern ID3D11Buffer* LightConstantBuffer;
    
    extern ID3D11RenderTargetView* finalRTV;
    extern ID3D11Texture2D* finalTexture;
    
    extern int renderMode;
    
    void InitCommonStates(FGraphicsDevice*& Graphics);
    void InitBuffers(ID3D11Device*& Device);
    void InitShaders(ID3D11Device*& Device);
    void InitTextures(FGraphicsDevice*& Graphics);
    void InitDepthStencilStates(ID3D11Device*& Device);
    void InitDepthTextures(FGraphicsDevice * Graphics);
    void InitRenderTargetViews(FGraphicsDevice * Graphics);
    void Render(ID3D11DeviceContext*& DeviceContext, ID3D11ShaderResourceView*& ColorSRV);
    void ClearRTV(ID3D11DeviceContext*& DeviceContext);
    void Release();
    void ReleaseRTVDepth();
    void UpdateViewportConstantBuffer(ID3D11DeviceContext*& DeviceContext);
    void UpdateFogConstantBuffer(ID3D11DeviceContext*& DeviceContext, AExponentialHeightFog* newFog);
    void UpdateCameraConstantBuffer(ID3D11DeviceContext*& DeviceContext);
    void UpdateSettingConstantBuffer(ID3D11DeviceContext*& DeviceContext);



    void CopyBackBufferToColorSRV(ID3D11DeviceContext*& DeviceContext, ID3D11Texture2D*& ColorTexture, ID3D11Texture2D*& FrameBuffer);
    // Depth Stencil Buffer를 Depth Map Texture에 복사s
    void CopyDepthBufferToDepthOnlySRV(ID3D11DeviceContext*& DeviceContext, ID3D11Texture2D*& SrcDepthTexture);

    static bool CreateRenderTargetResources(ID3D11Device* Device, UINT Width, UINT Height, DXGI_FORMAT Format, ID3D11Texture2D** OutTexture, ID3D11RenderTargetView** OutRTV, ID3D11ShaderResourceView** OutSRV);

    
}
