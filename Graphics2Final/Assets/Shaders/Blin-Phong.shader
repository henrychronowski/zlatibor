Shader "Custom/Blinn-Phong"
{
    Properties
    {
        uColor("Color", Color) = (1, 1, 1, 1)
        uSpecScale("Specular Scale", Float) = 1.0
        uSpecColor("Specular Color", Color) = (1, 1, 1, 1)
        uTexture("Texture", 2D) = "tex" {}
        uNormalMap("Normals", 2D) = "bump" {}
        uBumpScale("Bump Scale", Float) = 1.0
        uSpecularPower("Specular Power", Float) = 128
        uEmissionMap("Emission", 2D) = "black" {}
        uEmissiveStrength("Emissive Strength", Float) = 1.0
    }
        SubShader
    {
        Tags { "RenderType" = "Opaque" }

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
            sampler2D uNormalMap;
            sampler2D uEmissionMap;
            float uBumpScale;
            float uEmissiveStrength;
            float4 uTexture_ST;
            uniform float4 uColor;
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
                output.normal = normalize(mul(float4(v.normal, 0.0), unity_WorldToObject).xyz); // Calculate Normal
                output.position = UnityObjectToClipPos(v.vertex);
                output.uv = TRANSFORM_TEX(v.uv, uTexture);

                return output;
            }

            fixed4 frag(vertToFrag input) : COLOR
            {
                // Calculate Normal
                float3 normal;
                normal = UnpackScaleNormal(tex2D(uNormalMap, input.uv), uBumpScale);    // Unpack from DXT5nm to usable
                normal = normal.xzy;
                normal = normalize(normal);

                float3 viewDir = normalize(_WorldSpaceCameraPos.xyz - input.worldPos.xyz); // Calculate view direction
                float3 lightDir = normalize(_WorldSpaceLightPos0.xyz);
                float3 halfwayDir = normalize(lightDir + viewDir);

                float attenuation = lerp(1.0, 1.0 / lightDir, _WorldSpaceLightPos0.w);
                float3 lightFace = _WorldSpaceLightPos0.xyz - input.worldPos.xyz * _WorldSpaceLightPos0.w;

                float3 ambient = UNITY_LIGHTMODEL_AMBIENT.rgb * uColor.rgb;
                float3 diffuse = attenuation * _LightColor0.rgb * uColor.rgb * max(0.0, dot(normal, lightFace));
                float3 specular;
                
                if (dot(input.normal, lightFace) < 0.0)
                {
                    specular = float3(0.0, 0.0, 0.0);
                }
                else
                {
                    specular = attenuation* _LightColor0.rgb* uSpecColor.rgb* pow(max(dot(normal, halfwayDir), 0.0f), uSpecularPower);
                }

                float3 emissive = tex2D(uEmissionMap, input.uv.xy) * uEmissiveStrength;

                float3 color = (ambient + diffuse) * tex2D(uTexture, input.uv) + specular + emissive;

                return float4(color, 1);
            }
            
            ENDCG
        }

        Pass
        {
            Tags { "LightMode" = "ForwardAdd" }

            CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag

            #include "UnityCG.cginc" //Provides light and camera data

            uniform float4 _LightColor0; //From UnityCG

            sampler2D uTexture;
            float4 uTexture_ST;
            uniform float4 uColor;
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
                output.normal = normalize(mul(float4(v.normal, 0.0), unity_WorldToObject).xyz); // Calculate Normal
                output.position = UnityObjectToClipPos(v.vertex);
                output.uv = TRANSFORM_TEX(v.uv, uTexture);

                return output;
            }

            fixed4 frag(vertToFrag input) : COLOR
            {
                float3 normal = normalize(input.normal);
                float3 viewDir = normalize(_WorldSpaceCameraPos.xyz - input.worldPos.xyz); // Calculate view direction
                float3 lightDir = normalize(_WorldSpaceLightPos0.xyz);
                float3 halfwayDir = normalize(lightDir + viewDir);

                float attenuation = lerp(1.0, 1.0 / lightDir, _WorldSpaceLightPos0.w);
                float3 lightFace = _WorldSpaceLightPos0.xyz - input.worldPos.xyz * _WorldSpaceLightPos0.w;

                float3 ambient = UNITY_LIGHTMODEL_AMBIENT.rgb * uColor.rgb;
                float3 diffuse = attenuation * _LightColor0.rgb * uColor.rgb * max(0.0, dot(normal, lightFace));
                float3 specular = attenuation * _LightColor0.rgb * uSpecColor.rgb * pow(max(dot(normal, halfwayDir), 0.0f), uSpecularPower);

                float3 color = (ambient + diffuse) * tex2D(uTexture, input.uv) + specular;

                return float4(color, 1);
            }

            ENDCG
        }

        // Adds support for unity's shadow casting
        UsePass "Legacy Shaders/VertexLit/SHADOWCASTER"
    }
    CustomEditor "BlinnPhongGUI"
}
