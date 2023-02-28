#include "TRBase.fx"

//rasterizerstates
RasterizerState gRS_Filled 
{ 
	CullMode = FRONT; 
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

//MAIN VERTEX SHADER
//******************
PS_Input MainVS(VS_Input input) {
	
    PS_Input output = (PS_Input) 0;

    output.Position = mul(float4(input.Position, 1.0), gMatrixWVP);
    output.Normal = mul(input.Normal, (float3x3) gMatrixWorld);
    output.Tangent = mul(input.Tangent, (float3x3) gMatrixWorld);
    output.TexCoord = input.TexCoord;
	output.lPos = mul(float4(input.Position, 1), mul(gMatrixWorld, gViewProj_Light));

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

float4 ShadowMapVS(VS_Input vertex) : SV_POSITION
{
    //construct WVP
    return mul(float4(vertex.Position, 1), gMatrixWVP);
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
