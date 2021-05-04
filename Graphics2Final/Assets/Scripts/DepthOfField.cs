using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[ExecuteInEditMode, ImageEffectAllowedInSceneView]
public class DepthOfField : MonoBehaviour
{
    [HideInInspector]
    public Shader DOFShader;
    [HideInInspector]
    public Material DOFMat;

    public bool isActive = false;

    [Range(0.1f, 100f)]
    public float focusDistance = 10.0f;
    [Range(0.1f, 10f)]
    public float focusRange = 3.0f;
    [Range(1f, 10f)]
    public float bokehRadius = 4f;
    [Range(0, 4)]
    public int pass = 4;

    const int FOCUS_PASS = 0;
    const int DEPTH_DOWNSAMPLE_PASS = 1;
    const int BOKEH_PASS = 2;
    const int BLUR_PASS = 3;
    const int COMPOSITE_PASS = 4;

    private void Start()
    {
        Camera cam = GetComponent<Camera>();
        cam.depthTextureMode = cam.depthTextureMode | DepthTextureMode.Depth;
    }

    private void OnRenderImage(RenderTexture source, RenderTexture destination)
    {
        if(isActive)
        {
            // Create new material to be used by camera's RenderTexture
            if (DOFMat == null)
            {
                DOFMat = new Material(DOFShader);
                DOFMat.hideFlags = HideFlags.HideAndDontSave;
            }

            // Send uniforms to DOF shader
            DOFMat.SetFloat("_FocusDistance", focusDistance);
            DOFMat.SetFloat("_InvFocusRange", 1.0f / focusRange);
            DOFMat.SetFloat("_BokehRadius", bokehRadius);

            int halfWidth = source.width / 2;
            int halfHeight = source.height / 2;

            // Focus "g-buffer" (RenderTexture in Unity)
            RenderTexture focus = RenderTexture.GetTemporary(source.width, source.height, 0, RenderTextureFormat.RHalf, RenderTextureReadWrite.Linear);
            // "Bokeh "g-buffer"
            RenderTexture bokehHalfRes = RenderTexture.GetTemporary(halfWidth, halfHeight, 0, source.format);
            // "Half Res g-buffer"
            RenderTexture halfRes = RenderTexture.GetTemporary(halfWidth, halfHeight, 0, source.format);

            // Send g-buffer data to DOF shader
            DOFMat.SetTexture("_FocusTexture", focus);
            DOFMat.SetTexture("_DOFTexture", halfRes);

            // Focus Pass
            Graphics.Blit(source, focus, DOFMat, FOCUS_PASS);
            if (pass == FOCUS_PASS)
            {
                // Output only the focus pass
                Graphics.Blit(focus, destination);
                return;
            }
            // Depth Downsample Pass
            Graphics.Blit(source, halfRes, DOFMat, DEPTH_DOWNSAMPLE_PASS);
            if (pass == DEPTH_DOWNSAMPLE_PASS)
            {
                // Output only depth downsample pass
                Graphics.Blit(halfRes, destination);
                return;
            }
            // Bokeh Pass
            Graphics.Blit(halfRes, bokehHalfRes, DOFMat, BOKEH_PASS);
            if (pass == BOKEH_PASS)
            {
                // Output only depth bokeh pass
                Graphics.Blit(bokehHalfRes, destination);
                return;
            }
            // Bokeh Half Pass
            Graphics.Blit(bokehHalfRes, halfRes, DOFMat, BLUR_PASS);
            if (pass == BLUR_PASS)
            {
                // Output only depth bokeh half pass
                Graphics.Blit(halfRes, destination);
                return;
            }
            // Composite Pass
            Graphics.Blit(source, destination, DOFMat, COMPOSITE_PASS);

            // Release g-buffers
            RenderTexture.ReleaseTemporary(focus);
            RenderTexture.ReleaseTemporary(halfRes);
            RenderTexture.ReleaseTemporary(bokehHalfRes);
        }
        else
        {
            // Render the scene normally
            Graphics.Blit(source, destination);
        }
    }
}
