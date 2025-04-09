#include "EditorShaderConstants.hlsli"
#include "ShaderConstants.hlsli"

// Input Layout은 float3이지만, shader에서 missing w = 1로 처리해서 사용
// https://stackoverflow.com/questions/29728349/hlsl-sv-position-why-how-from-float3-to-float4
struct VS_INPUT
{
    float4 position : POSITION; // 버텍스 위치
    float4 color : COLOR; // 버텍스 색상
    float3 normal : NORMAL; // 버텍스 노멀
    float2 texcoord : TEXCOORD;
    int materialIndex : MATERIAL_INDEX;
};

struct PS_INPUT
{
    float4 position : SV_Position;
    float4 color : COLOR;
};

/////////////////////////////////////////////
// GIZMO
PS_INPUT gizmoVS(VS_INPUT input)
{
    PS_INPUT output;
    
    float4 pos;
    pos = mul(input.position, ModelMatrix);
    pos = mul(pos, ViewMatrix);
    pos = mul(pos, ProjMatrix);
    
    output.position = pos;
    
    output.color = float4(Material.DiffuseColor, 1.f);
    
    return output;
}

float4 gizmoPS(PS_INPUT input) : SV_Target
{
    return input.color;
}

/////////////////////////////////////////////
// Axis
// Input buffer는 없고 대신 Draw(6)하면됨.

const static float4 AxisPos[6] =
{
    float4(0, 0, 0, 1),
    float4(10000000, 0, 0, 1),
    float4(0, 0, 0, 1),
    float4(0, 10000000, 0, 1),
    float4(0, 0, 0, 1),
    float4(0, 0, 10000000, 1)
};

const static float4 AxisColor[3] =
{
    float4(1, 0, 0, 1),
    float4(0, 1, 0, 1),
    float4(0, 0, 1, 1)
};

// Draw()에서 NumVertices만큼 SV_VertexID만 다른채로 호출됨.
// 어차피 월드에 하나이므로 Vertex를 받지않음.
PS_INPUT axisVS(uint vertexID : SV_VertexID)
{
    PS_INPUT output;
    
    float4 Vertex = AxisPos[vertexID];
    Vertex = mul(Vertex, ViewMatrix);
    Vertex = mul(Vertex, ProjMatrix);
    output.position = Vertex;
    
    output.color = AxisColor[vertexID / 2];
    
    return output;
}

float4 axisPS(PS_INPUT input) : SV_Target
{
    return input.color;
}

/////////////////////////////////////////////
// AABB
struct VS_INPUT_POS_ONLY
{
    float4 position : POSITION0;
};

PS_INPUT aabbVS(VS_INPUT_POS_ONLY input, uint instanceID : SV_InstanceID)
{
    PS_INPUT output;
    
    float3 pos = DataAABB[instanceID].Position;
    float3 scale = DataAABB[instanceID].Extent;
    //scale = float3(1, 1, 1);
    
    float4 localPos = float4(input.position.xyz * scale + pos, 1.f);
        
    localPos = mul(localPos, ViewMatrix);
    localPos = mul(localPos, ProjMatrix);
    output.position = localPos;
    
    // color는 지정안해줌
    
    return output;
}

float4 aabbPS(PS_INPUT input) : SV_Target
{
    return float4(1.0f, 1.0f, 0.0f, 1.0f); // 노란색 AABB
}

/////////////////////////////////////////////
// Sphere
PS_INPUT sphereVS(VS_INPUT_POS_ONLY input, uint instanceID : SV_InstanceID)
{
    PS_INPUT output;
    
    float3 pos = DataSphere[instanceID].Position;
    float scale = DataSphere[instanceID].Radius;
    //scale = float3(1, 1, 1);
    
    float4 localPos = float4(input.position.xyz * scale + pos, 1.f);
        
    localPos = mul(localPos, ViewMatrix);
    localPos = mul(localPos, ProjMatrix);
    output.position = localPos;
    
    // color는 지정안해줌
    
    return output;
}

float4 spherePS(PS_INPUT input) : SV_Target
{
    return float4(0.777f, 1.0f, 1.0f, 1.0f); // 하늘색
}

