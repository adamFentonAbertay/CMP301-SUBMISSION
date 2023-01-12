// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"	// include dxframework
#include "ManipulationShader.h"
#include "BloomShader.h"
#include "BlurHorizontalShader.h"
#include "BlurVerticalShader.h"
#include "textureShader.h"
#include "LavaShader.h"
#include "DepthShader.h"

class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

	void wireframeRender();

	void bloomInitialPass();

protected:
	void bloomBlurHorizontalPass(RenderTexture* output, RenderTexture* toBlur);
	void bloomCull();
	void downscaleBloom(RenderTexture* output, RenderTexture* toDownscale);
	void upscaleBloom(RenderTexture* output, RenderTexture* toUpscale);
	void bloomBlurVerticalPass(RenderTexture* output, RenderTexture* toBlur);
	void addTextures(RenderTexture* add1, RenderTexture* add2);
	void depthPass();
	void depthSecondPass();
	bool render();
	void gui();

private:
	ManipulationShader* shader;
	BloomShader* bloom;
	RenderTexture* renderTexture;
	RenderTexture* cullBloomTexture;




	RenderTexture* horizontalBlurTexture8x;
	RenderTexture* verticalBlurTexture8x;

	
	RenderTexture* downscaleTexture8x;
	
	RenderTexture* upscaleTexture;

	BlurVerticalShader* verticalBlurShader;
	BlurHorizontalShader* horizontalBlurShader;
	LavaShader* lavaShader;
	textureShader* texture_Shader;
	bool bloomToggle;
	float bloomCutoff;
	PlaneMesh* mesh;
	Light* light;
	Light* light2;
	PlaneMesh* river;
	OrthoMesh* orthoMesh;
	Light* lightSpot;
	XMFLOAT3 lightSpotPosition;
	XMFLOAT3 lightSpotDir;
	XMFLOAT3 light1Position;
	XMFLOAT3 light1Dir;
	XMFLOAT3 light2Position;
	XMFLOAT3 light2Dir;
	float lightSpotCutoff;
	DepthShader* depthShader;
	CubeMesh* cubeMesh;

	float rotate;

	ShadowMap* shadowMap;
	ShadowMap* shadowMap2;
};

#endif