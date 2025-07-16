// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"	// include dxframework
#include "TextureShader.h"
#include "ShadowShader.h"
#include "DepthShader.h"
#include "HorizontalBlurShader.h"
#include "VerticalBlurShader.h"
#include "DoFShader.h"

class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected: // functions to be called in source
	bool render();
	void povDepthPass();
	void depthPass();
	void firstPass();
	void hBlurPass();
	void vBlurPass();
	void dofPass();
	void finalPass();
	void gui();

private:
	
	
	OrthoMesh* orthoMesh;

	// lights and shadows 
	Light* lights[2];
	ShadowMap* shadowMaps[2];

	// geometry
	AModel* model;
	CubeMesh* cubeMesh;
	PlaneMesh* mesh;
	PlaneMesh* sea;

	// shaders
	TextureShader* textureShader;
	ShadowShader* shadowShader;
	DepthShader* depthShader;
	HorizontalBlurShader* hBlurShader;
	VerticalBlurShader* vBlurShader;
	DoFShader* dofShader;
	
	// render textures
	RenderTexture* renderTexture;
	RenderTexture* povRenderTexture;
	RenderTexture* hBlurTexture;
	RenderTexture* vBlurTexture;
	RenderTexture* dofTexture;
	
	// interactable through sliders 
	float cubePos[3];
	float lightDirection[2][3];

	// blur settings 
	float dofDistance = 5.0f; 
};

#endif