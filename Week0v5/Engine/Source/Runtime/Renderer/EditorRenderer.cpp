#include "EditorRenderer.h"

#include <d3dcompiler.h>
#include "Engine/World.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UObject/UObjectIterator.h"
#include "BaseGizmos/GizmoBaseComponent.h"
#include "D3D11RHI/GraphicDevice.h"
#include "Engine/Classes/Actors/Player.h"
#include "Renderer.h"
#include "Engine/Classes/Components/FireBallComponent.h"
#include "Engine/Classes/Components/LightComponent.h"


void FEditorRenderer::Initialize(FRenderer* InRenderer)
{
    Renderer = InRenderer;
    CreateShaders();
    CreateBuffers();
    CreateConstantBuffers();
}

void FEditorRenderer::Release()
{
    ReleaseShaders();
}

void FEditorRenderer::CreateShaders()
{
    ID3DBlob* errorBlob = nullptr;
    ID3DBlob* VertexShaderCSO;
    ID3DBlob* PixelShaderCSO;

    /////////////////////////////
    // 기즈모
    D3DCompileFromFile(L"Shaders/EditorShader.hlsl", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "gizmoVS", "vs_5_0", 0, 0, &VertexShaderCSO, &errorBlob);
    if (errorBlob)
    {
        OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        errorBlob->Release();
    }
    Renderer->Graphics->Device->CreateVertexShader(VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), nullptr, &Resources.Shaders.Gizmo.Vertex);
    
    D3DCompileFromFile(L"Shaders/EditorShader.hlsl", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "gizmoPS", "ps_5_0", 0, 0, &PixelShaderCSO, &errorBlob);

    Renderer->Graphics->Device->CreatePixelShader(PixelShaderCSO->GetBufferPointer(), PixelShaderCSO->GetBufferSize(), nullptr, &Resources.Shaders.Gizmo.Pixel);

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"MATERIAL_INDEX", 0, DXGI_FORMAT_R32_UINT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    Renderer->Graphics->Device->CreateInputLayout(
        layout, ARRAYSIZE(layout), VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), &Resources.Shaders.Gizmo.Layout
    );
    Resources.Shaders.Gizmo.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    VertexShaderCSO->Release();
    PixelShaderCSO->Release();

    VertexShaderCSO = nullptr;
    PixelShaderCSO = nullptr;

    /////////////////////////////
    // axisline
    D3DCompileFromFile(L"Shaders/EditorShader.hlsl", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "axisVS", "vs_5_0", 0, 0, &VertexShaderCSO, &errorBlob);
    if (errorBlob)
    {
        OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        errorBlob->Release();
    }
    Renderer->Graphics->Device->CreateVertexShader(VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), nullptr, &Resources.Shaders.AxisLine.Vertex);

    D3DCompileFromFile(L"Shaders/EditorShader.hlsl", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "axisPS", "ps_5_0", 0, 0, &PixelShaderCSO, &errorBlob);

    Renderer->Graphics->Device->CreatePixelShader(PixelShaderCSO->GetBufferPointer(), PixelShaderCSO->GetBufferSize(), nullptr, &Resources.Shaders.AxisLine.Pixel);
    
    // AxisLine은 layout을 받지 않고, SV_VertexID를 사용합니다.
    Resources.Shaders.AxisLine.Layout = nullptr;
    Resources.Shaders.AxisLine.Topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;

    VertexShaderCSO->Release();
    PixelShaderCSO->Release();

    VertexShaderCSO = nullptr;
    PixelShaderCSO = nullptr;

    /////////////////////////////
    // AABB
    D3DCompileFromFile(L"Shaders/EditorShader.hlsl", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "aabbVS", "vs_5_0", 0, 0, &VertexShaderCSO, &errorBlob);
    if (errorBlob)
    {
        OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        errorBlob->Release();
    }
    Renderer->Graphics->Device->CreateVertexShader(VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), nullptr, &Resources.Shaders.AABB.Vertex);

    D3DCompileFromFile(L"Shaders/EditorShader.hlsl", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "aabbPS", "ps_5_0", 0, 0, &PixelShaderCSO, &errorBlob);

    Renderer->Graphics->Device->CreatePixelShader(PixelShaderCSO->GetBufferPointer(), PixelShaderCSO->GetBufferSize(), nullptr, &Resources.Shaders.AABB.Pixel);

    // Box의 vertex
    D3D11_INPUT_ELEMENT_DESC layout1[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    Renderer->Graphics->Device->CreateInputLayout(
        layout1, ARRAYSIZE(layout1), VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), &Resources.Shaders.AABB.Layout
    );
    Resources.Shaders.AABB.Topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;

    VertexShaderCSO->Release();
    PixelShaderCSO->Release();

    VertexShaderCSO = nullptr;
    PixelShaderCSO = nullptr;

    /////////////////////////////
    // Sphere
    D3DCompileFromFile(L"Shaders/EditorShader.hlsl", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "sphereVS", "vs_5_0", 0, 0, &VertexShaderCSO, &errorBlob);
    if (errorBlob)
    {
        OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        errorBlob->Release();
    }
    Renderer->Graphics->Device->CreateVertexShader(VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), nullptr, &Resources.Shaders.Sphere.Vertex);

    D3DCompileFromFile(L"Shaders/EditorShader.hlsl", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "spherePS", "ps_5_0", 0, 0, &PixelShaderCSO, &errorBlob);

    Renderer->Graphics->Device->CreatePixelShader(PixelShaderCSO->GetBufferPointer(), PixelShaderCSO->GetBufferSize(), nullptr, &Resources.Shaders.Sphere.Pixel);

    Renderer->Graphics->Device->CreateInputLayout(
        layout1, ARRAYSIZE(layout1), VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), &Resources.Shaders.Sphere.Layout
    );
    Resources.Shaders.Sphere.Topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;

    VertexShaderCSO->Release();
    PixelShaderCSO->Release();

    VertexShaderCSO = nullptr;
    PixelShaderCSO = nullptr;

    /////////////////////////////
    // Cone
    D3DCompileFromFile(L"Shaders/EditorShader.hlsl", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "coneVS", "vs_5_0", 0, 0, &VertexShaderCSO, &errorBlob);
    if (errorBlob)
    {
        OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        errorBlob->Release();
    }
    Renderer->Graphics->Device->CreateVertexShader(VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), nullptr, &Resources.Shaders.Cone.Vertex);

    D3DCompileFromFile(L"Shaders/EditorShader.hlsl", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "conePS", "ps_5_0", 0, 0, &PixelShaderCSO, &errorBlob);

    Renderer->Graphics->Device->CreatePixelShader(PixelShaderCSO->GetBufferPointer(), PixelShaderCSO->GetBufferSize(), nullptr, &Resources.Shaders.Cone.Pixel);

    Renderer->Graphics->Device->CreateInputLayout(
        layout1, ARRAYSIZE(layout1), VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), &Resources.Shaders.Cone.Layout
    );
    Resources.Shaders.Cone.Topology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;

    VertexShaderCSO->Release();
    PixelShaderCSO->Release();
}

