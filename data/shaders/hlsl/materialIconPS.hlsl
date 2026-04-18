#include "LightHelper.hlsli"
#include "const_buffers/cbps_rare_changed.hlsli"
#include "const_buffers/cb_camera.hlsli"
#include "const_buffers/cbps_material_colors.hlsli"
#include "types/ps_in_basic.hlsli"


//---------------------------
// GLOBALS
//---------------------------
TextureCube  gCubeMap       : register(t0);
Texture2D    gTextures[22]  : register(t100);
SamplerState gBasicSampler  : register(s0);

//---------------------------
// PIXEL SHADER
//---------------------------
float4 PS(PS_IN pin) : SV_Target
{
    const float2 uv = float2(pin.texU, pin.texV);

    float4 textureColor = gTextures[1].Sample(gBasicSampler, uv);
    float3 normalMap    = gTextures[6].Sample(gBasicSampler, uv).rgb;

	
    // execute alpha clipping
    if (gAlphaClipping)
        clip(textureColor.a - 0.1f);
	
	// --------------------  NORMAL MAP   --------------------
	  
	// a vector in the world space from vertex to eye pos
    float3 toEyeW = normalize(gCamPosW - pin.posW);
	  
    // normalize the normal vector after interpolation
    float3 normalW = normalize(pin.normalW);

	// for foliage, tree branches, bushes: fix normals (when cull:none)
	if (gAlphaClipping && (dot(toEyeW, normalW) < 0))
		normalW *= -1;
	
	// pack normal into [0,1]
	//return float4(normalW, 1.0);
	//return float4(0.5 * normalW + 0.5, 1.0);

	// alpha only
	//return float4(1,1,1,1);
	
	// color only
	return float4(textureColor.rgb, 1.0);

	// compute the bumped normal in the world space
    float3 bumpedNormalW = NormalSampleToWorldSpace(normalMap, normalW, pin.tangentW);
	float3 normal = normalW;

    // --------------------  LIGHT   --------------------

    // start with a sum of zero
    float4 ambient = float4(0,0,0,0);
    float4 diffuse = float4(0,0,0,0);
    float4 spec    = float4(0,0,0,0);

    // sum the light contribution from each light source (ambient, diffuse, specular)
    float4 A, D, S;

    // hardcoded directed light
    DirectionalLight dirLight;
    dirLight.ambient   = float4(1,1,1,1);
    dirLight.diffuse   = float4(1,1,1,1);
    dirLight.specular  = float4(0.3f, 0.3f, 0.3f, 1.0f);
    dirLight.direction = normalize(float3(-1.0f, -1.0f, 1));

    Material mat;
    mat.ambient = gAmbient;
    mat.diffuse = gDiffuse;
    mat.specular = gSpecular;
    mat.reflect = gReflect;

    ComputeDirectionalLight(
        mat,
        dirLight,
        normal,
        toEyeW,
        0.0f,             // specular map value
        A, D, S);

    ambient += A;
    diffuse += D;
    spec += S;


    // modulate with late add
    float4 litColor = textureColor * (ambient + diffuse) + spec;
    litColor.a = 1.0f;

    // common to take alpha from diffuse material and texture
    //litColor.a = ((Material)pin.material).diffuse.a * textureColor.a;

    return litColor;


}
