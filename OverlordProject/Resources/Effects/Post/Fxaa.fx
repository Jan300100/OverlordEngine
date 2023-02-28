//=============================================================================
//// Shader uses position and texture
//=============================================================================
SamplerState samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Mirror;
    AddressV = Mirror;
};

SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Mirror;
    AddressV = Mirror;
};

Texture2D gDepthSRV;
Texture2D gColorSRV;

/// Create Depth Stencil State (ENABLE DEPTH WRITING)
DepthStencilState DepthWrite
{
    DepthEnable = TRUE;
    DepthWriteMask = ALL;
};

/// Create Rasterizer State (Backface culling) 
RasterizerState CullBack
{
    CullMode = BACK;
};

//IN/OUT STRUCTS
//--------------
struct VS_INPUT
{
    float3 Position : POSITION;
	float2 TexCoord : TEXCOORD0;

};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD1;
};


//VERTEX SHADER
//-------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	// Set the Position
    output.Position = float4(input.Position, 1);
	// Set the TexCoord
    output.TexCoord = input.TexCoord;
	return output;
}


//PIXEL SHADER
//------------
float4 PS(PS_INPUT input): SV_Target
{
	// Step 1: find the dimensions of the texture (the texture has a method for that)	
    int w, h;
    gDepthSRV.GetDimensions(w, h);
	// Step 2: calculate dx and dy (UV space for 1 pixel)	
    float dx = 1.0f / w;
    float dy = 1.0f / h;
	// Step 3: Create a double for loop (5 iterations each)
    
    float4 finalColor = (float4)0;
    
    for (int i = -2; i <= 2; i++)
    {
        for (int j = -2; j <= 2; j++)
        {
	        //  Inside the loop, calculate the offset in each direction. Make sure not to take every pixel but move by 2 pixels each time
            float2 offset = float2(i * dx, j * dy);
            //	Do a texture lookup using your previously calculated uv coordinates + the offset, and add to the final color
            finalColor += gDepthSRV.Sample(samPoint, input.TexCoord + offset);
        }
    }
	// Step 4: Divide the final color by the number of passes (in this case 5*5)
    finalColor /= 25.0f;
	// Step 5: return the final color
    return float4(finalColor.rgb, 1);
}




static const float EDGE_THRESHOLD_MAX = 0.125;
static const float EDGE_THRESHOLD_MIN = 0.0312;
static const int ITERATIONS = 12;
static const float SUBPIXEL_QUALITY = 0.75;

float quality(int i)
{
    return 1 + (ITERATIONS - i)*0.5f * (float(i) > float(ITERATIONS/2.0f));
}

float rgb2luma(float3 rgb){
    return sqrt(dot(rgb, float3(0.299f, 0.587f, 0.114f)));
}


