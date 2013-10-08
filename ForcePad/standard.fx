// Model quantities

float4x4 gmWorld;
float4x4 gmWorldInverseTranspose;

// Camera properties

float4x4 gmViewProjection;
float4   gvCameraPositionWorld;
float4x4 gmCameraInverseViewProjection;
float4x4 gmCameraInverseView;
float4x4 gmCameraInverseProjection;

// Light properties

float4x4 gmLightViewProjection;
float4   gvLightWorldDirection;
float4   gvLightAmbient;
float4   gvLightDiffuse;

// Surface properties

float4   gvMaterial;

// Textures

float gfTexN; // Texture size

texture  gTex;          // Diffuse texture
texture  gTexShadow;    // Shadow map
texture  gTexSpotlight; // Light's projected texture
texture  gtWBuffer;
texture  gtNormal;

float2 gvScreenPixels;

// Samplers

sampler TexS = sampler_state
{
    Texture = <gTex>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
};

sampler TexShadow = sampler_state
{
    Texture = <gTexShadow>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    AddressU = BORDER;
    AddressV = BORDER;
    BorderColor = 0;
};

sampler TexSpotlight = sampler_state
{
    Texture = <gTexSpotlight>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    AddressU = BORDER;
    AddressV = BORDER;
    BorderColor = 0;
};

sampler gsWBuffer = sampler_state
{
    Texture = <gtWBuffer>;
    MinFilter = POINT;
    MagFilter = POINT;
    MipFilter = POINT;
};

sampler gsNormal = sampler_state
{
    Texture = <gtNormal>;
    MinFilter = POINT;
    MagFilter = POINT;
    MipFilter = POINT;
};



// Simulation time

float gfTime;

struct VS_OUTPUT
{
    float4 position0 : POSITION0;
    float2 texcoord0: TEXCOORD0;
    float3 normal0 : TEXCOORD1;
    float4 worldPosition : TEXCOORD2;
    float4 clipPosition : TEXCOORD3;
};

struct PS_OUTPUT
{
    float4 Color[4] : COLOR0;
};

VS_OUTPUT WNormalVS(float4 position0 : POSITION0, 
                       float3 normal0   : NORMAL0)
{
    VS_OUTPUT outVS = (VS_OUTPUT) 0;
    outVS.worldPosition = mul(position0, gmWorld);
    outVS.position0 = mul(outVS.worldPosition, gmViewProjection);
    outVS.normal0 = normalize(mul(float4(normal0, 0), gmWorldInverseTranspose).xyz);
    outVS.clipPosition = outVS.position0;
    return outVS;
}


PS_OUTPUT WNormalPS(float3 worldNormal : TEXCOORD1,
                     float4 worldPosition : TEXCOORD2,
                     float4 clipPosition : TEXCOORD3)
{
    PS_OUTPUT output = (PS_OUTPUT) 0;
    output.Color[0].r = clipPosition.w;
    output.Color[1].rgb = worldNormal * 0.5 + 0.5;
    return output;
}

technique WNormal
{
    pass P0
    {
        vertexShader = compile vs_2_0 WNormalVS();
        pixelShader  = compile ps_2_0 WNormalPS();
    }
}






VS_OUTPUT WireframeVS(float4 position0 : POSITION0)
{
    VS_OUTPUT outVS = (VS_OUTPUT) 0;
    outVS.worldPosition = mul(position0, gmWorld);
    outVS.position0 = mul(outVS.worldPosition, gmViewProjection);
    return outVS;
}

PS_OUTPUT WireframePS()
{
    PS_OUTPUT output = (PS_OUTPUT) 0;
    output.Color[0] = 1;
    return output;
}

technique Wireframe
{
    pass P0
    {
        vertexShader = compile vs_2_0 WireframeVS();
        pixelShader = compile ps_2_0 WireframePS();
        FillMode = Wireframe;
    }
}




















VS_OUTPUT PassthroughVS(float4 position0 : POSITION0, 
                   float2 texcoord0 : TEXCOORD0)
{
    VS_OUTPUT outVS = (VS_OUTPUT) 0;
    outVS.position0 = position0;
    outVS.texcoord0 = texcoord0;
    outVS.clipPosition = position0;
    return outVS;
}