void FEditorRenderer::PrepareShader(FShaderResource ShaderResource) const
{
    Renderer->Graphics->DeviceContext->VSSetShader(ShaderResource.Vertex, nullptr, 0);
    Renderer->Graphics->DeviceContext->PSSetShader(ShaderResource.Pixel, nullptr, 0);
    Renderer->Graphics->DeviceContext->IASetInputLayout(ShaderResource.Layout);
    Renderer->Graphics->DeviceContext->IASetPrimitiveTopology(ShaderResource.Topology);
}

void FEditorRenderer::ReleaseShaders()
{

    if (Resources.Shaders.Line.Pixel)
    {
        Resources.Shaders.Line.Pixel->Release();
        Resources.Shaders.Line.Pixel = nullptr;
    }
    if (Resources.Shaders.Line.Vertex)
    {
        Resources.Shaders.Line.Vertex->Release();
        Resources.Shaders.Line.Vertex = nullptr;
    }
    if (Resources.Shaders.Line.Layout)
    {
        Resources.Shaders.Line.Layout->Release();
        Resources.Shaders.Line.Layout = nullptr;
    }
}

void FEditorRenderer::CreateBuffers()
{
    ////////////////////////////////////
    // Box 버퍼 생성
    TArray<FVector> CubeFrameVertices;
    CubeFrameVertices.Add({ -1.f, -1.f, -1.f}); // 0
    CubeFrameVertices.Add({ -1.f, 1.f, -1.f}); // 1
    CubeFrameVertices.Add({ 1.f, -1.f, -1.f}); // 2
    CubeFrameVertices.Add({ 1.f, 1.f, -1.f}); // 3
    CubeFrameVertices.Add({ -1.f, -1.f, 1.f}); // 4
    CubeFrameVertices.Add({ 1.f, -1.f, 1.f}); // 5
    CubeFrameVertices.Add({ -1.f, 1.f, 1.f}); // 6
    CubeFrameVertices.Add({ 1.f, 1.f, 1.f}); // 7

    TArray<uint32> CubeFrameIndices = {
        // Bottom face
        0, 1, 1, 3, 3, 2, 2, 0,
        // Top face
        4, 6, 6, 7, 7, 5, 5, 4,
        // Side faces
        0, 4, 1, 6, 2, 5, 3, 7
    };

    // 버텍스 버퍼 생성
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // will never be updated 
    bufferDesc.ByteWidth = sizeof(FVector) * CubeFrameVertices.Num();
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = CubeFrameVertices.GetData();

    HRESULT hr = Renderer->Graphics->Device->CreateBuffer(&bufferDesc, &initData, &Resources.Primitives.Box.Vertex);

    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.ByteWidth = sizeof(uint32) * CubeFrameIndices.Num();

    initData.pSysMem = CubeFrameIndices.GetData();

    hr = Renderer->Graphics->Device->CreateBuffer(&bufferDesc, &initData, &Resources.Primitives.Box.Index);

    Resources.Primitives.Box.NumVertices = CubeFrameVertices.Num();
    Resources.Primitives.Box.VertexStride = sizeof(FVector);
    Resources.Primitives.Box.NumIndices = CubeFrameIndices.Num();

    ////////////////////////////////////
    // Sphere 버퍼 생성
    FVector SphereFrameVertices[] =
    {
        {1.0, 0.0, 0},
        {0.9795299412524945, 0.20129852008866006, 0},
        {0.9189578116202306, 0.39435585511331855, 0},
        {0.8207634412072763, 0.5712682150947923, 0},
        {0.6889669190756866, 0.72479278722912, 0},
        {0.5289640103269624, 0.8486442574947509, 0},
        {0.3473052528448203, 0.9377521321470804, 0},
        {0.1514277775045767, 0.9884683243281114, 0},
        {-0.05064916883871264, 0.9987165071710528, 0},
        {-0.2506525322587204, 0.9680771188662043, 0},
        {-0.4403941515576344, 0.8978045395707416, 0},
        {-0.6121059825476629, 0.7907757369376985, 0},
        {-0.758758122692791, 0.6513724827222223, 0},
        {-0.8743466161445821, 0.48530196253108104, 0},
        {-0.9541392564000488, 0.29936312297335804, 0},
        {-0.9948693233918952, 0.10116832198743228, 0},
        {-0.9948693233918952, -0.10116832198743204, 0},
        {-0.9541392564000489, -0.29936312297335776, 0},
        {-0.8743466161445822, -0.4853019625310808, 0},
        {-0.7587581226927911, -0.651372482722222, 0},
        {-0.6121059825476627, -0.7907757369376986, 0},
        {-0.44039415155763423, -0.8978045395707417, 0},
        {-0.2506525322587205, -0.9680771188662043, 0},
        {-0.05064916883871266, -0.9987165071710528, 0},
        {0.15142777750457667, -0.9884683243281114, 0},
        {0.3473052528448203, -0.9377521321470804, 0},
        {0.5289640103269624, -0.8486442574947509, 0},
        {0.6889669190756865, -0.72479278722912, 0},
        {0.8207634412072763, -0.5712682150947924, 0},
        {0.9189578116202306, -0.3943558551133187, 0},
        {0.9795299412524945, -0.20129852008866028, 0},
        {1, 0, 0},
        {1.0, 0, 0.0},
        {0.9795299412524945, 0, 0.20129852008866006},
        {0.9189578116202306, 0, 0.39435585511331855},
        {0.8207634412072763, 0, 0.5712682150947923},
        {0.6889669190756866, 0, 0.72479278722912},
        {0.5289640103269624, 0, 0.8486442574947509},
        {0.3473052528448203, 0, 0.9377521321470804},
        {0.1514277775045767, 0, 0.9884683243281114},
        {-0.05064916883871264, 0, 0.9987165071710528},
        {-0.2506525322587204, 0, 0.9680771188662043},
        {-0.4403941515576344, 0, 0.8978045395707416},
        {-0.6121059825476629, 0, 0.7907757369376985},
        {-0.758758122692791, 0, 0.6513724827222223},
        {-0.8743466161445821, 0, 0.48530196253108104},
        {-0.9541392564000488, 0, 0.29936312297335804},
        {-0.9948693233918952, 0, 0.10116832198743228},
        {-0.9948693233918952, 0, -0.10116832198743204},
        {-0.9541392564000489, 0, -0.29936312297335776},
        {-0.8743466161445822, 0, -0.4853019625310808},
        {-0.7587581226927911, 0, -0.651372482722222},
        {-0.6121059825476627, 0, -0.7907757369376986},
        {-0.44039415155763423, 0, -0.8978045395707417},
        {-0.2506525322587205, 0, -0.9680771188662043},
        {-0.05064916883871266, 0, -0.9987165071710528},
        {0.15142777750457667, 0, -0.9884683243281114},
        {0.3473052528448203, 0, -0.9377521321470804},
        {0.5289640103269624, 0, -0.8486442574947509},
        {0.6889669190756865, 0, -0.72479278722912},
        {0.8207634412072763, 0, -0.5712682150947924},
        {0.9189578116202306, 0, -0.3943558551133187},
        {0.9795299412524945, 0, -0.20129852008866028},
        {1, 0, 0},
        {0, 1.0, 0.0},
        {0, 0.9795299412524945, 0.20129852008866006},
        {0, 0.9189578116202306, 0.39435585511331855},
        {0, 0.8207634412072763, 0.5712682150947923},
        {0, 0.6889669190756866, 0.72479278722912},
        {0, 0.5289640103269624, 0.8486442574947509},
        {0, 0.3473052528448203, 0.9377521321470804},
        {0, 0.1514277775045767, 0.9884683243281114},
        {0, -0.05064916883871264, 0.9987165071710528},
        {0, -0.2506525322587204, 0.9680771188662043},
        {0, -0.4403941515576344, 0.8978045395707416},
        {0, -0.6121059825476629, 0.7907757369376985},
        {0, -0.758758122692791, 0.6513724827222223},
        {0, -0.8743466161445821, 0.48530196253108104},
        {0, -0.9541392564000488, 0.29936312297335804},
        {0, -0.9948693233918952, 0.10116832198743228},
        {0, -0.9948693233918952, -0.10116832198743204},
        {0, -0.9541392564000489, -0.29936312297335776},
        {0, -0.8743466161445822, -0.4853019625310808},
        {0, -0.7587581226927911, -0.651372482722222},
        {0, -0.6121059825476627, -0.7907757369376986},
        {0, -0.44039415155763423, -0.8978045395707417},
        {0, -0.2506525322587205, -0.9680771188662043},
        {0, -0.05064916883871266, -0.9987165071710528},
        {0, 0.15142777750457667, -0.9884683243281114},
        {0, 0.3473052528448203, -0.9377521321470804},
        {0, 0.5289640103269624, -0.8486442574947509},
        {0, 0.6889669190756865, -0.72479278722912},
        {0, 0.8207634412072763, -0.5712682150947924},
        {0, 0.9189578116202306, -0.3943558551133187},
        {0, 0.9795299412524945, -0.20129852008866028},
        {0, 1, 0}
    };

    uint32 SphereFrameIndices[] =
    {
        0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10,
        11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20,
        21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30,
        31, 32, 33, 33, 34, 34, 35, 35, 36, 36, 37, 37, 38, 38, 39, 39, 40, 40,
        41, 41, 42, 42, 43, 43, 44, 44, 45, 45, 46, 46, 47, 47, 48, 48, 49, 49, 50, 50,
        51, 51, 52, 52, 53, 53, 54, 54, 55, 55, 56, 56, 57, 57, 58, 58, 59, 59, 60, 60,
        61, 61, 62, 62, 63, 64, 65, 65, 66, 66, 67, 67, 68, 68, 69, 69, 70, 70,
        71, 71, 72, 72, 73, 73, 74, 74, 75, 75, 76, 76, 77, 77, 78, 78, 79, 79, 80, 80,
        81, 81, 82, 82, 83, 83, 84, 84, 85, 85, 86, 86, 87, 87, 88, 88, 89, 89, 90, 90,
        91, 91, 92, 92, 93, 93, 94, 94, 95
    };

    // 버텍스 버퍼 생성
    bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // will never be updated 
    bufferDesc.ByteWidth = sizeof(SphereFrameVertices);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;

    initData = {};
    initData.pSysMem = SphereFrameVertices;

    hr = Renderer->Graphics->Device->CreateBuffer(&bufferDesc, &initData, &Resources.Primitives.Sphere.Vertex);

    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.ByteWidth = sizeof(SphereFrameIndices);

    initData.pSysMem = SphereFrameIndices;

    hr = Renderer->Graphics->Device->CreateBuffer(&bufferDesc, &initData, &Resources.Primitives.Sphere.Index);

    Resources.Primitives.Sphere.NumVertices = ARRAYSIZE(SphereFrameVertices);
    Resources.Primitives.Sphere.VertexStride = sizeof(FVector);
    Resources.Primitives.Sphere.NumIndices = ARRAYSIZE(SphereFrameIndices);



    ////////////////////////////////////
    // Cone 버퍼 생성
    uint32 NumSegments = 32;
    TArray<FVector> ConeVertices;
    ConeVertices.Add({0.0f, 0.0f, 0.0f}); // Apex
    for (int i = 0; i < NumSegments; i++)
    {
        float angle = 2.0f * 3.1415926535897932f * i / (float)NumSegments;
        float x = cos(angle);
        float y = sin(angle);
        ConeVertices.Add({x, y, 1.0f}); // Bottom
    }
    TArray<uint32> ConeIndices;
    for (int i = 0; i < NumSegments-1; i++)
    {
        ConeIndices.Add(0);
        ConeIndices.Add(i + 1);
        ConeIndices.Add(i + 2);
    }
    ConeIndices.Add(0);
    ConeIndices.Add(NumSegments);
    ConeIndices.Add(1);


    // 버텍스 버퍼 생성
    bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // will never be updated 
    bufferDesc.ByteWidth = ConeVertices.Num() * sizeof(FVector);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;

    initData = {};
    initData.pSysMem = ConeVertices.GetData();

    hr = Renderer->Graphics->Device->CreateBuffer(&bufferDesc, &initData, &Resources.Primitives.Cone.Vertex);

    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.ByteWidth = ConeIndices.Num() * sizeof(FVector);

    initData.pSysMem = ConeIndices.GetData();

    hr = Renderer->Graphics->Device->CreateBuffer(&bufferDesc, &initData, &Resources.Primitives.Cone.Index);

    Resources.Primitives.Cone.NumVertices = ConeVertices.Num();
    Resources.Primitives.Cone.VertexStride = sizeof(FVector);
    Resources.Primitives.Cone.NumIndices = ConeIndices.Num();

}

