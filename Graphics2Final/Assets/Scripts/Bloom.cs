using System.Collections;
using System.Collections.Generic;
using UnityEngine;

// Class to manage bloom on a camera 
// Written by Ethan Heil and Henry Chronowski
// Heavily referenced Project 2: Multi-Pass & Post-Processing Pipelines for the pass structure here
//		and of course referencing the blue book's algorithms and suggestions for improvements

[ExecuteInEditMode]
public class Bloom : MonoBehaviour
{
	// Editor Parameters
	public bool isActive = false;
	[Tooltip("Bright, Full Screen Blur, Half Screen Blur, Composite")]
	[Range(0, 3)]
	public int pass = 3;
	[Range(1, 16)]
	public int iterations = 1;
	[Range(0, 1)]
	public float threshold = 1;

	// Shader and material to run the passes with
	[HideInInspector]
	public Shader bloomShader;
	[HideInInspector]
	public Material bloomMat;

	// Pass identifiers
	const int BRIGHT_PASS = 0;
	const int FULL_BLUR_PASS = 1;
	const int HALF_BLUR_PASS = 2;
	const int COMPOSITE_PASS = 3;

	// Temporary storage for render textures moving through the passes
	RenderTexture[] textures = new RenderTexture[16];

	private void OnRenderImage(RenderTexture source, RenderTexture destination)
	{
		if (isActive)
		{
			// Create a new temporary bloom material to use
			if (bloomMat == null)
			{
				bloomMat = new Material(bloomShader);
				bloomMat.hideFlags = HideFlags.HideAndDontSave;
			}

			// Pass the bloom threshold value to the shader
			bloomMat.SetFloat("_Threshold", threshold);

			// Calculate the half height and width for the current camera
			int halfHeight, halfWidth, i;
			halfHeight = source.height / 2;
			halfWidth = source.width / 2;

			// Prepare and store the pre pass texture
			RenderTexture prePass;
			RenderTexture current = RenderTexture.GetTemporary(halfWidth, halfHeight, 0, source.format);
			textures[0] = current;
			prePass = textures[0];

			// Run the bright pass to adjust the contrast and highlight where bloom should happen
			Graphics.Blit(source, current, bloomMat, BRIGHT_PASS);

			// If the bright pass is selected as output set it as the destination render texture and stop
			if (pass == BRIGHT_PASS)
			{
				Graphics.Blit(current, destination);
				return;
			}

			// Run the full blur pass on a gradually decreasing sample of the render texture. Runs either the number of iterations or until the size is 1px
			for (i = 1; i < iterations && halfHeight > 2; i++)
			{
				// Decrease the size of the sample
				halfWidth /= 2;
				halfHeight /= 2;

				// Run and store the full blur pass
				current = RenderTexture.GetTemporary(halfWidth, halfHeight, 0, source.format);
				textures[i] = current;
				Graphics.Blit(prePass, current, bloomMat, FULL_BLUR_PASS);

				prePass = current;
			}

			// If the full blur pass is selected as output set it as the destination render texture and stop
			if (pass == FULL_BLUR_PASS)
			{
				Graphics.Blit(current, destination);
				return;
			}

			// Run the half blur pass for slightly fewer iterations with the sample gradually increasing in size, upsampling the render texture
			for (i -= 2; i >= 0; i--)
			{
				current = textures[i];
				textures[i] = null;

				Graphics.Blit(prePass, current, bloomMat, HALF_BLUR_PASS);
				RenderTexture.ReleaseTemporary(prePass);
				prePass = current;
			}

			// If the half blur pass is selected as output set it as the destination render texture and stop
			if (pass == HALF_BLUR_PASS)
			{
				Graphics.Blit(current, destination);
				return;
			}

			// Pass the source texture to the shader and run the compositing pass, outputting the result to the destination render texture
			bloomMat.SetTexture("_SourceTex", source);
			Graphics.Blit(prePass, destination, bloomMat, COMPOSITE_PASS);

			// Release the temporary render texture
			RenderTexture.ReleaseTemporary(prePass);
		}
		else	// If bloom is deactivated simply pass the source render texture
		{
			Graphics.Blit(source, destination);
		}
	}
}
