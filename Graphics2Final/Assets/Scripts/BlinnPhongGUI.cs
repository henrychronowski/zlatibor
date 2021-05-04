using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

// A custom editor interface for shaders, drawing from the editor for default shaders
// Written by Henry Chronowski
// Referenced https://catlikecoding.com/unity/tutorials/rendering/part-9/

public class BlinnPhongGUI : ShaderGUI
{
	static GUIContent staticLabel = new GUIContent();

	MaterialEditor editor;
	MaterialProperty[] properties;

	// Helper function to slightly reduce the length of calls
	MaterialProperty FindProperty(string name)
	{
		return FindProperty(name, properties);
	}

	// Helper function to simplify labelling GUI elements
	static GUIContent MakeLabel( string text, string tooltip = null)
	{
		staticLabel.text = text;
		staticLabel.tooltip = tooltip;
		return staticLabel;
	}

	public override void OnGUI(MaterialEditor materialEditor, MaterialProperty[] properties)
	{
		editor = materialEditor;
		this.properties = properties;

		// Display the main maps and then the parameters specific to this shader
		MainMaps();
		ShaderSpecific();
	}

	// General maps common on the default Unity shader
	void MainMaps()
	{
		GUILayout.Label("Main Maps", EditorStyles.boldLabel);

		// The base colour map
		MaterialProperty mainTex = FindProperty("uTexture");
		editor.TexturePropertySingleLine(MakeLabel("Albedo", "Albedo (RGB)"), mainTex, FindProperty("uColor"));

		NormalMap();
		EmissiveMap();

		// The offset and scale coordinates for the shader's UVs
		EditorGUI.indentLevel += 2;
		editor.TextureScaleOffsetProperty(mainTex); 
		EditorGUI.indentLevel -= 2;
	}

	// The normal map with a float intensity scale
	void NormalMap()
	{
		MaterialProperty map = FindProperty("uNormalMap");
		editor.TexturePropertySingleLine(MakeLabel("Normal Map"), map, map.textureValue ? FindProperty("uBumpScale") : null);
	}

	// The emissive map with a float intensity scale
	void EmissiveMap()
	{
		MaterialProperty map = FindProperty("uEmissionMap");
		editor.TexturePropertySingleLine(MakeLabel("Emissive"), map, map.textureValue ? FindProperty("uEmissiveStrength") : null);
	}

	// Non-general parameters (I know it's a bad wording but I can't think of anything better)
	void ShaderSpecific()
	{
		GUILayout.Label("Parameters", EditorStyles.boldLabel);
		EditorGUI.indentLevel += 2;
		SpecScale();
		SpecColor();
		SpecPower();
		EditorGUI.indentLevel -= 2;
	}

	// float scale of the specular effect
	void SpecScale()
	{
		MaterialProperty slider = FindProperty("uSpecScale");
		editor.ShaderProperty(slider, MakeLabel("Specular Scale"));
	}

	// Tint colour for the specular effect
	void SpecColor()
	{
		MaterialProperty slider = FindProperty("uSpecColor");
		editor.ShaderProperty(slider, MakeLabel("Specular Color"));
	}

	// The float power for the specular effect
	void SpecPower()
	{
		MaterialProperty slider = FindProperty("uSpecularPower");
		editor.ShaderProperty(slider, MakeLabel("Specular Power"));
	}
}
