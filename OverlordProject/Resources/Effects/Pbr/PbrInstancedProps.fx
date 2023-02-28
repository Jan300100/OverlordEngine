#include "PbrBase_Shadow.fx"

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
    SrcBlendAlpha = ZERO;
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


//SOLID VERTEX SHADER
//******************
PS_Input MainVS(VS_Vertex vertex, VS_Instance instance) {
	
    PS_Input output = (PS_Input)0;

    //set y position to be in relation to the ground
    instance.m3.y += ((gNoiseTexture.SampleLevel(gTextureSampler, float2(instance.m3.x / gNoiseUVScale, instance.m3.z / gNoiseUVScale), 0).r) - 0.5f) * gNoiseHeight;
    //construct WVP
    float4x4 world = float4x4(instance.m0, instance.m1, instance.m2, instance.m3);
    float4x4 wvp = mul(world, mul(gMatrixView, gMatrixProj));

    output.Position = mul(float4(vertex.Position,1), wvp);
    output.WorldPosition = mul(float4(vertex.Position,1), world);
    output.Normal = mul(vertex.Normal, (float3x3) world);
    output.Tangent = mul(vertex.Tangent, (float3x3) world);
    output.TexCoord = vertex.TexCoord;
    output.lPos = mul(float4(vertex.Position,1), mul(world, gViewProj_Light));

    return output;
}


//Foliage VERTEX SHADER
PS_Input FoliageVS(VS_Vertex vertex, VS_Instance instance)
{
    PS_Input output = (PS_Input)0;

    //set y position to be in relation to the ground
    instance.m3.y += ((gNoiseTexture.SampleLevel(gTextureSampler, float2(instance.m3.x / gNoiseUVScale, instance.m3.z / gNoiseUVScale), 0).r) - 0.5f) * gNoiseHeight;
    //construct WVP
    float4x4 world = float4x4(instance.m0, instance.m1, instance.m2, instance.m3);
    //float4x4 wvp = mul(world, mul(gMatrixView, gMatrixProj));

    //windEffect
   
    float4 wPos = mul(float4(vertex.Position, 1.0f), world);
    
    float randomStrength = gNoiseTexture.SampleLevel(gTextureSampler, float2(instance.m3.x / gNoiseUVScale, instance.m3.z / gNoiseUVScale) + gWindDirection * gWindForce * gTimePassed,0).r - 0.5f;
    float distance = length(wPos - instance.m3); //distance from the base point of the instance
    float2 windDisplacement = (gWindDirection * distance * gDistanceInfluence //the further away from the center of the instanceposition, the more effect the wind has.
                * randomStrength * gWindForce) * gInfluence;

    float4 displacedPos = wPos;
    displacedPos.x += windDisplacement.x;
    displacedPos.z += windDisplacement.y;

    //
    output.Position = mul(displacedPos, mul(gMatrixView, gMatrixProj));
    //output.WorldPosition = mul(displacedPos, world);
    output.WorldPosition = wPos;
    
    output.Normal = mul(vertex.Normal, (float3x3) world);
    output.Tangent = mul(vertex.Tangent, (float3x3)world);
	output.TexCoord = vertex.TexCoord;
    
    //output.lPos = mul(displacedPos, mul(world, gViewProj_Light)); //should become world * gLightViewProj;
    output.lPos = mul(wPos, gViewProj_Light); //should become world * gLightViewProj;

    return output;
}

//MAIN PIXEL SHADER
//*****************
float4 FoliagePS(PS_Input input) : SV_TARGET
{
    float shadowValue = EvaluateShadowMap(input.lPos);

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
 
        newNormal = (mul(newNormal, localAxis));
    }
	//DIFFUSE
    float4 diffColor = float4(gAlbedoColor, 1);
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
    specular += (environmentColor * fresnel * (1 - roughness)) / gPi;
	
	//Ambient occlusion
    float aoValue = 1;
    if (gUseAO)
    {
        aoValue = pow(gRDAM.Sample(gTextureSampler, input.TexCoord).b, gAoStrength);
    }
	
    float lambertDot = abs(dot(-newNormal, lDir));
	
	//FINAL COLOR CALCULATION
    float3 finalColor = specular;
    if (metal == 0.0f) //magic metal number
    {
		//not a metal
        float diffuseR = 1 - fresnel.r;
        finalColor = ((diffColor.rgb * diffuseR) + specular);
    }
    finalColor *= shadowValue * aoValue * lambertDot * gLightIntensity;
	//OPACITY
    float opacity = diffColor.a;
	
    return float4(finalColor, opacity);
}



// Default Technique
technique11 tDefault {
	pass p0 {
        SetBlendState(NULL, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
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
        SetBlendState(gBS_EnableBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, FoliageVS()));
        SetHullShader(NULL);
        SetDomainShader(NULL);
		SetGeometryShader( NULL );
		SetPixelShader(CompileShader(ps_5_0, FoliagePS()));
    }
}


float4 ShadowMapStaticVS(VS_Vertex vertex, VS_Instance instance) : SV_POSITION
{
    //set y position to be in relation to the ground
    instance.m3.y += ((gNoiseTexture.SampleLevel(gTextureSampler, float2(instance.m3.x / gNoiseUVScale, instance.m3.z / gNoiseUVScale), 0).r) - 0.5f) * gNoiseHeight;
    //construct WVP
    float4x4 world = float4x4(instance.m0, instance.m1, instance.m2, instance.m3);    
    return mul(float4(vertex.Position, 1), mul(world, gViewProj_Light));
}

float4 ShadowMapFoliageVS(VS_Vertex vertex, VS_Instance instance) : SV_POSITION
{
    
    //set y position to be in relation to the ground
    instance.m3.y += ((gNoiseTexture.SampleLevel(gTextureSampler, float2(instance.m3.x / gNoiseUVScale, instance.m3.z / gNoiseUVScale), 0).r) - 0.5f) * gNoiseHeight;
    //construct WVP
    float4x4 world = float4x4(instance.m0, instance.m1, instance.m2, instance.m3);
    
    float4 wPos = mul(float4(vertex.Position, 1.0f), world);
    float randomStrength = gNoiseTexture.SampleLevel(gTextureSampler, float2(instance.m3.x / gNoiseUVScale, instance.m3.z / gNoiseUVScale) + gWindDirection * gWindForce * gTimePassed, 0).r - 0.5f;
    float distance = length(wPos - instance.m3); //distance from the base point of the instance
    float2 windDisplacement = (gWindDirection * distance * gDistanceInfluence //the further away from the center of the instanceposition, the more effect the wind has.
                * randomStrength * gWindForce) * gInfluence;

    wPos.x += windDisplacement.x;
    wPos.z += windDisplacement.y;
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
        SetRasterizerState(gRS_doubleSided);
        SetVertexShader(CompileShader(vs_5_0, ShadowMapStaticVS()));
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
        SetVertexShader(CompileShader(vs_5_0, ShadowMapFoliageVS()));
        SetHullShader(NULL);
        SetDomainShader(NULL);
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, ShadowMapPS_VOID()));
    }
}