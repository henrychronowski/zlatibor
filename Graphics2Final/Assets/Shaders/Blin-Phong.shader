// Written by Henry Chronowski and Ethan Heil
// The blue book was heavily referenced for methodology although everything had to be translated into HLSL
// Referenced for HLSL conversion and how to deal with multiple light sources in Unity: https://janhalozan.com/2017/08/12/phong-shader/

Shader "Custom/Blinn-Phong"
{
    Properties
    {
        // Base colour and tint
        uTexture("Texture", 2D) = "tex" {}
        uColor("Color", Color) = (1, 1, 1, 1)
        
        // Normal and scale
        uNormalMap("Normals", 2D) = "bump" {}
        uBumpScale("Bump Scale", Float) = 1.0
        
        // Specular
        uSpecScale("Specular Scale", Float) = 1.0
        uSpecColor("Specular Color", Color) = (1, 1, 1, 1)
        uSpecularPower("Specular Power", Float) = 128
        
        // Emissive map and scale
        uEmissionMap("Emission", 2D) = "black" {}
        uEmissiveStrength("Emissive Strength", Float) = 1.0
    }
        SubShader
    {
        Tags { "RenderType" = "Opaque" }

        // First pass for primary light in scene and base colour
        Pass
        {
            Tags { "LightMode" = "ForwardBase" }

            CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag

            #include "UnityCG.cginc" //Provides light and camera data
            #include "UnityStandardUtils.cginc" //Provides unpack normal function to deal with DXT5nm

            uniform float4 _LightColor0; //From UnityCG

            sampler2D uTexture;
            float4 uTexture_ST;
            uniform float4 uColor;

            sampler2D uNormalMap;
            float uBumpScale;

            sampler2D uEmissionMap;
            float uEmissiveStrength;
            
            uniform float4 uSpecColor;
            uniform float uSpecScale;
            uniform float uSpecularPower;

            // Varyings - App -> vertex shader
            struct appdata
            {
                float4 vertex : POSITION;
                float3 normal : NORMAL;
                float2 uv : TEXCOORD0;
            };

            // Varyings - vertex shader -> fragment shader
            struct vertToFrag
            {
                float4 position : POSITION;
                float3 normal : NORMAL;
                float2 uv : TEXCOORD0;
                float4 worldPos : TEXCOORD1;
            };

            //Vertex Shader
            vertToFrag vert(appdata v)
            {
                vertToFrag output;

                output.worldPos = mul(unity_ObjectToWorld, v.vertex); // Calculate world position of vertex
                output.normal = normalize(mul(float4(v.normal, 0.0), unity_WorldToObject).xyz); // Calculate vertex Normal
                output.position = UnityObjectToClipPos(v.vertex);   // Transform position to clip space
                output.uv = TRANSFORM_TEX(v.uv, uTexture);

                return output;
            }

            //Fragment Shader
            fixed4 frag(vertToFrag input) : COLOR
            {
                // Calculate fragment normal from normal map
                float3 normal;
                normal = UnpackScaleNormal(tex2D(uNormalMap, input.uv), uBumpScale);    // Unpack from DXT5nm to usable
                normal = normal.xzy;
                normal = normalize(normal);

                // Calculate the view direction, light direction, and the vector halfway between
                float3 viewDir = normalize(_WorldSpaceCameraPos.xyz - input.worldPos.xyz); // view to fragment
                float3 lightDir = normalize(_WorldSpaceLightPos0.xyz);  // fragment to light
                float3 halfwayDir = normalize(lightDir + viewDir);

                // Calculate the attenuation and the facing direction of the light. Attenuation algorithm based off of blue book
                float attenuation = lerp(1.0, 1.0 / lightDir, _WorldSpaceLightPos0.w);
                float3 lightFace = _WorldSpaceLightPos0.xyz - input.worldPos.xyz * _WorldSpaceLightPos0.w;

                // Acquire the ambient light factor and calculate the diffuse color
                float3 ambient = UNITY_LIGHTMODEL_AMBIENT.rgb * uColor.rgb;
                float3 diffuse = attenuation * _LightColor0.rgb * uColor.rgb * max(0.0, dot(normal, lightFace));
                float3 specular;
                
                // If the light is behind the vertex the pixel is on do not add specular
                if (dot(input.normal, lightFace) < 0.0)
                {
                    specular = float3(0.0, 0.0, 0.0);
                }
                else
                {
                    specular = attenuation* _LightColor0.rgb* uSpecColor.rgb* pow(max(dot(normal, halfwayDir), 0.0f), uSpecularPower);
                }

                // Add the scaled emissive factor
                float3 emissive = tex2D(uEmissionMap, input.uv.xy) * uEmissiveStrength;

                // Scale the base color by the ambient and diffuse then add the specular and emissive light
                float3 color = (ambient + diffuse) * tex2D(uTexture, input.uv) + specular + emissive;

                return float4(color, 1);
            }
            
            ENDCG
        }

        // This pass additively blends the influence of all secondary lights
        Pass
        {
            Tags { "LightMode" = "ForwardAdd" }
            Blend One One

            CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag

            #include "UnityCG.cginc" //Provides light and camera data
            #include "UnityStandardUtils.cginc" //Provides unpack normal function to deal with DXT5nm

            uniform float4 _LightColor0; //From UnityCG

            sampler2D uTexture;
            float4 uTexture_ST;
            uniform float4 uColor;

            sampler2D uNormalMap;
            float uBumpScale;

            sampler2D uEmissionMap;
            float uEmissiveStrength;

            uniform float4 uSpecColor;
            uniform float uSpecScale;
            uniform float uSpecularPower;

            // Varyings - App -> vertex shader
            struct appdata
            {
                float4 vertex : POSITION;
                float3 normal : NORMAL;
                float2 uv : TEXCOORD0;
            };

            // Varyings - vertex shader -> fragment shader
            struct vertToFrag
            {
                float4 position : POSITION;
                float3 normal : NORMAL;
                float2 uv : TEXCOORD0;
                float4 worldPos : TEXCOORD1;
            };

            //Vertex Shader
            vertToFrag vert(appdata v)
            {
                vertToFrag output;

                output.worldPos = mul(unity_ObjectToWorld, v.vertex); // Calculate world position of vertex
                output.normal = normalize(mul(float4(v.normal, 0.0), unity_WorldToObject).xyz); // Calculate vertex Normal
                output.position = UnityObjectToClipPos(v.vertex);   // Transform position to clip space
                output.uv = TRANSFORM_TEX(v.uv, uTexture);

                return output;
            }

            // Fragment Shader
            fixed4 frag(vertToFrag input) : COLOR
            {
                // Calculate fragment normal from normal map
                float3 normal;
                normal = UnpackScaleNormal(tex2D(uNormalMap, input.uv), uBumpScale);    // Unpack from DXT5nm to usable
                normal = normal.xzy;
                normal = normalize(normal);

                // Calculate the view direction, light direction, and the vector halfway between
                float3 viewDir = normalize(_WorldSpaceCameraPos.xyz - input.worldPos.xyz); // view to fragment
                float3 lightDir = normalize(_WorldSpaceLightPos0.xyz);  // fragment to light
                float3 halfwayDir = normalize(lightDir + viewDir);

                // Calculate the attenuation and the facing direction of the light. Attenuation algorithm based off of blue book
                float attenuation = lerp(1.0, 1.0 / lightDir, _WorldSpaceLightPos0.w);
                float3 lightFace = _WorldSpaceLightPos0.xyz - input.worldPos.xyz * _WorldSpaceLightPos0.w;

                // Calculate the diffuse colour
                float3 diffuse = attenuation * _LightColor0.rgb * uColor.rgb * max(0.0, dot(normal, lightFace));
                float3 specular;

                // If the light is behind the vertex the pixel is on do not add specular
                if (dot(input.normal, lightFace) < 0.0)
                {
                    specular = float3(0.0, 0.0, 0.0);
                }
                else
                {
                    specular = attenuation * _LightColor0.rgb * uSpecColor.rgb * pow(max(dot(normal, halfwayDir), 0.0f), uSpecularPower);
                }

                // Add the scaled emissive factor
                float3 emissive = tex2D(uEmissionMap, input.uv.xy) * uEmissiveStrength;

                // Scale the base color by the ambient and diffuse then add the specular and emissive light
                float3 color = diffuse * tex2D(uTexture, input.uv) + specular + emissive;

                return float4(color, 1);
            }

            ENDCG
        }

        // Adds support for unity's shadow casting, allowing this to be drawn into the depth map
        UsePass "Legacy Shaders/VertexLit/SHADOWCASTER"
    }
    CustomEditor "BlinnPhongGUI"
}
