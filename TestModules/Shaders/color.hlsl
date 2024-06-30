
cbuffer cbPerObject : register(b0)
{
    float4x4 gWorldMat;
};

cbuffer cbPerPass : register(b1)
{
    float4x4 gViewMat;
    float4x4 gInvViewMat;
    float4x4 gProjMat;
    float4x4 gInvProjMat;
    float4x4 gViewProjMat;
    float4x4 gInvViewProjMat;
    float3 gEyePosW;
    float pad1;
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
};

struct VertexIn
{
    float3 PosL : POSITION;
    float4 Color : COLOR;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float4 Color : COLOR;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    
    // Transform to homogenous clip space
    float4 posW = mul(float4(vin.PosL, 1.0f), gWorldMat);
    vout.PosH = mul(posW, gViewProjMat);
    
    // pass vertex color into the pixel shader
    vout.Color = vin.Color;
    
    return vout;
}

float4 PS(VertexOut pin)   : SV_Target
{
    return pin.Color;
}