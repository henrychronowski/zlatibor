// Written by Henry Chronowski and Ethan Heil
// The blue book was referenced for methodology/algorithms and we followed this tutorial from catlikecoding.com 
//      for the HLSL implementation: https://catlikecoding.com/unity/tutorials/advanced-rendering/bloom/

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

        // Factors to get relative luminance, from the blue book listing 9.26
        float3 luminanceFormula = float3(0.299, 0.587, 0.144);

        // Data received from the cpu
        struct inVert
        {
            float4 vertex : POSITION;
            float2 uv : TEXCOORD0;
        };

        // Data passed from vertex shader to fragment shader
        struct vertex2fragment
        {
            float2 uv : TEXCOORD0;
            float4 vertex : SV_POSITION;
        };

        // Actual vertex main program
        vertex2fragment vert(inVert v)
        {
            // Transform vertex position into clip space
            vertex2fragment o;
            o.vertex = UnityObjectToClipPos(v.vertex);
            o.uv = v.uv;
            return o;
        }

        
        half4 BoxSample(float2 uv, float offset)
        {
            // Down sample scene texture using a box filter https://scholarworks.rit.edu/cgi/viewcontent.cgi?referer=https://www.google.com/&httpsredir=1&article=10864&context=theses pg.24
            float4 quarts = float4(_MainTex_TexelSize.xyxy * float2(-offset, offset).xxyy);
            half4 boxSample = tex2D(_MainTex, uv + quarts.xy) + tex2D(_MainTex, uv + quarts.zy) + 
                                tex2D(_MainTex, uv + quarts.xw) + tex2D(_MainTex, uv + quarts.zw);
            return boxSample * 0.25;
        }

        half3 Prefilter(half3 col)
        {
            // Get color's maximum component to determine brightness
            half brightness = max(col.r, max(col.g, col.b));
            // Calculate color's contribution factor
            half contribution = max(0, brightness - _Threshold);
            contribution /= max(brightness, 0.00001); // Ensure that denominator is never 0
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
                // Pre filter color
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
                // Blur using box filter
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
                // Half screen blur using box filter
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
                // Sample scene texture
                half4 sourceTex = tex2D(_SourceTex, i.uv);

                // Apply fina plur
                half4 col = BoxSample(i.uv, 0.5);
                
                // Composite
                col += sourceTex;

                col.w = 1.0;
                return col;
            }
            ENDCG
        }

    }
}