void FEditorRenderer::CreateConstantBuffers()
{
    D3D11_BUFFER_DESC ConstantBufferDesc = {};
    ConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    ConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    ConstantBufferDesc.ByteWidth = sizeof(FConstantBufferCamera);
    Renderer->Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &Resources.ConstantBuffers.Camera00);

    // 16개 고정
    // 그려야할 대상이 더 많을 경우 16개씩 쪼개서 사용
    ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    ConstantBufferDesc.ByteWidth = sizeof(FConstantBufferDebugAABB) * ConstantBufferSizeAABB;
    Renderer->Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &Resources.ConstantBuffers.AABB13);

    ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    ConstantBufferDesc.ByteWidth = sizeof(FConstantBufferDebugSphere) * ConstantBufferSizeSphere;
    Renderer->Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &Resources.ConstantBuffers.Sphere13);

    ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    ConstantBufferDesc.ByteWidth = sizeof(FConstantBufferDebugCone) * ConstantBufferSizeCone;
    Renderer->Graphics->Device->CreateBuffer(&ConstantBufferDesc, nullptr, &Resources.ConstantBuffers.Cone13);

}

void FEditorRenderer::PreparePrimitives()
{
    Resources.Components.PrimitiveObjs.Empty();
    // gizmo 제외하고 넣기
    if (GEngine->GetWorld()->WorldType == EWorldType::Editor)
    {
        for (const auto iter : TObjectRange<UPrimitiveComponent>())
        {
            if (!Cast<UGizmoBaseComponent>(iter))
                Resources.Components.PrimitiveObjs.Add(iter);
        }
        for (const auto iter : TObjectRange<ULightComponentBase>())
        {
            Resources.Components.LightObjs.Add(iter);
        }
    }
}

