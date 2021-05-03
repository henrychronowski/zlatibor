﻿Shader "Custom/DepthOfField"
{
    Properties
    {
        _MainTex("Texture", 2D) = "white" {}
    }

        SubShader
    {
        Cull Off ZTest Always ZWrite Off

        Tags { "RenderType" = "Opaque"}

        // Shared vertex shader
        CGINCLUDE
        #include "UnityCG.cginc"

        sampler2D _MainTex, _FocusTexture, _DOFTexture;
        uniform sampler2D _CameraDepthTexture;
        float4 _MainTex_TexelSize;
        float _FocusDistance, _InvFocusRange, _BokehRadius;

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

        ENDCG

        // Focus Pass
        Pass
        {
            CGPROGRAM

            #pragma vertex vert
            #pragma fragment frag


            half frag(vertex2fragment i) : SV_Target
            {
                float depth = SAMPLE_DEPTH_TEXTURE(_CameraDepthTexture, i.uv);
                depth = LinearEyeDepth(depth);

                float focus = (depth - _FocusDistance) * _InvFocusRange;
                focus = clamp(focus, -1, 1) * _BokehRadius;

                return focus;
            }
            ENDCG
        }
        
        // Depth Downsample Pass
        Pass
        {
            CGPROGRAM

            #pragma vertex vert
            #pragma fragment frag


            half Weigh(half3 col)
            {
                return 1 / (1 + max(max(col.r, col.g), col.b));
            }


            half4 frag(vertex2fragment i) : SV_Target
            {
               float4 quarts = _MainTex_TexelSize.xyxy * float2(-0.5, 0.5).xxyy;

               half3 source0 = tex2D(_MainTex, i.uv + quarts.xy).rgb;
               half3 source1 = tex2D(_MainTex, i.uv + quarts.zy).rgb;
               half3 source2 = tex2D(_MainTex, i.uv + quarts.xw).rgb;
               half3 source3 = tex2D(_MainTex, i.uv + quarts.zw).rgb;

               half weight0 = Weigh(source0);
               half weight1 = Weigh(source1);
               half weight2 = Weigh(source2);
               half weight3 = Weigh(source3);

               half3 col = source0 * weight0 + source1 * weight1 + source2 * weight2 + source3 * weight3;
               col /= max(weight0 + weight1 + weight2 + source3, 0.00001);

               half focus0 = tex2D(_FocusTexture, i.uv + quarts.xy).r;
               half focus1 = tex2D(_FocusTexture, i.uv + quarts.zy).r;
               half focus2 = tex2D(_FocusTexture, i.uv + quarts.xw).r;
               half focus3 = tex2D(_FocusTexture, i.uv + quarts.zw).r;

               half focusMin = min(min(min(focus0, focus1), focus2), focus3);
               half focusMax = max(max(max(focus0, focus1), focus2), focus3);

               half focus = focusMax >= -focusMin ? focusMax : focusMin;

               return half4(col, focus);
            }
            ENDCG
        }

        // Bokeh Pass
        Pass
        {
            CGPROGRAM

            #pragma vertex vert
            #pragma fragment frag

            // From https://github.com/Unity-Technologies/PostProcessing/
            // blob/v2/PostProcessing/Shaders/Builtins/DiskKernels.hlsl
            static const int kernelSampleCount = 22;
            static const float2 kernel[kernelSampleCount] = {
                float2(0, 0),
                float2(0.53333336, 0),
                float2(0.3325279, 0.4169768),
                float2(-0.11867785, 0.5199616),
                float2(-0.48051673, 0.2314047),
                float2(-0.48051673, -0.23140468),
                float2(-0.11867763, -0.51996166),
                float2(0.33252785, -0.4169769),
                float2(1, 0),
                float2(0.90096885, 0.43388376),
                float2(0.6234898, 0.7818315),
                float2(0.22252098, 0.9749279),
                float2(-0.22252095, 0.9749279),
                float2(-0.62349, 0.7818314),
                float2(-0.90096885, 0.43388382),
                float2(-1, 0),
                float2(-0.90096885, -0.43388376),
                float2(-0.6234896, -0.7818316),
                float2(-0.22252055, -0.974928),
                float2(0.2225215, -0.9749278),
                float2(0.6234897, -0.7818316),
                float2(0.90096885, -0.43388376),
            };
            
            

            half4 frag(vertex2fragment i) : SV_Target
            {
                half focus = tex2D(_MainTex, i.uv).a;

                half4 bgCol = 0, fgCol = 0;
                float bgWeight = 0, fgWeight = 0;
                
                // Sample texles around the current fragment
                for (int k = 0; k < kernelSampleCount; k++)
                {
                    float2 texelOffset = kernel[k] * _BokehRadius;

                    half radius = length(texelOffset);
                    texelOffset *= _MainTex_TexelSize.xy;
                    
                    half4 texSample = tex2D(_MainTex, i.uv + texelOffset);
                    
                    half sampleWeight = saturate((max(0, min(texSample.a, focus)) - radius + 2) * 0.5);
                    bgCol += texSample * sampleWeight;
                    bgWeight += sampleWeight;

                    sampleWeight = saturate((-texSample.a - radius + 2) * 0.5);
                    fgCol += texSample * sampleWeight;
                    fgWeight += sampleWeight;
                }

                bgCol *= 1.0 / (bgWeight + (bgWeight == 0));
                bgCol.w = 1;
                
                fgCol *= 1.0 / (fgWeight + (fgWeight == 0));
                fgCol.w = 1;

                half bgfg = min(1, fgWeight * 3.141592654 / kernelSampleCount);

                half3 col = lerp(bgCol.rgb, fgCol.rgb, bgfg);

                return half4(col, bgfg);
            }
         ENDCG
        }

        // Blur Pass
        Pass
        {
            CGPROGRAM

            #pragma vertex vert
            #pragma fragment frag


            half4 frag(vertex2fragment i) : SV_Target
            {
                float4 quarts = _MainTex_TexelSize.xyxy * float2(-0.5, 0.5).xxyy;

                half4 boxSample = tex2D(_MainTex, i.uv + quarts.xy) + tex2D(_MainTex, i.uv + quarts.zy) +
                                  tex2D(_MainTex, i.uv + quarts.xw) + tex2D(_MainTex, i.uv + quarts.zw);
                
                return boxSample * 0.25;
            }
            ENDCG
            }

        // Composite Pass
        Pass
        {
            CGPROGRAM

            #pragma vertex vert
            #pragma fragment frag


            half4 frag(vertex2fragment i) : SV_Target
            {
                half4 sourceTexture = tex2D(_MainTex, i.uv);
                half focus = tex2D(_FocusTexture, i.uv).r;
                half4 dof = tex2D(_DOFTexture, i.uv);

                half dofStrength = smoothstep(0.1, 1, focus);
                half3 col = lerp(sourceTexture.rgb, dof.rgb, dofStrength + dof.a - dofStrength * dof.a);
                
                return half4(col, sourceTexture.a);
            }
        ENDCG
        }
    }
}
