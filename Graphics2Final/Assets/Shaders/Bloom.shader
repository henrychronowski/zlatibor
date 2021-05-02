Shader "Custom/Bloom"
{
    Properties
    {
        _MainTex ("Texture", 2D) = "white" {}
    }

    SubShader
    {
        Cull Off ZTest Always ZWrite Off

        // Shared vertex shader
        CGINCLUDE
        #include "UnityCG.cginc"
                
        sampler2D _MainTex, _SourceTex;
        float4 _MainTex_TexelSize;
        float _Threshold;

        float3 luminanceFormula = float3(0.299, 0.587, 0.144);

        struct inVert
        {
            float4 vertex : POSITION;
            float2 uv : TEXCOORD0;
        };

        struct vertex2fragment
        {
            float2 uv : TEXCOORD0;
            float4 vertex : SV_POSITION;
        };

        vertex2fragment vert(inVert v)
        {
            vertex2fragment o;
            o.vertex = UnityObjectToClipPos(v.vertex);
            o.uv = v.uv;
            return o;
        }

        half4 BoxSample(float2 uv, float offset)
        {
            float4 quarts = float4(_MainTex_TexelSize.xyxy * float2(-offset, offset).xxyy);

            half4 boxSample = tex2D(_MainTex, uv + quarts.xy) + tex2D(_MainTex, uv + quarts.zy) + 
                                tex2D(_MainTex, uv + quarts.xw) + tex2D(_MainTex, uv + quarts.zw);
            return boxSample * 0.25;
        }

        half3 Prefilter(half3 col)
        {
            half brightness = max(col.r, max(col.g, col.b));
            half contribution = max(0, brightness - _Threshold);
            contribution /= max(brightness, 0.00001);
            return col * contribution;
        }

        ENDCG

            // Bright Pass
            Pass
        {
            CGPROGRAM

            #pragma vertex vert
            #pragma fragment frag


            fixed4 frag(vertex2fragment i) : SV_Target
            {
                half4 col;
                col.rgb = Prefilter(BoxSample(i.uv, 1.0).rgb);
                col.a = 1.0;
                return col;
            }
            ENDCG
        }

        // Full Screen blur
            Pass
        {
            CGPROGRAM

            #pragma vertex vert
            #pragma fragment frag


            fixed4 frag(vertex2fragment i) : SV_Target
            {
                half4 col = BoxSample(i.uv, 1.0);
                col.w = 1.0;
                return col;
            }
            ENDCG
        }

        // Half screen blur
        Pass
        {
            Blend One One
            CGPROGRAM

            #pragma vertex vert
            #pragma fragment frag


            fixed4 frag(vertex2fragment i) : SV_Target
            {
                half4 col = BoxSample(i.uv, 0.5);
                col.w = 1.0;
                return col;
            }
            ENDCG
        }

            // Composite pass
            Pass
        {
            CGPROGRAM

            #pragma vertex vert
            #pragma fragment frag


            fixed4 frag(vertex2fragment i) : SV_Target
            {
                half4 sourceTex = tex2D(_SourceTex, i.uv);
                half4 col = BoxSample(i.uv, 0.5);
                
                col += sourceTex;

                col.w = 1.0;
                return col;
            }
            ENDCG
        }

    }
}
