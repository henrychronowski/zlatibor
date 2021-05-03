using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[ExecuteInEditMode, ImageEffectAllowedInSceneView]
public class DepthOfField : MonoBehaviour
{
    [HideInInspector]
    public Shader DOFShader;
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
            if (DOFMat == null)
            {
                DOFMat = new Material(DOFShader);
                DOFMat.hideFlags = HideFlags.HideAndDontSave;
            }

            
            DOFMat.SetFloat("_FocusDistance", focusDistance);
            DOFMat.SetFloat("_InvFocusRange", 1.0f / focusRange);
            DOFMat.SetFloat("_BokehRadius", bokehRadius);

            int halfWidth = source.width / 2;
            int halfHeight = source.height / 2;

            RenderTexture focus = RenderTexture.GetTemporary(source.width, source.height, 0, RenderTextureFormat.RHalf, RenderTextureReadWrite.Linear);
            RenderTexture bokehHalfRes = RenderTexture.GetTemporary(halfWidth, halfHeight, 0, source.format);
            RenderTexture halfRes = RenderTexture.GetTemporary(halfWidth, halfHeight, 0, source.format);

            DOFMat.SetTexture("_FocusTexture", focus);
            DOFMat.SetTexture("_DOFTexture", halfRes);

            Graphics.Blit(source, focus, DOFMat, FOCUS_PASS);
            if (pass == FOCUS_PASS)
            {
                Graphics.Blit(focus, destination);
                return;
            }
            Graphics.Blit(source, halfRes, DOFMat, DEPTH_DOWNSAMPLE_PASS);
            if (pass == DEPTH_DOWNSAMPLE_PASS)
            {
                Graphics.Blit(halfRes, destination);
                return;
            }
            Graphics.Blit(halfRes, bokehHalfRes, DOFMat, BOKEH_PASS);
            if (pass == BOKEH_PASS)
            {
                Graphics.Blit(bokehHalfRes, destination);
                return;
            }
            Graphics.Blit(bokehHalfRes, halfRes, DOFMat, BLUR_PASS);
            if (pass == BLUR_PASS)
            {
                Graphics.Blit(halfRes, destination);
                return;
            }
            Graphics.Blit(source, destination, DOFMat, COMPOSITE_PASS);

            RenderTexture.ReleaseTemporary(focus);
            RenderTexture.ReleaseTemporary(halfRes);
            RenderTexture.ReleaseTemporary(bokehHalfRes);
        }
        else
        {
            Graphics.Blit(source, destination);
        }
    }
}