float4 FxaaPS(PS_INPUT input): SV_Target
{
    //texture info
    int w, h;
    gDepthSRV.GetDimensions(w, h);
	// Step 2: calculate dx and dy (UV space for 1 pixel)	
    float dx = 1.0f / w;
    float dy = 1.0f / h;
    //

    float3 colorCenter = (gColorSRV.Sample(samLinear,input.TexCoord).rgb);
    float lumaCenter = rgb2luma(colorCenter);

    // Luma at every neighbor
    float lumas[9];
    //lumas:
    //0  3  6
    //1 [4] 7
    //2  5  8

    float maxLuma = 0, minLuma = 1;
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            lumas[i*3 + j] = rgb2luma(gColorSRV.Sample(samLinear,input.TexCoord + float2(i * dx, j * dy)).rgb);
            if ( lumas[i*3 + j] > maxLuma) maxLuma =  lumas[i*3 + j];
            else if (lumas[i*3 + j] < minLuma) minLuma = lumas[i*3 + j];
        }
    }
    // Compute the delta.
    float lumaRange = maxLuma - minLuma;

    // If the luma variation is lower that a threshold (or if we are in a really dark area), we are not on an edge, don't perform any AA.
    if(lumaRange < max(EDGE_THRESHOLD_MIN,maxLuma*EDGE_THRESHOLD_MAX))
        return float4(colorCenter,1);

    // Combine the four edges lumas (using intermediary variables for future computations with the same values).
    float lumaDownUp = lumas[5] + lumas[3];
    float lumaLeftRight = lumas[1] + lumas[7];

    // Same for corners
    float lumaLeftCorners = lumas[2] + lumas[0];
    float lumaDownCorners = lumas[2] + lumas[8];
    float lumaRightCorners = lumas[8] + lumas[6];
    float lumaUpCorners = lumas[6] + lumas[0];

    // Compute an estimation of the gradient along the horizontal and vertical axis.
    //direction of the edge
    float edgeHorizontal =  abs(-2.0 * lumas[1] + lumaLeftCorners)  + abs(-2.0 * lumas[4] + lumaDownUp ) * 2.0    + abs(-2.0 * lumas[7] + lumaRightCorners);
    float edgeVertical =    abs(-2.0 * lumas[3] + lumaUpCorners)      + abs(-2.0 * lumas[4] + lumaLeftRight) * 2.0  + abs(-2.0 * lumas[5] + lumaDownCorners);

    // // Is the local edge horizontal or vertical ?
    bool isHorizontal = (edgeHorizontal >= edgeVertical);

    //horizontal or vertical, there are still 2 sides at which the edge can be.
    //check gradients of 2 neighbors, either the up and bottom or the left and right, to see at what side the edge is.

    // Select the two neighboring texels lumas in the opposite direction to the local edge.
    float luma1 = isHorizontal ? lumas[5] : lumas[1];
    float luma2 = isHorizontal ? lumas[3] : lumas[7];
    // Compute gradients in this direction.
    float gradient1 = luma1 - lumaCenter;
    float gradient2 = luma2 - lumaCenter;

    // Which direction is the steepest ?
    bool is1Steepest = abs(gradient1) >= abs(gradient2);
    // Gradient in the corresponding direction, normalized.
    float gradientScaled = 0.25*max(abs(gradient1),abs(gradient2));

    // Choose the step size (one pixel) according to the edge direction.
    float stepLength = isHorizontal ? dy : dx;

    // Average luma in the correct direction.
    float lumaLocalAverage = 0.0;
    if(is1Steepest){
        // Switch the direction
        stepLength = - stepLength;
        lumaLocalAverage = 0.5*(luma1 + lumaCenter);
    } else {
        lumaLocalAverage = 0.5*(luma2 + lumaCenter);
    }

    // Shift UV in the correct direction by half a pixel.
    float2 currentUv = input.TexCoord;
    if(isHorizontal){
        currentUv.y += stepLength * 0.5;
    } else {
        currentUv.x += stepLength * 0.5;
    }

    // Compute offset (for each iteration step) in the right direction.
    float2 offset = isHorizontal ? float2(dx,0.0) : float2(0.0,dy);
    // Compute UVs to explore on each side of the edge, orthogonally. The QUALITY allows us to step faster.
    float2 uv1 = currentUv - offset;
    float2 uv2 = currentUv + offset;

    // Read the lumas at both current extremities of the exploration segment, and compute the delta wrt to the local average luma.
    float lumaEnd1 = rgb2luma(gColorSRV.Sample(samLinear,uv1).rgb);
    float lumaEnd2 = rgb2luma(gColorSRV.Sample(samLinear,uv2).rgb);
    lumaEnd1 -= lumaLocalAverage;
    lumaEnd2 -= lumaLocalAverage;

    // If the luma deltas at the current extremities are larger than the local gradient, we have reached the side of the edge.
    bool reached1 = abs(lumaEnd1) >= gradientScaled;
    bool reached2 = abs(lumaEnd2) >= gradientScaled;
    bool reachedBoth = reached1 && reached2;

    // If the side is not reached, we continue to explore in this direction.
    if(!reached1){
        uv1 -= offset;
    }
    if(!reached2){
        uv2 += offset;
    }

    // If both sides have not been reached, continue to explore.
    if(!reachedBoth)
    {

        for(int i = 2; i < ITERATIONS; i++){
            // If needed, read luma in 1st direction, compute delta.
            if(!reached1){
                lumaEnd1 = rgb2luma(gColorSRV.Sample(samLinear,uv1).rgb);
                lumaEnd1 = lumaEnd1 - lumaLocalAverage;
            }
            // If needed, read luma in opposite direction, compute delta.
            if(!reached2){
                lumaEnd2 = rgb2luma(gColorSRV.Sample(samLinear,uv2).rgb);
                lumaEnd2 = lumaEnd2 - lumaLocalAverage;
            }
            // If the luma deltas at the current extremities is larger than the local gradient, we have reached the side of the edge.
            reached1 = abs(lumaEnd1) >= gradientScaled;
            reached2 = abs(lumaEnd2) >= gradientScaled;
            reachedBoth = reached1 && reached2;

            // If the side is not reached, we continue to explore in this direction, with a variable quality.
            if(!reached1){
                uv1 -= offset * quality(i);
            }
            if(!reached2){
                uv2 += offset * quality(i);
            }

            // If both sides have been reached, stop the exploration.
            if(reachedBoth){ break;}
        }
    }

    // Compute the distances to each extremity of the edge.
    float distance1 = isHorizontal ? (input.TexCoord.x - uv1.x) : (input.TexCoord.y - uv1.y);
    float distance2 = isHorizontal ? (uv2.x - input.TexCoord.x) : (uv2.y - input.TexCoord.y);

    // In which direction is the extremity of the edge closer ?
    bool isDirection1 = distance1 < distance2;
    float distanceFinal = min(distance1, distance2);

    // Length of the edge.
    float edgeThickness = (distance1 + distance2);

    // UV offset: read in the direction of the closest side of the edge.
    float pixelOffset = - distanceFinal / edgeThickness + 0.5;

    // Is the luma at center smaller than the local average ?
    bool isLumaCenterSmaller = lumaCenter < lumaLocalAverage;

    // If the luma at center is smaller than at its neighbour, the delta luma at each end should be positive (same variation).
    // (in the direction of the closer side of the edge.)
    bool correctVariation = ((isDirection1 ? lumaEnd1 : lumaEnd2) < 0.0) != isLumaCenterSmaller;

    // If the luma variation is incorrect, do not offset.
    float finalOffset = correctVariation ? pixelOffset : 0.0;

    // Sub-pixel shifting
    // Full weighted average of the luma over the 3x3 neighborhood.
    float lumaAverage = (1.0/12.0) * (2.0 * (lumaDownUp + lumaLeftRight) + lumaLeftCorners + lumaRightCorners);
    // Ratio of the delta between the global average and the center luma, over the luma range in the 3x3 neighborhood.
    float subPixelOffset1 = clamp(abs(lumaAverage - lumaCenter)/lumaRange,0.0,1.0);
    float subPixelOffset2 = (-2.0 * subPixelOffset1 + 3.0) * subPixelOffset1 * subPixelOffset1;
    // Compute a sub-pixel offset based on this delta.
    float subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2 * SUBPIXEL_QUALITY;

    // Pick the biggest of the two offsets.
    finalOffset = max(finalOffset,subPixelOffsetFinal);

    // Compute the final UV coordinates.
    float2 finalUv = input.TexCoord;
    if(isHorizontal){
        finalUv.y += finalOffset * stepLength;
    } else {
        finalUv.x += finalOffset * stepLength;
    }

    // Read the color at the new UV coordinates, and use it.
    return gColorSRV.Sample(samLinear,finalUv);
}

//TECHNIQUE
//---------
technique11 Fxaa
{
    pass P0
    {
		// Set states...
        SetRasterizerState(CullBack);
        SetDepthStencilState(DepthWrite,0);
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetHullShader(NULL);
        SetDomainShader(NULL);
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, FxaaPS() ) );
    }
}