void FEditorRenderer::PrepareConstantbufferGlobal()
{
    if (Resources.ConstantBuffers.Camera00)
    {
        Renderer->Graphics->DeviceContext->VSSetConstantBuffers(0, 1, &Resources.ConstantBuffers.Camera00);
    }
}

void FEditorRenderer::UpdateConstantbufferGlobal(FConstantBufferCamera Buffer)
{
    if (Resources.ConstantBuffers.Camera00)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����

        Renderer->Graphics->DeviceContext->Map(Resources.ConstantBuffers.Camera00, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        memcpy(ConstantBufferMSR.pData, &Buffer, sizeof(Buffer));
        Renderer->Graphics->DeviceContext->Unmap(Resources.ConstantBuffers.Camera00, 0); // GPU�� �ٽ� ��밡���ϰ� �����
    }
}

void FEditorRenderer::Render(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    ID3D11DepthStencilState* DepthStateDisable = Renderer->Graphics->DepthStateDisable;
    Renderer->Graphics->DeviceContext->OMSetDepthStencilState(DepthStateDisable, 0);
    PreparePrimitives();
    PrepareConstantbufferGlobal();
    FConstantBufferCamera buf;
    buf.ViewMatrix = ActiveViewport->GetViewMatrix();
    buf.ProjMatrix = ActiveViewport->GetProjectionMatrix();
    UpdateConstantbufferGlobal(buf);

    RenderAABBInstanced(World);
    RenderPointlightInstanced(World);
    RenderSpotlightInstanced(World);
    RenderAxis(ActiveViewport);
    RenderGizmos(World);
}

