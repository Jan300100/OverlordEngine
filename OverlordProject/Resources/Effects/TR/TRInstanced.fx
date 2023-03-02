#include "TRBase.fx"

//rasterizerstates
RasterizerState gRS_Filled
{
    CullMode = BACK;
};

struct VS_Vertex
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoord : TEXCOORD0;
};

struct VS_Instance
{
    //worldmatrix per instance, 4 float4 to be compatible with the buildinputlayout
    float4 M0: INSTANCE4F0;
    float4 M1: INSTANCE4F1;
    float4 M2: INSTANCE4F2;
    float4 M3: INSTANCE4F3;
};

//MAIN VERTEX SHADER
//******************
PS_Input MainVS(VS_Vertex vertex, VS_Instance instance) {
	
    PS_Input output = (PS_Input)0;

    //construct WVP
    float4x4 world = float4x4(instance.M0, instance.M1, instance.M2, instance.M3);

    output.WorldPosition = mul(float4(vertex.Position, 1), world).xyz;
    output.Position = mul(float4(output.WorldPosition, 1), mul(gMatrixView, gMatrixProj));
    output.Normal = mul(vertex.Normal, (float3x3) world);
    output.Tangent = mul(vertex.Tangent, (float3x3) world);
    output.TexCoord = vertex.TexCoord;
    output.lPos = mul(float4(vertex.Position, 1), mul(world, gViewProj_Light));

    return output;
}

// Default Technique
technique11 tDefault {
	pass p0 {
        SetBlendState(NULL, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        //SetDepthStencilState(EnableDepth, 0);
		SetRasterizerState(gRS_Filled);
		SetVertexShader(CompileShader(vs_5_0, MainVS()));
        SetHullShader(NULL);
        SetDomainShader(NULL);
		SetGeometryShader( NULL );
		SetPixelShader(CompileShader(ps_5_0, MainPS()));
    }
}

float4 ShadowMapVS(VS_Vertex vertex, VS_Instance instance) : SV_POSITION
{

    //construct WVP
    float4x4 world = float4x4(instance.M0, instance.M1, instance.M2, instance.M3);
    return mul(float4(vertex.Position, 1), mul(world, gViewProj_Light));
}

void ShadowMapPS_VOID(float4 position : SV_POSITION)
{
    
}

technique11 tGenerateShadowsTechnique
{
    pass p0
    {
        SetRasterizerState(gRS_Filled);
        SetVertexShader(CompileShader(vs_5_0, ShadowMapVS()));
        SetHullShader(NULL);
        SetDomainShader(NULL);
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, ShadowMapPS_VOID()));
    }
}
