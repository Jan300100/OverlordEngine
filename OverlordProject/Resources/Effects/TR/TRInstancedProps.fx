#include "TRBase.fx"

//used to calculate height in the world
Texture2D gNoiseTexture;
float gNoiseHeight = 10.0f;
float gNoiseUVScale = 0.01f;

//wind
float2 gWindDirection = float2(0,1); //moves in x,z plane
float gTimePassed = 0.0f;
float gWindForce = 1.0f;
float gInfluence = 1.0f;
float gDistanceInfluence = 5.0f;



BlendState gBS_EnableBlending
{
    BlendEnable[0] = TRUE;
    SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
    BlendOp = ADD;
    SrcBlendAlpha = ONE;
    DestBlendAlpha = ZERO;
    BlendOpAlpha = ADD;
    RenderTargetWriteMask[0] = 0x0f;
};

DepthStencilState EnableDepth
{
    DepthEnable = TRUE;
    DepthWriteMask = ALL;
};

//rasterizerstates
RasterizerState gRS_doubleSided 
{ 
	CullMode = NONE; 
};
RasterizerState gRS_Cull
{
    CullMode = FRONT;
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
    //worldMatrix columns per instance
    float4 m0: INSTANCE4F0;
    float4 m1: INSTANCE4F1;
    float4 m2: INSTANCE4F2;
    float4 m3: INSTANCE4F3;
};

//Foliage VERTEX SHADER
PS_Input MainVS(VS_Vertex vertex, VS_Instance instance)
{
    PS_Input output = (PS_Input)0;

    //set y position to be in relation to the ground
    instance.m3.y += ((gNoiseTexture.SampleLevel(gTextureSampler, float2(instance.m3.x / gNoiseUVScale, instance.m3.z / gNoiseUVScale), 0).r) - 0.5f) * gNoiseHeight;
    //construct WVP
    float4x4 world = float4x4(instance.m0, instance.m1, instance.m2, instance.m3);
    //windEffect
    float4 wPos = mul(float4(vertex.Position, 1.0f), world);
    float randomStrength = gNoiseTexture.SampleLevel(gTextureSampler, float2(instance.m3.x / gNoiseUVScale, instance.m3.z / gNoiseUVScale) + gWindDirection * gWindForce * gTimePassed,0).r - 0.5f;
    float distance = wPos.y - instance.m3.y;
    distance = (pow(distance * gDistanceInfluence, 2) * randomStrength) * gInfluence;
    wPos.xz += distance * gWindDirection.xy;
    //
    output.Position = mul(wPos, mul(gMatrixView, gMatrixProj));
    output.Normal = mul(vertex.Normal, (float3x3) world);
    output.Tangent = mul(vertex.Tangent, (float3x3)world);
	output.TexCoord = vertex.TexCoord;
    output.lPos = mul(wPos, gViewProj_Light); 

    return output;
}


//MAIN PIXEL SHADER
//*****************
float4 DoubleSidedPS(PS_Input input) : SV_TARGET 
{
	//DIFFUSE
    float4 finalColor = (gAlbedoTexture.Sample(gTextureSampler, input.TexCoord)) * gUseAlbedoTexture + !gUseAlbedoTexture * float4(gAlbedoColor,0);
    finalColor.rgb /= gPi;
    if (finalColor.a == 0) discard;

	float shadowValue = EvaluateShadowMap(input.lPos);
	//NORMAL

    float3 newNormal = normalize(input.Normal);
    float3 newTangent = normalize(input.Tangent);

    float3 viewDirection = normalize(input.Position.xyz - gMatrixViewInverse[3].xyz);

    float3 binormal = normalize(cross(newTangent, newNormal));
    binormal = (((gFlipGreenChannel)*2)-1) * binormal;
    float3x3 localAxis = float3x3(newTangent, binormal, newNormal);
    newNormal = gNormalMap.Sample(gTextureSampler, input.TexCoord).xyz;
    newNormal = 2.0f * newNormal - 1.0f; //remap to [-1,1]
    newNormal = (mul(newNormal, localAxis)) * gUseNormalMap + !gUseNormalMap * input.Normal;

    //if (dot(viewDirection, newNormal) < 0) //normal away from camera
    //{
    //    newNormal = -newNormal;
    //}

	//Ambient occlusion
    float aoValue = pow(gRDAM.Sample(gTextureSampler, input.TexCoord).b, gAoStrength) * gUseAO + !gUseAO * 1.0f;
    //FINAL COLOR CALCULATION
    float4 lightContribution = ((1.0 - gAmbient) * dot(-newNormal, normalize(gLightDirection)) * shadowValue + gAmbient) * gLightColor;
    finalColor = finalColor * lightContribution * gLightIntensity;
    return finalColor;
}


// Default Technique
technique11 tDefault {
	pass p0 {
        SetBlendState(NULL, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetRasterizerState(gRS_Cull);
		SetVertexShader(CompileShader(vs_5_0, MainVS()));
        SetHullShader(NULL);
        SetDomainShader(NULL);
		SetGeometryShader( NULL );
		SetPixelShader(CompileShader(ps_5_0, MainPS()));
    }
}

// foliage Technique
technique11 tFoliage {
	pass p0 {
        SetBlendState(NULL, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetRasterizerState(gRS_doubleSided);
		SetVertexShader(CompileShader(vs_5_0, MainVS()));
        SetHullShader(NULL);
        SetDomainShader(NULL);
		SetGeometryShader( NULL );
		SetPixelShader(CompileShader(ps_5_0, DoubleSidedPS()));
    }
}


float4 ShadowMapVS(VS_Vertex vertex, VS_Instance instance) : SV_POSITION
{
    
    PS_Input output = (PS_Input) 0;
   
    //set y position to be in relation to the ground
    instance.m3.y += ((gNoiseTexture.SampleLevel(gTextureSampler, float2(instance.m3.x / gNoiseUVScale, instance.m3.z / gNoiseUVScale), 0).r) - 0.5f) * gNoiseHeight;
    //construct WVP
    float4x4 world = float4x4(instance.m0, instance.m1, instance.m2, instance.m3);
    //windEffect
    float4 wPos = mul(float4(vertex.Position, 1.0f), world);
    float randomStrength = gNoiseTexture.SampleLevel(gTextureSampler, float2(instance.m3.x / gNoiseUVScale, instance.m3.z / gNoiseUVScale) + gWindDirection * gWindForce * gTimePassed, 0).r - 0.5f;
    float distance = wPos.y - instance.m3.y;
    distance = (pow(distance * gDistanceInfluence, 2) * randomStrength) * gInfluence;
    wPos.xz += distance * gWindDirection.xy;
    //
    
    return mul(wPos, gViewProj_Light);
}


void ShadowMapPS_VOID(float4 position : SV_POSITION)
{
    
}


technique11 tGenerateShadowsStatic
{
    pass P0
    {
        SetBlendState(NULL, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetDepthStencilState(EnableDepth, 0);
        SetRasterizerState(gRS_Cull);
        SetVertexShader(CompileShader(vs_5_0, ShadowMapVS()));
        SetHullShader(NULL);
        SetDomainShader(NULL);
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, ShadowMapPS_VOID()));
    }
}

technique11 tGenerateShadowsFoliage
{
    pass P0
    {

        SetBlendState(gBS_EnableBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetDepthStencilState(EnableDepth, 0);
        SetRasterizerState(gRS_doubleSided);
        SetVertexShader(CompileShader(vs_5_0, ShadowMapVS()));
        SetHullShader(NULL);
        SetDomainShader(NULL);
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, ShadowMapPS_VOID()));
    }
}