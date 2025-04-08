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
};
const static float2 QuadPos[6] =
{
    float2(-1, -1), float2(-1, 1), float2(1, -1), // 좌하단, 좌상단, 우하단
    float2(-1, 1), float2(1, 1), float2(1, -1) // 좌상단, 우상단, 우하단
};

PS_INPUT_GRID gridVS(uint vertexID : SV_VertexID)
{
    PS_INPUT_GRID output;
    
    output.Position = float4(QuadPos[vertexID], 0.0, 1.0);
    
    output.NearPoint = float4(QuadPos[vertexID], 0.0, 1.0);
    output.NearPoint = mul(output.NearPoint, InverseViewProj);
    output.NearPoint = output.NearPoint / output.NearPoint.w; // COLOR로 넘겨줘서 해줘야함
    
    output.FarPoint = float4(QuadPos[vertexID], 1.0, 1.0);
    output.FarPoint = mul(output.FarPoint, InverseViewProj);
    output.FarPoint = output.FarPoint / output.FarPoint.w; // COLOR로 넘겨줘서 해줘야함
    
    return output;
}

// 그리드 함수
float4 gridCalc(float3 fragPos3D, float scale)
{
    float2 coord = fragPos3D.xy * scale; // Use the scale variable to set the distance between the lines
    float2 derivative = float2(ddx(coord.x), ddy(coord.y)); // HLSL의 fwidth는 ddx, ddy로 대체
    float2 grid = abs(frac(coord - 0.5) - 0.5) / derivative;
    float gridX = grid.x;
    float gridY = grid.y;
    float LinePos = min(gridX, gridY);
    float minimumz = min(derivative.y, 1);
    float minimumx = min(derivative.x, 1);
    float4 color = float4(0.2, 0.2, 0.2, 1.0 - min(LinePos, 1.0));
    
    // Z axis
    if (fragPos3D.x > -0.1 * minimumx && fragPos3D.x < 0.1 * minimumx)
        color.z = 1.0;
    
    // X axis
    if (fragPos3D.z > -0.1 * minimumz && fragPos3D.z < 0.1 * minimumz)
        color.x = 1.0;
    
    return color;
}

// 픽셀 셰이더
float4 gridPS(PS_INPUT_GRID input) : SV_Target
{
    float t = -input.NearPoint.z / (input.FarPoint.z - input.NearPoint.z);
    //return float4(t * 100, t * 100, t * 100, 1);
    float3 fragPos3D = input.NearPoint + t * (input.FarPoint - input.NearPoint);
   
    if (t < 0)
    {
        return float4(1, 0, 0, 1);

    }
    if (t > 1)
    {
        return float4(0,1,0, 1);
    }
    return float4(fragPos3D / 1000, 1);
    return float4(1, 1, 1, 1);
    
        //return gridCalc(fragPos3D, 10.0f) * float(t > 0.0f); // t > 0이면 출력
}