float4 PassthroughPS(float2 texcoord0 : TEXCOORD0) : COLOR
{
    float4 c = tex2D(TexS, texcoord0);
    //return (c.r * c.r - c.g) / c.g * 100000.0f + 0.5f;
    //return c.r * 0.002f;
    //return c;
    float4 biaser = float4(texcoord0.x, texcoord0.y, 0, 4);
    return 1.0f - 2.0f * (tex2D(TexS, texcoord0).r - tex2Dbias(TexS, biaser).r);
}


float4 BlurPS(float2 texcoord0 : TEXCOORD0) : COLOR
{
    texcoord0 += 0.5 / gfTexN;
    float4 c = tex2D(TexS, float2(texcoord0.y, texcoord0.x - 0.5f / gfTexN))
             + tex2D(TexS, float2(texcoord0.y, texcoord0.x + 0.5f / gfTexN))
             + tex2D(TexS, float2(texcoord0.y, texcoord0.x + 1.5f / gfTexN))
             + tex2D(TexS, float2(texcoord0.y, texcoord0.x - 1.5f / gfTexN))
             ;
    return c / 4.0f;
}


technique Blur
{
    pass P0
    {
        vertexShader = compile vs_2_0 PassthroughVS();
        pixelShader = compile ps_2_0 BlurPS();
        ZEnable = False;
        ZWriteEnable = False;
    }
}


float4 SSAOPS(float2 texcoord0 : TEXCOORD0) : COLOR
{
    float4 tc;
    tc.xy = texcoord0.xy;
    tc.z = 0;
    tc.w = 5;
    return 1 + tex2Dbias(TexS, tc).r - tex2D(TexS, texcoord0).r;
    //return tex2D(TexS, texcoord0);
}

float2 clip2texcoord(float4 x)
{
    return float2(
        +0.5f * x.x / x.w + 0.5f,
        -0.5f * x.y / x.w + 0.5f);
}


float4 ReconstructWorldPS(float2 texcoord0 : TEXCOORD0, float4 clipPosition : TEXCOORD3) : COLOR
{
    texcoord0 += 0.5f / gvScreenPixels;
    clipPosition.xy += float2(1, -1) / gvScreenPixels;
    // Reconstruct worldPosition
    float4 v;
    v.w = tex2D(gsWBuffer, texcoord0).r;
    v.xy = clipPosition.xy * v.w;
    v.z = 100.0f * (v.w - 1.0f) / 99.0f;
    float4 worldPosition = mul(v, gmCameraInverseViewProjection);

    float3 normal0 = tex2D(gsNormal, texcoord0).xyz - 0.5;

// ?!?!?!?!?!
// Render world position to 32F and compare
// Render normals to 32F and compare


    // delta to light
    float3 delta;
    delta.xyz = gvLightWorldDirection.xyz * worldPosition.w - worldPosition.xyz * gvLightWorldDirection.w;

    // Camera space
    float4 lightClipPosition = mul(worldPosition, gmLightViewProjection);
    float2 tc = clip2texcoord(lightClipPosition) + 0.5f / 256.0f;
    float2 us = tex2D(TexShadow, tc).rg;
    // Go from mean square to variance 
    // To prevent acne, slightly widen the distribution, proportional to the distance (and error)
    us.g = max(us.g - us.r * us.r * 0.99999f, us.g * 0.00001f);
    // We care about relative offset
    us.r -= lightClipPosition.w;
    float exposure = (us.r < 0) ? us.g / (us.g + us.r * us.r) : 1;

    // Occlusion

    float4 color;
    color.rgb = (max(0, dot(normalize(normal0), normalize(delta))) * exposure) * gvMaterial.rgb;
    color.a = 1;
    //return color;
    return float4(normal0 + 0.5f, 1);
    //return 1.0 / v.w;
    //return float4(frac(texcoord0 * gvScreenPixels + 0.5), 0, 0);

}