void FEditorRenderer::RenderGizmos(const UWorld* World)
{
    PrepareShader(Resources.Shaders.Gizmo);

    //Renderer->PrepareShader(Renderer->RenderResources.Shaders.StaticMesh);
    //Renderer->PrepareShader(Resources.Shaders.Gizmo);

    if (!World->GetSelectedActor())
    {
        return;
    }

    TArray<UGizmoBaseComponent*> GizmoObjs;

    for (const auto iter : TObjectRange<UGizmoBaseComponent>())
    {
        GizmoObjs.Add(iter);
    }

    //  fill solid,  Wirframe 에서도 제대로 렌더링되기 위함
    Renderer->Graphics->DeviceContext->RSSetState(UEditorEngine::graphicDevice.RasterizerStateSOLID);

    for (auto GizmoComp : GizmoObjs)
    {

        if ((GizmoComp->GetGizmoType() == UGizmoBaseComponent::ArrowX ||
            GizmoComp->GetGizmoType() == UGizmoBaseComponent::ArrowY ||
            GizmoComp->GetGizmoType() == UGizmoBaseComponent::ArrowZ)
            && World->GetEditorPlayer()->GetControlMode() != CM_TRANSLATION)
            continue;
        else if ((GizmoComp->GetGizmoType() == UGizmoBaseComponent::ScaleX ||
            GizmoComp->GetGizmoType() == UGizmoBaseComponent::ScaleY ||
            GizmoComp->GetGizmoType() == UGizmoBaseComponent::ScaleZ)
            && World->GetEditorPlayer()->GetControlMode() != CM_SCALE)
            continue;
        else if ((GizmoComp->GetGizmoType() == UGizmoBaseComponent::CircleX ||
            GizmoComp->GetGizmoType() == UGizmoBaseComponent::CircleY ||
            GizmoComp->GetGizmoType() == UGizmoBaseComponent::CircleZ)
            && World->GetEditorPlayer()->GetControlMode() != CM_ROTATION)
            continue;
        FMatrix Model = GizmoComp->GetComponentTransform();
        {
            FConstantBufferActor buf;
            buf.IsSelectedActor = 0;
            buf.UUID = GizmoComp->EncodeUUID() / 255.0f;
            Renderer->UpdateConstantbufferActor(buf);
        }
        {
            FConstantBufferMesh buf;
            buf.ModelMatrix = Model;
            if (GizmoComp == World->GetPickingGizmo())
            {
                buf.IsSelectedMesh = 1;
            }
            else
            {
                buf.IsSelectedMesh = 0;
            }
            Renderer->UpdateConstantbufferMesh(buf);
        }
        // TODO: 기즈모 선택효과 안보임
        // 현재 RenderPrimitive()에서 다시 SelectedMesh를 갱신하는데, 안에서 기즈모를 인식시킬 수 없음.

        if (!GizmoComp->GetStaticMesh()) continue;

        OBJ::FStaticMeshRenderData* renderData = GizmoComp->GetStaticMesh()->GetRenderData();
        if (renderData == nullptr) continue;

        Renderer->RenderPrimitive(Model, renderData, GizmoComp->GetStaticMesh()->GetMaterials(), GizmoComp->GetOverrideMaterials(), -1);

    }

    Renderer->Graphics->DeviceContext->RSSetState(Renderer->Graphics->GetCurrentRasterizer());

#pragma region GizmoDepth
    ID3D11DepthStencilState* originalDepthState = Renderer->Graphics->DepthStencilState;
    Renderer->Graphics->DeviceContext->OMSetDepthStencilState(originalDepthState, 0);
#pragma endregion GizmoDepth
}

