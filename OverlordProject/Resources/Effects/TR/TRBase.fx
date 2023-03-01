//GLOBAL Variables
//***************
// The World View Projection Matrix
float4x4 gMatrixWVP : WORLDVIEWPROJECTION;
// The ViewInverse Matrix - the third row contains the camera position!
float4x4 gMatrixViewInverse : VIEWINVERSE;
// The World Matrix
float4x4 gMatrixWorld : WORLD;

// The View Matrix
float4x4 gMatrixView : VIEW;
// The Projection Matrix
float4x4 gMatrixProj : PROJECTION;

// PI constant
float gPi = 3.14159265f;

//SAMPLER STATES
//**************`
SamplerState gTextureSampler
{
    Filter = ANISOTROPIC;
 	AddressU = WRAP;
	AddressV = WRAP;
	AddressW = WRAP;
};

//LIGHT
//*****
float4 gLightColor = float4(1.0f, 1.0f, 1.0f, 1.0);
float3 gLightDirection : DIRECTION = float3(0.577f, -0.577f, 0.577f);
float gLightIntensity = 5.0f;

////ENVIRONMENT MAPPING
////*******************
//bool gUseTextureEnvironment = false;
//TextureCube gCubeEnvironment;
//float gRefractionIndex = 0.3f;


//ALBEDO
//*******
bool gUseAlbedoTexture;
Texture2D gAlbedoTexture;
float3 gAlbedoColor = float3(1, 0, 1);

//AO
//**
bool gUseAO = true;
float gAoStrength = 1.0f;
//METAL
//*****
bool gIsMetal = false;
bool gUseMetalnessMap = false;

//ROUGH-DISP-AO-METAL packed > RDAM
Texture2D gRDAM;
//ROUGHNESS
//*********
bool gUseRoughnessMap;
float gRoughness = 1.0f;

//NORMAL MAPPING
//**************
bool gUseNormalMap = false;
bool gFlipGreenChannel = false;
Texture2D gNormalMap;

//SHADOWING
//*********
float4x4 gViewProj_Light;
float gShadowMapBias = 0.001f;
float gAmbient = 0.2f;
Texture2D gShadowMap;

float2 texOffset(int u, int v)
{
	float w,h;
	gShadowMap.GetDimensions(w, h);
	return float2( u * 1.0f/w, v * 1.0f/h );
}

SamplerComparisonState cmpSampler
{
	// sampler state
	Filter = COMPARISON_MIN_MAG_MIP_LINEAR;
	AddressU = MIRROR;
	AddressV = MIRROR;

	// sampler comparison state
	ComparisonFunc = LESS_EQUAL;
};

float EvaluateShadowMap(float4 lpos)
{
	//back to homogenous space
	//lpos /= lpos.w;

	//return ambient color if not within view of the light
	//range x : [-1,1]
	//range y : [-1,1]
	//range z : [0,1]
	if (lpos.x < -1.0f || lpos.x > 1.0f 
	|| lpos.y < -1.0f || lpos.y > 1.0f 
	|| lpos.z < 0.0f || lpos.z > 1.0f)
        return 0.0f;

	//transform clip space coords [-1,1]  to texture space coords [0,1]
    lpos.x = lpos.x/2 + 0.5;
    lpos.y = lpos.y/-2 + 0.5;

	//apply shadow map bias
    lpos.z -= gShadowMapBias;

    //PCF sampling for shadow map
    float shadowFactor = 0;

	//perform PCF filtering on a 4 x 4 texel neighborhood
    for (float y = -1.5; y <= 1.5; y += 1.0)
    {
        for (float x = -1.5; x <= 1.5; x += 1.0)
        {
            shadowFactor += gShadowMap.SampleCmpLevelZero( cmpSampler, lpos.xy + texOffset(x,y), lpos.z).r;
        }
    }
    shadowFactor /= 16.0; //* something?
	return shadowFactor;
}

//STRUCTS
//***********

struct PS_Input
{
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoord : TEXCOORD0;
    float4 Position : SV_POSITION;
    float4 lPos : TEXCOORD1;
};

//MAIN PIXEL SHADER
//*****************
float4 MainPS(PS_Input input) : SV_TARGET 
{
	//DIFFUSE
    float4 finalColor = (gAlbedoTexture.Sample(gTextureSampler, input.TexCoord)) * gUseAlbedoTexture + !gUseAlbedoTexture * float4(gAlbedoColor,0);
    finalColor.rgb /= gPi;
    if (finalColor.a == 0) discard;

    float shadowValue = EvaluateShadowMap(input.lPos);
	//NORMAL
    float3 newNormal = normalize(input.Normal);
    float3 newTangent = normalize(input.Tangent);
    float3 binormal = normalize(cross(newTangent, newNormal));
    binormal = (((gFlipGreenChannel)*2)-1) * binormal;
    float3x3 localAxis = float3x3(newTangent, binormal, newNormal);
    newNormal = gNormalMap.Sample(gTextureSampler, input.TexCoord).xyz;
    newNormal = 2.0f * newNormal - 1.0f; //remap to [-1,1]
    newNormal = (mul(newNormal, localAxis)) * gUseNormalMap + !gUseNormalMap * input.Normal;

	//Ambient occlusion
    float aoValue = 1.0f;
    if (gUseAO)
    {
        aoValue = pow(gRDAM.Sample(gTextureSampler, input.TexCoord).b, gAoStrength);
    }
	//FINAL COLOR CALCULATION
    float4 lightContribution = dot(-newNormal, normalize(gLightDirection)) * gLightColor;
    lightContribution = (saturate(shadowValue + gAmbient) * lightContribution) * aoValue;
    finalColor = finalColor * lightContribution * gLightIntensity;
    return finalColor;
}