float4 ScatteringPS(float2 texcoord0 : TEXCOORD0, float4 clipPosition : TEXCOORD3) : COLOR
{
    // Reconstruct worldPosition
    float4 v;
    v.w = tex2D(TexS, texcoord0);
    v.xy = clipPosition.xy * v.w;
    v.z = 100.0f * (v.w - 1.0f) / 99.0f;
    float4 worldPosition = mul(v, gmCameraInverseViewProjection);

    // Begin hacks
    float killDepth = v.w;

    float exposure = 0;

    for (float i = 0; i < 10.0f; i += 0.1f)
    {
        // Hack
        v.w = i; // frac(gfTime * 0.1f) * 10.0f;
        v.xy = clipPosition.xy * v.w;
        v.z = 100.0f * (v.w - 1.0f) / 99.0f;
        worldPosition = mul(v, gmCameraInverseViewProjection);

        // Camera space
        float4 lightClipPosition = mul(worldPosition, gmLightViewProjection);
        float2 tc = clip2texcoord(lightClipPosition);
        float2 us = tex2D(TexShadow, tc).rg;
        // Go from mean square to variance 
        // To prevent acne, slightly widen the distribution, proportional to the distance (and error)
        us.g = max(us.g - us.r * us.r * 0.99999f, us.g * 0.00001f);
        // We care about relative offset
        us.r -= lightClipPosition.w;
        exposure += ((us.r < 0) ? us.g / (us.g + us.r * us.r) : 1) * ((i < killDepth) ? 1 : 0);
    }
    return exposure / 100.0f;
}





VS_OUTPUT AmbientVS(float4 position0 : POSITION0, 
                   float2 texcoord0 : TEXCOORD0)
{
    VS_OUTPUT outVS = (VS_OUTPUT) 0;
    outVS.position0 = mul(mul(position0, gmWorld), gmViewProjection);
    outVS.texcoord0 = texcoord0;
    return outVS;
}

float4 AmbientPS(float2 texcoord0 : TEXCOORD0) : COLOR
{
    return gvLightAmbient * (tex2D(TexS, texcoord0) * gvMaterial);
}


VS_OUTPUT IlluminatedVS(float4 position0 : POSITION0, 
                       float3 normal0   : NORMAL0, 
                       float2 texcoord0 : TEXCOORD0)
{
    VS_OUTPUT outVS = (VS_OUTPUT) 0;
    float3 vWorldNormal = normalize(mul(float4(normal0, 0), gmWorldInverseTranspose).xyz);
    
    outVS.worldPosition = mul(position0, gmWorld);
    
    outVS.position0 = mul(outVS.worldPosition, gmViewProjection);
    outVS.texcoord0 = texcoord0;
    outVS.normal0 = vWorldNormal;
    outVS.clipPosition = mul(outVS.worldPosition, gmLightViewProjection);
    return outVS;
}

static const float dt = 0 * 0.5f / 512;

float4 IlluminatedPS(float2 texcoord0 : TEXCOORD0, 
                     float3 normal0   : TEXCOORD1,
                     float4 worldPosition : TEXCOORD2,
                     float4 clipPosition : TEXCOORD3) : COLOR
{
    float3 delta;
    delta.xyz = gvLightWorldDirection.xyz * worldPosition.w - worldPosition.xyz * gvLightWorldDirection.w;

    float2 tc = clip2texcoord(clipPosition);
    float2 us = tex2D(TexShadow, tc).rg;
    // Go from mean square to variance 
    // To prevent acne, slightly widen the distribution, proportional to the distance (and error)
    us.g = max(us.g - us.r * us.r * 0.99999f, us.g * 0.00001f);
    // We care about relative offset
    us.r -= clipPosition.w;
    float exposure = (us.r < 0) ? us.g / (us.g + us.r * us.r) : 1;
    float4 color;
    color.rgb = (max(0, dot(normalize(normal0), normalize(delta))) * exposure * tex2D(TexSpotlight, tc)) * tex2D(TexS, texcoord0).rgb * gvMaterial.rgb * gvLightDiffuse.rgb;
    color.a = 1;
    return color;
}


