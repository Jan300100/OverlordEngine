#include "PbrBase_Shadow.fx"

float2 gWindDirection = float2(0,1); //moves in x,z plane
float gNoiseUvScale = 0.01f;
float gTimePassed = 0.0f;
float gWindForce = 1.0f;
float gInfluence = 1.0f;
float gDistanceInfluence = 0.1f;

Texture2D gNoiseMap;

BlendState gBS_EnableBlending
{
    BlendEnable[0] = TRUE;
    SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
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
	
	PS_Input output = (PS_Input)0;

    float4 wPos = mul(float4(input.Position, 1.0), gMatrixWorld);
    float randomStrength = gNoiseMap.SampleLevel(gTextureSampler, float2(wPos.x / gNoiseUvScale,wPos.z/gNoiseUvScale) + gWindDirection * gWindForce * gTimePassed,0);
    float distance = length(wPos); //distance from the base point of the instance
    float2 windDisplacement = (gWindDirection * distance * gDistanceInfluence //the higher above the ground, the more the wind affects the vertex
                * randomStrength * gWindForce) * gInfluence;

     float4 displacedPos = float4(input.Position,1);
    displacedPos.x += windDisplacement.x;
    displacedPos.z += windDisplacement.y;

	output.Position = mul(displacedPos, gMatrixWVP);
    output.WorldPosition = mul(displacedPos, gMatrixWorld);
    output.Normal = mul(input.Normal, (float3x3) gMatrixWorld);
    output.Tangent = mul(input.Tangent, (float3x3)gMatrixWorld);
	output.TexCoord = input.TexCoord;
    output.lPos = mul(displacedPos,  mul(gMatrixWorld, gViewProj_Light));
    
    return output;
}

// Default Technique
technique11 tDefault {
	pass p0 {
        SetBlendState(gBS_EnableBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, MainVS()));
        SetHullShader(NULL);
        SetDomainShader(NULL);
		SetGeometryShader( NULL );
		SetPixelShader(CompileShader(ps_5_0, MainPS()));
    }
}