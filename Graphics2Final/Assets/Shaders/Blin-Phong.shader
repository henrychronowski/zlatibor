// Upgrade NOTE: replaced 'mul(UNITY_MATRIX_MVP,*)' with 'UnityObjectToClipPos(*)'

Shader "Custom/Blinn-Phong"
{
    Properties
    {
        _Color ("Color", Color) = (1, 1, 1, 1)
        uTexture("Texture", 2D) = "tex" {}
    }
        SubShader
    {
        Tags { "RenderType" = "Opaque" }

        Pass
        {
            CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag
            #include "UnityCG.cginc" //Provides light and camera data

            uniform sampler2D uTexture;
            uniform sampler2D uTex_shadow;
            uniform float4 uColor;
            uniform float4 uSpecColor;
            uniform float uRoughness;

            // Varyings - App -> vertex shader
            struct appdata
            {
                float4 vertex : POSITION;
                float3 normal : NORMAL;
                float2 texCoord : TEXCOORD0;
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
                //output.uv = TRANSFORM_TEX(v.texCoord, uTexture);

                return output;
            }

            fixed4 frag(vertToFrag input) : COLOR
            {
                return float4(0, 0, 1, 1);
            }
            
            ENDCG
        }
    }
}