/////////////////////////////////////////////
// Cone
float3x3 CreateRotationMatrixFromZ(float3 targetDir)
{
    float3 from = float3(0.0f, 0.0f, 1.0f); // 기준 방향
    float3 to = normalize(targetDir); // 타겟 방향 정규화

    float cosTheta = dot(from, to);

    // 이미 정렬된 경우: 단위 행렬 반환
    if (cosTheta > 0.9999f)
    {
        return float3x3(
            1, 0, 0,
            0, 1, 0,
            0, 0, 1
        );
    }

    // 반대 방향인 경우: 180도 회전, 축은 X축이나 Y축 아무거나 가능
    if (cosTheta < -0.9999f)
    {
        float3 up = float3(0.0f, 1.0f, 0.0f);
        float3 axis = normalize(cross(from, up));
        float x = axis.x, y = axis.y, z = axis.z;
        float3x3 rot180 = float3x3(
            -1 + 2 * x * x, 2 * x * y, 2 * x * z,
                2 * x * y, -1 + 2 * y * y, 2 * y * z,
                2 * x * z, 2 * y * z, -1 + 2 * z * z
        );
        return rot180;
    }

    // 일반적인 경우: Rodrigues' rotation formula
    float3 axis = normalize(cross(to, from)); // 왼손 좌표계 보정
    float s = sqrt(1.0f - cosTheta * cosTheta); // sin(theta)
    float3x3 K = float3x3(
         0, -axis.z, axis.y,
         axis.z, 0, -axis.x,
        -axis.y, axis.x, 0
    );

    float3x3 I = float3x3(1, 0, 0, 0, 1, 0, 0, 0, 1);
    float3x3 R = I + s * K + (1 - cosTheta) * mul(K, K);
    return R;
}
PS_INPUT coneVS(VS_INPUT_POS_ONLY input, uint instanceID : SV_InstanceID)
{
    PS_INPUT output;
    
    float3 pos = DataCone[instanceID].ApexPosiiton;
    float3 scale = float3(DataCone[instanceID].Radius.xx, DataCone[instanceID].Height);
    float3x3 rot = CreateRotationMatrixFromZ(DataCone[instanceID].Direction);
    
    float3 localPos3 = input.position.xyz;
    localPos3 = localPos3 * scale;
    localPos3 = mul(localPos3, rot);
    localPos3 = localPos3 + pos;
    
    float4 localPos = float4(localPos3, 1.f);
        
    localPos = mul(localPos, ViewMatrix);
    localPos = mul(localPos, ProjMatrix);
    output.position = localPos;
    
    return output;
}

float4 conePS(PS_INPUT input) : SV_Target
{
    return float4(0.777f, 1.0f, 1.0f, 1.0f); // 하늘색
}

/////////////////////////////////////////////
// Grid
struct PS_INPUT_GRID
{
    float4 Position : SV_Position;
    float4 NearPoint : COLOR0;
    float4 FarPoint : COLOR1;
    float3 WorldPos : WORLD_POSITION;    
    float2 Deriv : TEXCOORD1;
    float ViewMode : TEXCOORD2;
};

static const float3 XYQuadPos[12] =
{
    float3(-1, -1,0), float3(-1, 1,0), float3(1,  -1,0), // 좌하단, 좌상단, 우하단
    float3(-1,  1,0), float3( 1, 1,0), float3(1,  -1,0), // 좌상단, 우상단, 우하단 - 오른손 좌표계
    float3(1,  -1,0), float3( 1, 1,0), float3(-1,  1,0),
    float3(1,  -1,0), float3(-1, 1,0), float3(-1, -1,0),
};

// YZ 평면: X = 0, (Y,Z) 사용
static const float3 YZQuadPos[12] =
{
    float3(0,-1, -1), float3(0,-1, 1), float3(0, 1, -1),  // 좌하단, 좌상단, 우하단
    float3(0,-1, 1), float3(0, 1, 1), float3(0, 1, -1),  // 좌상단, 우상단, 우하단 - 오른손 좌표계
    float3(0, 1, -1), float3(0, 1, 1), float3(0, -1, 1), 
    float3(0, 1, -1), float3(0,-1, 1), float3(0, -1,-1),
};

static const float3 XZQuadPos[12] =
{
    float3(-1,0, -1), float3(-1,0, 1), float3( 1, 0,-1), // 좌하단, 좌상단, 우하단
    float3(-1,0, 1 ), float3(1, 0,1), float3(  1, 0,-1), // 좌상단, 우상단, 우하단 - 오른손 좌표계
    float3(1, 0,-1 ), float3(1, 0,1), float3(  -1,0, 1),
    float3(1, 0,-1 ), float3(-1,0, 1), float3( -1,0, -1),
};

