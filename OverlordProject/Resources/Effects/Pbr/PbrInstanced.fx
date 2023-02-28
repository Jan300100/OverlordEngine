#include "PbrBase.fx"



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
    float4x4 wvp = mul(world, mul(gMatrixView, gMatrixProj));

    //wvp = gMatrixWVP;
    //world = gMatrixWorld;

    output.Position = mul(float4(vertex.Position,1), wvp);
    output.WorldPosition = mul(float4(vertex.Position,1), world);
    output.Normal = mul(vertex.Normal, (float3x3) world);
    output.Tangent = mul(vertex.Tangent, (float3x3) world);
    output.TexCoord = vertex.TexCoord;

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