void FEditorRenderer::PrepareShaderGizmo()
{
    
}

void FEditorRenderer::PrepareConstantbufferGizmo()
{
}

void FEditorRenderer::RenderAxis(const std::shared_ptr<FEditorViewportClient>& ActiveViewport)
{
    PrepareShader(Resources.Shaders.AxisLine);

    Renderer->Graphics->DeviceContext->Draw(6, 0);
}

void FEditorRenderer::PrepareShaderAxis()
{

}

void FEditorRenderer::RenderAABBInstanced(const UWorld* World)
{
    PrepareShader(Resources.Shaders.AABB);
    UINT offset = 0;
    Renderer->Graphics->DeviceContext->IASetVertexBuffers(0, 1, &Resources.Primitives.Box.Vertex, &Resources.Primitives.Box.VertexStride, &offset);
    Renderer->Graphics->DeviceContext->IASetIndexBuffer(Resources.Primitives.Box.Index, DXGI_FORMAT_R32_UINT, 0);

    // 위치랑 bounding box 크기 정보 가져오기
    TArray<FConstantBufferDebugAABB> BufferAll;
    for (UPrimitiveComponent* PrimComp : Resources.Components.PrimitiveObjs)
    //for (UPrimitiveComponent* StaticComp : Resources.Components.PrimitiveObjs)
    {
        // 현재 bounding box를 갱신안해주고있음 : 여기서 직접 갱신
        if (UStaticMeshComponent* StaticComp = Cast<UStaticMeshComponent>(PrimComp))
        {
            StaticComp->UpdateWorldAABB();
            FConstantBufferDebugAABB b;
            b.Position = StaticComp->GetBoundingBoxWorld().GetPosition();
            b.Extent = StaticComp->GetBoundingBoxWorld().GetExtent();
            BufferAll.Add(b);
        }
    }
  
    int BufferIndex = 0;
    for (int i = 0; i < (1 + BufferAll.Num() / ConstantBufferSizeAABB) * ConstantBufferSizeAABB; ++i)
    {
        TArray<FConstantBufferDebugAABB> SubBuffer;
        for (int j = 0; j < ConstantBufferSizeAABB; ++j)
        {
            if (BufferIndex < BufferAll.Num())
            {
                SubBuffer.Add(BufferAll[BufferIndex]);
                ++BufferIndex;
            }
            else
            {
                break;
            }
        }

        if (SubBuffer.Num() > 0)
        {
            UdpateConstantbufferAABBInstanced(SubBuffer);
            PrepareConstantbufferAABB();
            Renderer->Graphics->DeviceContext->DrawIndexedInstanced(Resources.Primitives.Box.NumIndices, SubBuffer.Num(), 0, 0, 0);
        }
    }
}