PS_INPUT_GRID gridVS(uint vertexID : SV_VertexID)
{
    // 상수버퍼의 CaemerLookAt : (screenWidth, screenHeight, ViewMode)
    
    PS_INPUT_GRID output;
    float viewMode = CameraLookAt.z;   
    float gridScale = 1000000.0f; // 최종 그리드 크기
    float3 pos;
    if (viewMode <= 2.0)
    {
        pos = XYQuadPos[vertexID];
    }
    else if (viewMode <= 4.0)
    {
        pos = XZQuadPos[vertexID];
    }
    else
    {
        pos = YZQuadPos[vertexID];
    }
    float3 vPos3 = pos * gridScale; // 정점 정의 거꾸로 되있었음..
    
    // 뷰모드에 따라 다른 카메라 오프셋 적용
    float3 offset = float3(0.0, 0.0, 0.0);
    if (viewMode <= 2.0)
    {
        offset = float3(CameraPos.x, CameraPos.y, 0.0);
    }
    else if (viewMode <= 4.0)
    {
        offset = float3(0.0, CameraPos.x, CameraPos.z);
    }
    else
    {
        offset = float3(CameraPos.y, 0.0, CameraPos.z);
    }
    vPos3 += offset;
    
    float4 vPos4 = float4(vPos3, 1.0f);
    vPos4 = mul(vPos4, ViewMatrix);
    vPos4 = mul(vPos4, ProjMatrix);
    output.Position = vPos4;
    output.WorldPos = vPos3;
    output.Deriv = 2.0 / CameraLookAt.xy;
    output.ViewMode = viewMode;
    
    return output;
}

float log10f(float x)
{
    return log(x) / log(10.0);
}

float max2(float2 v)
{
    return max(v.x, v.y);
}
// x, y 모두 음수 일 때에 패턴을 아예 출력 안하는 문제 발생
// HLSL의 fmod는 음수 입력에 대해 음수의 나머지를 반환하므로, saturate를 거치면 0에 수렴하는 문제가 있음
float modWrap(float x, float y)
{
    float m = fmod(x, y);
    return (m < 0.0) ? m + y : m;
}

float2 modWrap2(float2 xy, float y)
{
    return float2(modWrap(xy.x, y), modWrap(xy.y, y));
}

// 뷰 모드에 따른 2D 평면 좌표 반환
float2 GetPlaneCoords(float3 worldPos, float viewMode)
{
    if (viewMode <= 2.0)
        return worldPos.xy; // 뷰 모드 0~2 : XY 평면
    else if (viewMode <= 4.0)
        return worldPos.xz; // 뷰 모드 3~4 : XZ 평면
    else
        return worldPos.yz; // 뷰 모드 5 이상 : YZ 평면
}

// 주어진 평면 좌표와, 셀 크기, 미분값(dudv)에 해당하는 LOD 단계의 알파값 계산
float ComputeLODAlpha(float3 worldPos, float cellSize, float2 dudv, float viewMode)
{
    float2 planeCoords = GetPlaneCoords(worldPos, viewMode);
    float2 modResult = modWrap2(planeCoords, cellSize) / dudv;
    return max2(1.0 - abs(saturate(modResult) * 2.0 - 1.0));
}

struct PS_OUTPUT
{
    float4 Color : SV_Target;
    float Depth : SV_Depth;
};


PS_OUTPUT gridPS(PS_INPUT_GRID input)
{
    PS_OUTPUT output;
    
    // 기본 상수 값들
    const float gGridSize = 5.0;
    const float gGridMinPixelsBetweenCells = 0.5;
    const float gGridCellSize = 1.0;
    const float4 gGridColorThick = float4(0.3, 0.3, 0.3, 1.0);
    const float4 gGridColorThin = float4(0.2, 0.2, 0.2, 1.0);
    
    // 뷰모드에 따라 사용할 평면 좌표
    float2 planeCoords = GetPlaneCoords(input.WorldPos, input.ViewMode);
    float2 dvx = float2(ddx(planeCoords.x), ddy(planeCoords.x));
    float2 dvy = float2(ddx(planeCoords.y), ddy(planeCoords.y));
    
    // 최소값 클램핑 - 한 픽셀에서의 변화량이 미미할 때 떨림 현상 방지용 (효과는 없음)
    float epsilon = 1e-3;
    float lx = max(length(dvx), epsilon);
    float ly = max(length(dvy), epsilon);
    float2 dudv = float2(lx, ly);
    float l = length(dudv);

    // LOD 계산 (log10 기반)
    float LOD = max(0.0, log10f(l * gGridMinPixelsBetweenCells / gGridCellSize) + 1.0);

    float GridCellSizeLod0 = gGridCellSize * pow(10.0, floor(LOD));
    float GridCellSizeLod1 = GridCellSizeLod0 * 10.0;
    float GridCellSizeLod2 = GridCellSizeLod1 * 10.0;

    dudv *= 4.0;

    // 뷰모드에 따라 모듈러 연산에 전달할 2D 성분 결정
    float Lod0a = ComputeLODAlpha(input.WorldPos, GridCellSizeLod0, dudv, input.ViewMode);
    float Lod1a = ComputeLODAlpha(input.WorldPos, GridCellSizeLod1, dudv, input.ViewMode);
    float Lod2a = ComputeLODAlpha(input.WorldPos, GridCellSizeLod2, dudv, input.ViewMode);
    
    // LOD 페이드 (LOD의 소수 부분)
    float LODFade = frac(LOD);
    float4 Color;
    if (Lod2a > 0.0)
    {
        Color = gGridColorThick;
        Color.a *= Lod2a;
    }
    else if (Lod1a > 0.0)
    {
        Color = lerp(gGridColorThick, gGridColorThin, LODFade);
        Color.a *= Lod1a;
    }
    else
    {
        Color = gGridColorThin;
        Color.a *= (Lod0a * LODFade);
    }

    // 카메라와의 거리 기반 페이드아웃 ( TOFIX: 여기선 XY 평면을 기준으로함)
    float OpacityFalloff = (1.0 - saturate(length(input.WorldPos.xy - CameraPos.xy) / gGridSize));
    Color.a *= OpacityFalloff;

    output.Color = Color;
    output.Depth = 0.9999999;  // 월드 그리드는 강제로 먼 깊이값 부여 (Forced to be Occluded ALL THE TIME)
    return output;
}




