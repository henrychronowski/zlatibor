using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

// A custom editor interface for shaders, drawing from the editor for default shaders
// Written by Henry Chronowski
// Referenced https://catlikecoding.com/unity/tutorials/rendering/part-9/

public class CustomShaderGui : ShaderGUI
{
	static GUIContent staticLabel = new GUIContent();

	MaterialEditor editor;
	MaterialProperty[] properties;

	MaterialProperty FindProperty(string name)
	{
		return FindProperty(name, properties);
	}

	static GUIContent MakeLabel( string text, string tooltip = null)
	{
		staticLabel.text = text;
		staticLabel.tooltip = tooltip;
		return staticLabel;
	}

	public override void OnGUI(MaterialEditor materialEditor, MaterialProperty[] properties)
	{
		this.editor = materialEditor;
		this.properties = properties;

		MainMaps();
	}

	void MainMaps()
	{
		GUILayout.Label("Main Maps", EditorStyles.boldLabel);

		MaterialProperty mainTex = FindProperty("uTexture");
		editor.TexturePropertySingleLine(MakeLabel("Albedo", "Albedo (RGB)"), mainTex, FindProperty("uColor"));

		//MetallicMap();
		//Smoothness();
		NormalMap();

		editor.TextureScaleOffsetProperty(mainTex);
	}

	void NormalMap()
	{
		MaterialProperty map = FindProperty("uNormalMap");
		editor.TexturePropertySingleLine(MakeLabel("Normal Map"), map, map.textureValue ? FindProperty("uBumpScale") : null);
	}

	void MetallicMap()
	{
		MaterialProperty slider = FindProperty("_Metallic");
		EditorGUI.indentLevel += 2;
		editor.ShaderProperty(slider, MakeLabel("Metallic"));
		EditorGUI.indentLevel -= 2;
	}

	void Smoothness()
	{
		MaterialProperty slider = FindProperty("_Smoothness");
		EditorGUI.indentLevel += 2;
		editor.ShaderProperty(slider, MakeLabel("slider"));
		EditorGUI.indentLevel -= 2;
	}
}
