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
float3 gLightDirection : DIRECTION = float3(0.577f, -0.577f, 0.577f);
float gLightIntensity = 5.0f;

//ENVIRONMENT MAPPING
//*******************
bool gUseTextureEnvironment = false;
TextureCube gCubeEnvironment;
float gRefractionIndex = 0.3f;


//ALBEDO
//*******
bool gUseAlbedoTexture;
Texture2D gAlbedoTexture;
float3 gAlbedoColor = float3(1, 0, 1);

//AO
//**
bool gUseAO = false;
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

//STRUCTS
//***********

struct PS_Input
{
    float4 WorldPosition : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoord : TEXCOORD0;
    float4 Position : SV_POSITION;
};

//ENVIRONMENT MAPPING FUNCTION
float3 CalculateEnvironment(float3 viewDirection, float3 normal)
{
    float3 environmentColor = float3(0, 0, 0);
	
    if (gUseTextureEnvironment)
    {
		//reflected
        float3 reflected = reflect(viewDirection, normal);
        reflected = gCubeEnvironment.Sample(gTextureSampler, reflected).rgb;
        reflected *= 1;
		
		//refracted
        float3 refracted = refract(viewDirection, normal, gRefractionIndex);
        refracted = gCubeEnvironment.Sample(gTextureSampler, refracted).rgb;
        refracted *= 0;
		
        return reflected + refracted;
    }
		
    return environmentColor;
	
}

//MAIN PIXEL SHADER
//*****************
float4 MainPS(PS_Input input) : SV_TARGET 
{
    float3 viewDirection = normalize(input.WorldPosition.xyz - gMatrixViewInverse[3].xyz);
    float3 lDir = normalize(gLightDirection);
	
	//NORMAL
    float3 newNormal = normalize(input.Normal);
    if (gUseNormalMap)
    {
        float3 newTangent = normalize(input.Tangent);
        float3 binormal = normalize(cross(newTangent, newNormal));
        if (gFlipGreenChannel)
        {
            binormal = -binormal;
        }
        float3x3 localAxis = float3x3(newTangent, binormal, newNormal);
		
        if (gUseNormalMap)
            newNormal = gNormalMap.Sample(gTextureSampler, input.TexCoord).xyz;
        newNormal = 2.0f * newNormal - 1.0f; //remap to [-1,1]
 
        newNormal = normalize(mul(newNormal, localAxis));
    }
	//DIFFUSE
    float4 diffColor = float4(gAlbedoColor,1);
    if (gUseAlbedoTexture)
    {
        diffColor = (gAlbedoTexture.Sample(gTextureSampler, input.TexCoord));
    }
    
    diffColor.rgb /= gPi;
	//SPECULAR
	
	//Determine F0 value
    float3 f0 = float3(0.04, 0.04, 0.04);
    float metal = gIsMetal;
    if (gIsMetal)
    {
        if (gUseMetalnessMap)
        {
			//sample from metalnessmap
            metal = gRDAM.Sample(gTextureSampler, input.TexCoord).a;
            if (metal == 1.0f)
            {
                if (gUseAlbedoTexture)
                {               
				//sample from albedomap if metal, else stay 0.04 (non metal value);
                f0 = gAlbedoTexture.Sample(gTextureSampler, input.TexCoord).rgb;
                }
                else
                {
                    f0 = gAlbedoColor;
                }
            }
        }
        else
        {
            if (gUseAlbedoTexture)
            {
				//sample from albedomap if metal, else stay 0.04 (non metal value);
                f0 = gAlbedoTexture.Sample(gTextureSampler, input.TexCoord).rgb;
            }
            else
            {
                f0 = gAlbedoColor;
            }
        }
    }
	//calc halfvector between viewDir and lightDir
    float3 hv = (viewDirection + lDir) / length(viewDirection + lDir);
	//calculate Fresnel (F) (Schlick)
    float3 fresnel = f0 + ((float3) 1 - f0) * pow(1 - dot(hv, viewDirection), 5); //5 magic number
	
	//Calculate Normal Distribution (D)
    float roughness = gRoughness;
    if (gUseRoughnessMap)
    {
        roughness = gRDAM.Sample(gTextureSampler, input.TexCoord).r;
    }
	//trowbridge_reitz_ggx 
    float rPow2 = pow(roughness, 2);
    float rPow4 = pow(rPow2, 2);
    float distribution = saturate(rPow4 / (gPi * pow(pow(dot(-newNormal, hv), 2) * (rPow4 - 1) + 1, 2)));
	
	
	//Calculate Geometry //Smith_schlick
    float geometry = dot(newNormal, viewDirection) * dot(newNormal, lDir);
	
	//cook-torrance
    float3 specular = (fresnel * geometry * distribution) / (4 * dot(-viewDirection, newNormal) * dot(-lDir, newNormal));
	
	//ENVIRONMENT MAPPING
    float3 environmentColor = CalculateEnvironment(viewDirection, newNormal);
  
    //this is probably wrong
    specular += (environmentColor * fresnel * (1-roughness)) / gPi;
	
	//Ambient occlusion
    float aoValue = 1;
    if (gUseAO)
    {
        aoValue = pow(gRDAM.Sample(gTextureSampler, input.TexCoord).b, gAoStrength);
    }
	
    float lambertDot = saturate(dot(-newNormal, lDir));
	
	//FINAL COLOR CALCULATION
    float3 finalColor = specular;
    if (metal == 0.0f) //magic metal number
    {
		//not a metal
        float diffuseR = 1 - fresnel.r;
        finalColor = ((diffColor.rgb * diffuseR) + specular);
    }
    finalColor *= aoValue * lambertDot * gLightIntensity;
	//OPACITY
    float opacity = diffColor.a;
	
    return float4(finalColor, opacity);
}