float4 IlluminatedNoShadowPS(float2 texcoord0 : TEXCOORD0, 
                     float3 normal0   : TEXCOORD1,
                     float4 worldPosition : TEXCOORD2,
                     float4 clipPosition : TEXCOORD3) : COLOR
{
    float3 delta;
    delta.xyz = gvLightWorldDirection.xyz * worldPosition.w - worldPosition.xyz * gvLightWorldDirection.w;

    float2 tc = clip2texcoord(clipPosition);
    float2 us = tex2D(TexShadow, tc).rg;
    float4 color;
    color.rgb = (max(0, dot(normalize(normal0), normalize(delta))) * tex2D(TexSpotlight, tc)) * tex2D(TexS, texcoord0).rgb * gvMaterial.rgb * gvLightDiffuse.rgb;
    color.a = 1;
    return color;
}


float4 NormalPS(float2 texcoord0 : TEXCOORD0, 
                     float3 normal0   : TEXCOORD1) : COLOR
{
    float4 color;
    color.rgb = normalize(normal0) * 0.5 + 0.5;
    color.a = 1;
    return color;
}


VS_OUTPUT ClipVS(float4 position0 : POSITION0, 
                       float3 normal0   : NORMAL0, 
                       float2 texcoord0 : TEXCOORD0)
{
    VS_OUTPUT outVS = (VS_OUTPUT) 0;
    outVS.position0 = mul(mul(position0, gmWorld), gmViewProjection);
    outVS.clipPosition = outVS.position0;
    return outVS;
}

float4 ClipPS(float4 clipPosition : TEXCOORD3) : COLOR
{
    // return clipPosition.z / clipPosition.w;
    return clipPosition.w; // view-space z
}


VS_OUTPUT ShadowMapVS(float4 position0 : POSITION0)
{
    VS_OUTPUT outVS = (VS_OUTPUT) 0;
    outVS.position0 = mul(mul(position0, gmWorld), gmLightViewProjection);
    outVS.clipPosition = outVS.position0;
    return outVS;
}

// Requires ps_2_a for the ddx, ddy functions
float4 ShadowMapPS(float4 clipPosition : TEXCOORD3) : COLOR
{
    // return clipPosition.z / clipPosition.w;
    //return clipPosition.w; // view-space z
    float4 color = 0;
    color.r = clipPosition.w;
    float dx = ddx(clipPosition.w);
    float dy = ddy(clipPosition.w);
    color.g = clipPosition.w * clipPosition.w + 0.25*min((dx*dx + dy*dy), 1.0f);
    return color;
}



VS_OUTPUT DiffuseVS(float4 position0 : POSITION0, 
                       float3 normal0   : NORMAL0, 
                       float2 texcoord0 : TEXCOORD0)
{
    VS_OUTPUT outVS = (VS_OUTPUT) 0;
    float3 vWorldNormal = normalize(mul(float4(normal0, 0), gmWorldInverseTranspose).xyz);
    
    outVS.worldPosition = mul(position0, gmWorld);
    
    outVS.position0 = mul(mul(position0, gmWorld), gmViewProjection);
    outVS.texcoord0 = texcoord0;
    outVS.normal0 = vWorldNormal;
    return outVS;
}

float4 DiffusePS(float2 texcoord0 : TEXCOORD0, 
                     float3 normal0   : TEXCOORD1,
                     float4 worldPosition : TEXCOORD2) : COLOR
{
    float3 delta;
    delta.xyz = gvLightWorldDirection.xyz * worldPosition.w - worldPosition.xyz * gvLightWorldDirection.w;

    float4 color;
    color.rgb = (max(0, dot(normalize(normal0), normalize(delta)))) * (tex2D(TexS, texcoord0).rgb * gvMaterial.rgb * gvLightDiffuse.rgb);
    color.a = 1;
    return color;
}



VS_OUTPUT ShadowVS(float4 position0 : POSITION0, float3 normal0 : NORMAL0)
{
    VS_OUTPUT outVS = (VS_OUTPUT) 0;
    float3 vWorldNormal = mul(float4(normal0, 0), gmWorldInverseTranspose).xyz;
    float4 vWorldPosition = mul(position0, gmWorld);
    float4 delta;
    delta.xyz = vWorldPosition.xyz * gvLightWorldDirection.w - gvLightWorldDirection.xyz * vWorldPosition.w;
    delta.w = 0;
    
    if (dot(vWorldNormal, delta.xyz) >= 0)
    {
        outVS.position0 = mul(mul(position0, gmWorld), gmViewProjection);
    }
    else
    {
        outVS.position0 = mul(delta, gmViewProjection);
    }
    return outVS;
}