void FEditorRenderer::PrepareConstantbufferAABB()
{
    if (Resources.ConstantBuffers.AABB13)
    {
        Renderer->Graphics->DeviceContext->VSSetConstantBuffers(13, 1, &Resources.ConstantBuffers.AABB13);
    }
}

void FEditorRenderer::UdpateConstantbufferAABBInstanced(TArray<FConstantBufferDebugAABB> Buffer)
{
    if (Buffer.Num() > ConstantBufferSizeAABB)
    {
        // 최대개수 초과
        // 코드 잘못짠거 아니면 오면안됨
        UE_LOG(LogLevel::Error, "Invalid Buffer Num");
        return;
    }
    if (Resources.ConstantBuffers.AABB13)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����

        Renderer->Graphics->DeviceContext->Map(Resources.ConstantBuffers.AABB13, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        memcpy(ConstantBufferMSR.pData, Buffer.GetData(), sizeof(FConstantBufferDebugAABB) * Buffer.Num()); // TArray이니까 실제 값을 받아와야함
        Renderer->Graphics->DeviceContext->Unmap(Resources.ConstantBuffers.AABB13, 0); // GPU�� �ٽ� ��밡���ϰ� �����
    }
}

void FEditorRenderer::RenderPointlightInstanced(const UWorld* World)
{
    PrepareShader(Resources.Shaders.Sphere);
    UINT offset = 0;
    Renderer->Graphics->DeviceContext->IASetVertexBuffers(0, 1, &Resources.Primitives.Sphere.Vertex, &Resources.Primitives.Sphere.VertexStride, &offset);
    Renderer->Graphics->DeviceContext->IASetIndexBuffer(Resources.Primitives.Sphere.Index, DXGI_FORMAT_R32_UINT, 0);

    // 위치랑 bounding box 크기 정보 가져오기
    TArray<FConstantBufferDebugSphere> BufferAll;
    for (UPrimitiveComponent* PrimComp : Resources.Components.PrimitiveObjs)
        //for (UPrimitiveComponent* StaticComp : Resources.Components.PrimitiveObjs)
    {
        // Fireball 합치면 헤더랑 여기 풀기
        
        if (UFireBallComponent* FireBallComp = Cast<UFireBallComponent>(PrimComp))
        {
            FConstantBufferDebugSphere b;
            b.Position = FireBallComp->GetComponentLocation();
            b.Radius = FireBallComp->Radius;
            BufferAll.Add(b);
        }

    }

    int BufferIndex = 0;
    for (int i = 0; i < (1 + BufferAll.Num() / ConstantBufferSizeSphere) * ConstantBufferSizeSphere; ++i)
    {
        TArray<FConstantBufferDebugSphere> SubBuffer;
        for (int j = 0; j < ConstantBufferSizeAABB; ++j)
        {
            if (BufferIndex < BufferAll.Num())
            {
                SubBuffer.Add(BufferAll[BufferIndex]);
                ++BufferIndex;
            }
            else
            {
                break;
            }
        }

        if (SubBuffer.Num() > 0)
        {
            UdpateConstantbufferPointlightInstanced(SubBuffer);
            PrepareConstantbufferPointlight();
            Renderer->Graphics->DeviceContext->DrawIndexedInstanced(Resources.Primitives.Sphere.NumIndices, SubBuffer.Num(), 0, 0, 0);
        }
    }
}