/////////////////////////////////////////////
// Icon
struct PS_INPUT_ICON
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD;
};

Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

const static float2 QuadPos[6] =
{
    float2(-1, -1), float2(1, -1), float2(-1, 1), // 좌하단, 좌상단, 우하단
    float2(-1, 1), float2(1, -1), float2(1, 1) // 좌상단, 우상단, 우하단
};

const static float2 QuadTexCoord[6] =
{
    float2(0, 1), float2(1, 1), float2(0, 0), // 삼각형 1: 좌하단, 우하단, 좌상단
    float2(0, 0), float2(1, 1), float2(1, 0) // 삼각형 2: 좌상단, 우하단, 우상단
};


PS_INPUT_ICON iconVS(uint vertexID : SV_VertexID)
{
    PS_INPUT_ICON output;

    // 카메라를 향하는 billboard 좌표계 생성
    float3 forward = normalize(CameraPos - IconPosition);
    float3 up = float3(0, 0, 1);
    float3 right = normalize(cross(up, forward));
        up = cross(forward, right);

        // 쿼드 정점 계산 (아이콘 위치 기준으로 offset)
    float2 offset = QuadPos[vertexID];
    float3 worldPos = IconPosition + offset.x * right * IconScale + offset.y * up * IconScale;

        // 변환
    float4 viewPos = mul(float4(worldPos, 1.0), ViewMatrix);
        output.Position = mul(viewPos, ProjMatrix);

        output.TexCoord =
    QuadTexCoord[ vertexID];
        return
    output;
}


// 픽셀 셰이더
float4 iconPS(PS_INPUT_ICON input) : SV_Target
{
    float4 col = gTexture.Sample(gSampler, input.TexCoord);
    float threshold = 0.01; // 필요한 경우 임계값을 조정
    if (col.a < threshold)
        clip(-1); // 픽셀 버리기
    
    return col;
}


/////////////////////////////////////////////
// Arrow
PS_INPUT arrowVS(VS_INPUT input)
{
    PS_INPUT output;

    // 정규화된 방향
    float3 forward = normalize(ArrowDirection);

    // 기본 up 벡터와 forward가 나란할 때를 방지
    float3 up = abs(forward.y) > 0.99 ? float3(0, 0, 1) : float3(0, 1, 0);

    // 오른쪽 축
    float3 right = normalize(cross(up, forward));

    // 재정의된 up 벡터 (직교화)
    up = normalize(cross(forward, right));

    // 회전 행렬 구성 (Row-Major 기준)
    float3x3 rotationMatrix = float3x3(right, up, forward);

    input.position = input.position * ArrowScaleXYZ;
    input.position.z = input.position.z * ArrowScaleZ;
    // 로컬 → 회전 → 위치
    float3 worldPos = mul(input.position.xyz, rotationMatrix) + ArrowPosition;

    float4 pos = float4(worldPos, 1.0);
    pos = mul(pos, ViewMatrix);
    pos = mul(pos, ProjMatrix);

    output.position = pos;
    output.color = float4(0.7, 0.7, 0.7, 1.0f);

    return output;
}

float4 arrowPS(PS_INPUT input) : SV_Target
{
    return input.color;
}