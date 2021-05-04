using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[ExecuteInEditMode]
public class Bloom : MonoBehaviour
{
	public bool isActive = false;
	[Tooltip("Bright, Full Screen Blur, Half Screen Blur, Composite")]
	[Range(0, 3)]
	public int pass = 3;
	[Range(1, 16)]
	public int iterations = 1;
	[Range(0, 1)]
	public float threshold = 1;

	[HideInInspector]
	public Shader bloomShader;
	[HideInInspector]
	public Material bloomMat;

	const int BRIGHT_PASS = 0;
	const int FULL_BLUR_PASS = 1;
	const int HALF_BLUR_PASS = 2;
	const int COMPOSITE_PASS = 3;

	RenderTexture[] textures = new RenderTexture[16];

	private void OnRenderImage(RenderTexture source, RenderTexture destination)
	{
		if (isActive)
		{
			if (bloomMat == null)
			{
				bloomMat = new Material(bloomShader);
				bloomMat.hideFlags = HideFlags.HideAndDontSave;
			}

			bloomMat.SetFloat("_Threshold", threshold);

			int halfHeight, halfWidth, i;
			halfHeight = source.height / 2;
			halfWidth = source.width / 2;

			RenderTexture prePass;
			RenderTexture current = RenderTexture.GetTemporary(halfWidth, halfHeight, 0, source.format);
			textures[0] = current;
			prePass = textures[0];

			Graphics.Blit(source, current, bloomMat, BRIGHT_PASS);
			if (pass == BRIGHT_PASS)
			{
				Graphics.Blit(current, destination);
				return;
			}

			for (i = 1; i < iterations && halfHeight > 2; i++)
			{
				halfWidth /= 2;
				halfHeight /= 2;

				current = RenderTexture.GetTemporary(halfWidth, halfHeight, 0, source.format);
				textures[i] = current;
				Graphics.Blit(prePass, current, bloomMat, FULL_BLUR_PASS);

				prePass = current;
			}

			if (pass == FULL_BLUR_PASS)
			{
				Graphics.Blit(current, destination);
				return;
			}
			for (i -= 2; i >= 0; i--)
			{
				current = textures[i];
				textures[i] = null;

				Graphics.Blit(prePass, current, bloomMat, HALF_BLUR_PASS);
				RenderTexture.ReleaseTemporary(prePass);
				prePass = current;
			}

			if (pass == HALF_BLUR_PASS)
			{
				Graphics.Blit(current, destination);
				return;
			}
			bloomMat.SetTexture("_SourceTex", source);
			Graphics.Blit(prePass, destination, bloomMat, COMPOSITE_PASS);


			RenderTexture.ReleaseTemporary(prePass);
		}
		else
		{
			Graphics.Blit(source, destination);
		}
	}
}