float4 SimplePS(float4 color0 : COLOR0) : COLOR
{
    return color0;
}

float4 ShadowVolumePS() : COLOR
{
    return 0;
}


technique Illuminated
{
    pass P0
    {
        vertexShader = compile vs_2_0 IlluminatedVS();
        pixelShader = compile ps_2_0 IlluminatedPS();

        AlphaBlendEnable = true;
        SrcBlend = One;
        DestBlend = One;
        BlendOp = Add;
    }
}

technique IlluminatedNoShadow
{
    pass P0
    {
        vertexShader = compile vs_2_0 IlluminatedVS();
        pixelShader = compile ps_2_0 IlluminatedNoShadowPS();

        AlphaBlendEnable = true;
        SrcBlend = One;
        DestBlend = One;
        BlendOp = Add;
    }
}

technique IlluminatedStencil
{
    pass P0
    {
        vertexShader = compile vs_2_0 IlluminatedVS();
        pixelShader = compile ps_2_0 IlluminatedNoShadowPS();
        StencilEnable = true;
        StencilFunc = Equal;
        ZFunc = Equal;
        AlphaBlendEnable = true;
        SrcBlend = One;
        DestBlend = One;
        BlendOp = Add;
    }
}


technique Ambient
{
    pass P0
    {
        vertexShader = compile vs_2_0 AmbientVS();
        pixelShader = compile ps_2_0 AmbientPS();
        ZFunc = Less;
    }
}

technique ShadowVolume
{
    pass P0
    {
        vertexShader = compile vs_2_0 ShadowVS();
        pixelShader = compile ps_2_0 ShadowVolumePS();
        CullMode = None;
        ZWriteEnable = false;
        Zfunc = Less;
        StencilEnable = true;
        TwoSidedStencilMode = true;
        StencilPass = Incr;
        Ccw_StencilPass = Decr;
        //ColorWriteEnable = 0;
        AlphaBlendEnable = true;
        SrcBlend = Zero;
        DestBlend = One;
        BlendOp = Add;
    }
}

technique StencilAddDiffuse
{
    pass P0
    {
        vertexShader = compile vs_2_0 DiffuseVS();
        pixelShader = compile ps_2_0 DiffusePS();
        //FillMode = Wireframe;
        StencilEnable = true;
        StencilFunc = Equal;
        ZFunc = Equal;
        
        AlphaBlendEnable = true;
        SrcBlend = One;
        DestBlend = One;
        BlendOp = Add;

    }
}

technique DrawNormal
{
    pass P0
    {
        vertexShader = compile vs_2_0 IlluminatedVS();
        pixelShader = compile ps_2_0 NormalPS();
    }
}



technique DrawClip
{
    pass P0
    {
        vertexShader = compile vs_2_0 ClipVS();
        pixelShader = compile ps_2_0 ClipPS();
        //CullMode = Ccw;
    }
}

technique ShadowMap
{
    pass P0
    {
        // Requires ps_2_a for the ddx, ddy functions
        vertexShader = compile vs_2_a ShadowMapVS();
        pixelShader = compile ps_2_a ShadowMapPS();
        // CullMode = Cw;
    }
}

technique Passthrough
{
    pass P0
    {
        vertexShader = compile vs_2_0 PassthroughVS();
        pixelShader = compile ps_2_0 PassthroughPS();
    }
}

technique ReconstructWorld
{
    pass P0
    {
        vertexShader = compile vs_3_0 PassthroughVS();
        pixelShader = compile ps_3_0 ReconstructWorldPS();
    }
}

technique SSAO
{
    pass P0
    {
        vertexShader = compile vs_2_0 PassthroughVS();
        pixelShader = compile ps_2_0 SSAOPS();
    }
}