void FEditorRenderer::PrepareConstantbufferPointlight()
{
    if (Resources.ConstantBuffers.Sphere13)
    {
        Renderer->Graphics->DeviceContext->VSSetConstantBuffers(13, 1, &Resources.ConstantBuffers.Sphere13);
    }
}

void FEditorRenderer::UdpateConstantbufferPointlightInstanced(TArray<FConstantBufferDebugSphere> Buffer)
{
    if (Buffer.Num() > ConstantBufferSizeSphere)
    {
        // 최대개수 초과
        // 코드 잘못짠거 아니면 오면안됨
        UE_LOG(LogLevel::Error, "Invalid Buffer Num");
        return;
    }
    if (Resources.ConstantBuffers.Sphere13)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����

        Renderer->Graphics->DeviceContext->Map(Resources.ConstantBuffers.Sphere13, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        memcpy(ConstantBufferMSR.pData, Buffer.GetData(), sizeof(FConstantBufferDebugSphere) * Buffer.Num()); // TArray이니까 실제 값을 받아와야함
        Renderer->Graphics->DeviceContext->Unmap(Resources.ConstantBuffers.Sphere13, 0); // GPU�� �ٽ� ��밡���ϰ� �����
    }
}

void FEditorRenderer::RenderSpotlightInstanced(const UWorld* World)
{
    PrepareShader(Resources.Shaders.Cone);
    UINT offset = 0;
    Renderer->Graphics->DeviceContext->IASetVertexBuffers(0, 1, &Resources.Primitives.Cone.Vertex, &Resources.Primitives.Cone.VertexStride, &offset);
    Renderer->Graphics->DeviceContext->IASetIndexBuffer(Resources.Primitives.Cone.Index, DXGI_FORMAT_R32_UINT, 0);

    // 위치랑 bounding box 크기 정보 가져오기
    TArray<FConstantBufferDebugCone> BufferAll;
    for (ULightComponentBase* LightComp : Resources.Components.LightObjs)
        //for (UPrimitiveComponent* StaticComp : Resources.Components.PrimitiveObjs)
    {
        FConstantBufferDebugCone b;
        b.ApexPosiiton = LightComp->GetComponentLocation();
        b.Radius = LightComp->GetRadius();
        b.Height = 10.f;
        b.Direction = LightComp->GetUpVector();
        BufferAll.Add(b);
    }

    int BufferIndex = 0;
    for (int i = 0; i < (1 + BufferAll.Num() / ConstantBufferSizeCone) * ConstantBufferSizeCone; ++i)
    {
        TArray<FConstantBufferDebugCone> SubBuffer;
        for (int j = 0; j < ConstantBufferSizeAABB; ++j)
        {
            if (BufferIndex < BufferAll.Num())
            {
                SubBuffer.Add(BufferAll[BufferIndex]);
                ++BufferIndex;
            }
            else
            {
                break;
            }
        }

        if (SubBuffer.Num() > 0)
        {
            UdpateConstantbufferSpotlightInstanced(SubBuffer);
            PrepareConstantbufferSpotlight();
            Renderer->Graphics->DeviceContext->DrawIndexedInstanced(Resources.Primitives.Cone.NumIndices, SubBuffer.Num(), 0, 0, 0);
        }
    }
}

void FEditorRenderer::PrepareConstantbufferSpotlight()
{
    if (Resources.ConstantBuffers.Cone13)
    {
        Renderer->Graphics->DeviceContext->VSSetConstantBuffers(13, 1, &Resources.ConstantBuffers.Cone13);
    }
}

void FEditorRenderer::UdpateConstantbufferSpotlightInstanced(TArray<FConstantBufferDebugCone> Buffer)
{
    if (Buffer.Num() > ConstantBufferSizeCone)
    {
        // 최대개수 초과
        // 코드 잘못짠거 아니면 오면안됨
        UE_LOG(LogLevel::Error, "Invalid Buffer Num");
        return;
    }
    if (Resources.ConstantBuffers.Cone13)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����

        Renderer->Graphics->DeviceContext->Map(Resources.ConstantBuffers.Cone13, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        memcpy(ConstantBufferMSR.pData, Buffer.GetData(), sizeof(FConstantBufferDebugCone) * Buffer.Num()); // TArray이니까 실제 값을 받아와야함
        Renderer->Graphics->DeviceContext->Unmap(Resources.ConstantBuffers.Cone13, 0); // GPU�� �ٽ� ��밡���ϰ� �����
    }
}
