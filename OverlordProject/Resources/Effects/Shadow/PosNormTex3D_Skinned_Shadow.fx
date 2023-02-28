float4x4 gWorld : WORLD;
float4x4 gWorldViewProj : WORLDVIEWPROJECTION; 
float4x4 gWorldViewProj_Light;
float3 gLightDirection = float3(-0.577f, -0.577f, 0.577f);
float gShadowMapBias = 0.03f;
float gAmbient = 0.0f;
float4x4 gBones[70];

Texture2D gDiffuseMap;
Texture2D gShadowMap;

SamplerComparisonState cmpSampler
{
	// sampler state
	Filter = COMPARISON_MIN_MAG_MIP_LINEAR;
	AddressU = MIRROR;
	AddressV = MIRROR;

	// sampler comparison state
	ComparisonFunc = LESS_EQUAL;
};

SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;// or Mirror or Clamp or Border
    AddressV = Wrap;// or Mirror or Clamp or Border
};

struct VS_INPUT
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
	float4 BoneIndices : BLENDINDICES;
	float4 BoneWeights : BLENDWEIGHTS;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
	float4 lPos : TEXCOORD1;
};

DepthStencilState EnableDepth
{
	DepthEnable = TRUE;
	DepthWriteMask = ALL;
};

RasterizerState NoCulling
{
	CullMode = NONE;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	float4 originalPosition = float4(input.pos, 1);
	float4 transformedPosition = 0;
	float3 transformedNormal = 0;

	//Skinning Magic:
	for (int i = 0; i < 4; i++)
	{
		int boneIndex = input.BoneIndices[i];
		if (boneIndex >= 0) //vertex is attached to this bone
		{
			//update position and normal
			transformedPosition += mul(input.BoneWeights[i],mul(float4(input.pos,1),gBones[boneIndex]));
			//only rotate normmal
			transformedNormal += mul(input.BoneWeights[i], mul(input.normal, (float3x3)gBones[boneIndex]));
		}
	}
	//w part of position to 1
	transformedPosition.w = 1;

	
	//Don't forget to change the output.pos & output.normal variables...
	output.pos = mul ( float4(transformedPosition), gWorldViewProj ); //skinned position
	output.normal = normalize(mul(transformedNormal, (float3x3)gWorld)); //skinned normal
	output.lPos = mul(float4(input.pos, 1), gWorldViewProj_Light);
	output.texCoord = input.texCoord;
	
	return output;
}

float2 texOffset(int u, int v)
{
	float w,h;
	gShadowMap.GetDimensions(w, h);
	return float2( u * 1.0f/w, v * 1.0f/h );
}

float EvaluateShadowMap(float4 lpos)
{
	//TODO: complete
	//back to homogenous space
	lpos /= lpos.w;

	//return ambient color if not within view of the light
	//range x : [-1,1]
	//range y : [-1,1]
	//range z : [0,1]
	if (lpos.x < -1.0f || lpos.x > 1.0f 
	|| lpos.y < -1.0f || lpos.y > 1.0f 
	|| lpos.z < 0.0f || lpos.z > 1.0f) return gAmbient; 

	//transform clip space coords [-1,1]  to texture space coords [0,1]
    lpos.x = lpos.x/2 + 0.5;
    lpos.y = lpos.y/-2 + 0.5;

	//apply shadow map bias
    lpos.z -= gShadowMapBias;

    //PCF sampling for shadow map
    float sum = 0;
    float x, y;
	//perform PCF filtering on a 4 x 4 texel neighborhood
    for (y = -1.5; y <= 1.5; y += 1.0)
    {
        for (x = -1.5; x <= 1.5; x += 1.0)
        {
            sum += gShadowMap.SampleCmpLevelZero( cmpSampler, lpos.xy + texOffset(x,y), lpos.z ).r;
        }
    }
    float shadowFactor = sum / 16.0;
	return shadowFactor;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET
{
	float shadowValue = EvaluateShadowMap(input.lPos);

	float4 diffuseColor = gDiffuseMap.Sample( samLinear,input.texCoord );
	float3 color_rgb= diffuseColor.rgb;
	float color_a = diffuseColor.a;
	
	//HalfLambert Diffuse :)
	float diffuseStrength = dot(input.normal, -gLightDirection);
	diffuseStrength = diffuseStrength * 0.5 + 0.5;
	diffuseStrength = saturate(diffuseStrength);
	color_rgb = color_rgb * diffuseStrength;

	return float4( color_rgb * shadowValue, color_a );
}

//--------------------------------------------------------------------------------------
// Technique
//--------------------------------------------------------------------------------------
technique11 Default
{
    pass P0
    {
		SetRasterizerState(NoCulling);
		SetDepthStencilState(EnableDepth, 0);

		SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}
