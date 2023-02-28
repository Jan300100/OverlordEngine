#include "PbrBase_Shadow.fx"

//rasterizerstates
RasterizerState gRS_Filled 
{ 
	CullMode = NONE; 
};

RasterizerState gRS_WireFrame
{
    CullMode = NONE;
    FillMode = WIREFRAME;
};


struct VS_Input
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoord : TEXCOORD0;
};

struct VS_Instance
{
    //worldMatrix columns per instance
    float4 m0 : INSTANCE4F0;
    float4 m1 : INSTANCE4F1;
    float4 m2 : INSTANCE4F2;
    float4 m3 : INSTANCE4F3;
};

//MAIN VERTEX SHADER
//******************
PS_Input MainVS(VS_Input input, VS_Instance instance) {
	
    PS_Input output = (PS_Input) 0;

    float4x4 world = float4x4(instance.m0, instance.m1, instance.m2, instance.m3);
    float4x4 wvp = mul(world, mul(gMatrixView, gMatrixProj));
    
    output.Position = mul(float4(input.Position, 1.0), wvp);
    output.WorldPosition = mul(float4(input.Position, 1.0), world);
    output.Normal = mul(input.Normal, (float3x3) world);
    output.Tangent = mul(input.Tangent, (float3x3) world);
    output.TexCoord = input.TexCoord;
    output.lPos = mul(float4(input.Position, 1), mul(world, gViewProj_Light));

    return output;
}

// Default Technique
technique11 tDefault {
	pass p0 {
        //SetBlendState(NULL, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        //SetDepthStencilState(EnableDepth, 0);
		SetRasterizerState(gRS_Filled);
		SetVertexShader(CompileShader(vs_5_0, MainVS()));
        SetHullShader(NULL);
        SetDomainShader(NULL);
		SetGeometryShader( NULL );
		SetPixelShader(CompileShader(ps_5_0, MainPS()));
    }
}

technique11 tWireframe
{
    pass p0
    {
        //SetBlendState(NULL, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        //SetDepthStencilState(EnableDepth, 0);
        SetRasterizerState(gRS_WireFrame);
        SetVertexShader(CompileShader(vs_5_0, MainVS()));
        SetHullShader(NULL);
        SetDomainShader(NULL);
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, MainPS()));
    }
}

float4 ShadowMapVS(VS_Input vertex, VS_Instance instance) : SV_POSITION
{
    //construct WVP
    float4x4 world = float4x4(instance.m0, instance.m1, instance.m2, instance.m3);
    return mul(float4(vertex.Position, 1), mul(world, gViewProj_Light));
}

void ShadowMapPS_VOID(float4 position : SV_POSITION) {}

technique11 tGenerateShadowsTechnique
{
    pass P0
    {
        SetBlendState(NULL, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader(CompileShader(vs_5_0, ShadowMapVS()));
        SetHullShader(NULL);
        SetDomainShader(NULL);
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, ShadowMapPS_VOID()));
    }
}