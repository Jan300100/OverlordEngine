#include "TRBase.fx"

float4x4 gBones[70];


RasterizerState gRS_Solid
{
    FillMode = SOLID;
	CullMode = BACK;
};

struct VS_Input
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoord : TEXCOORD0;
    float4 blendIndices : BLENDINDICES;
    float4 blendWeights : BLENDWEIGHTS;
};

//MAIN VERTEX SHADER
//******************
PS_Input MainVS(VS_Input input) {
	
    
     //Skinning
    float4 originalPosition = float4(input.Position, 1);
    float4 transformedPosition = 0;
    float3 transformedNormal = 0;
    float3 transformedTangent = 0;
    for (int i = 0; i < 4; i++)
    {
        int boneIndex = input.blendIndices[i];
        if (boneIndex >= 0) //vertex is attached to this bone
        {
			//update position and normal and tangent
            transformedPosition += mul(input.blendWeights[i], mul(float4(input.Position, 1), gBones[boneIndex]));
			//only rotate normal
            transformedNormal += mul(input.blendWeights[i], mul(input.Normal, (float3x3) gBones[boneIndex]));
            //only rotate tangent
            transformedTangent += mul(input.blendWeights[i], mul(input.Tangent, (float3x3) gBones[boneIndex]));
        }
    }
    transformedPosition.w = 1;
    
    
    PS_Input output = (PS_Input) 0;
    
    output.Position = mul(transformedPosition, gMatrixWVP);
    output.WorldPosition = mul(transformedPosition, gMatrixWorld).xyz;
    output.Normal = mul(transformedNormal, (float3x3) gMatrixWorld);
    output.Tangent = mul(transformedTangent, (float3x3) gMatrixWorld);
    output.TexCoord = input.TexCoord;
    output.lPos = mul(transformedPosition, mul(gMatrixWorld, gViewProj_Light));

    return output;
}

// Default Technique
technique11 tDefault {
	pass p0 {
        SetBlendState(NULL, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        //SetDepthStencilState(EnableDepth, 0);
        SetRasterizerState(gRS_Solid);
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
        SetRasterizerState(gRS_Solid);
        SetVertexShader(CompileShader(vs_5_0, ShadowMapVS()));
        SetHullShader(NULL);
        SetDomainShader(NULL);
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, ShadowMapPS_VOID()));
    }
}
