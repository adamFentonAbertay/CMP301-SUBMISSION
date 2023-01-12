// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"

//need to upscale thingy slides lecture 7
//also adjust to work with any textures rather than hardcoded
//as if its not adding together
App1::App1()
{
	mesh = nullptr;
	shader = nullptr;
}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);
	
	// Load texture.
	textureMgr->loadTexture(L"texture2", L"res/texture2.png");
	textureMgr->loadTexture(L"height2", L"res/height3.png");
	textureMgr->loadTexture(L"lava", L"res/lava.png");
	textureMgr->loadTexture(L"black", L"res/black.png");


	//size info
	int shadowmapWidth = 2048;
	int shadowmapHeight = 2048;
	int sceneWidth = 100;
	int sceneHeight = 100;

	// Create Mesh objects and shader objects
	mesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	river = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth , screenHeight , 0, 0);
	texture_Shader = new textureShader(renderer->getDevice(), hwnd);
	shader = new ManipulationShader(renderer->getDevice(), hwnd);
	bloom = new BloomShader(renderer->getDevice(), hwnd);
	lavaShader = new LavaShader(renderer->getDevice(), hwnd);
	horizontalBlurShader = new BlurHorizontalShader(renderer->getDevice(), hwnd);
	verticalBlurShader = new BlurVerticalShader(renderer->getDevice(), hwnd);

	depthShader = new DepthShader(renderer->getDevice(), hwnd);
	//shadowShader = new ShadowShader(renderer->getDevice(), hwnd);
	renderTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	cullBloomTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	downscaleTexture8x = new RenderTexture(renderer->getDevice(), screenWidth / 8, screenHeight / 8, SCREEN_NEAR, SCREEN_DEPTH);
	cubeMesh = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext(), 10);
	horizontalBlurTexture8x = new RenderTexture(renderer->getDevice(), screenWidth , screenHeight  , SCREEN_NEAR, SCREEN_DEPTH);
	verticalBlurTexture8x = new RenderTexture(renderer->getDevice(), screenWidth , screenHeight  , SCREEN_NEAR, SCREEN_DEPTH);

	
	upscaleTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);

	shadowMap = new ShadowMap(renderer->getDevice(), shadowmapWidth * 4, shadowmapHeight * 4);
	shadowMap2 = new ShadowMap(renderer->getDevice(), shadowmapWidth * 4, shadowmapHeight * 4);
	
	rotate = 2;
	bloomCutoff = 0.43;
	bloomToggle = true;


	// Confirgure directional and spot light
	light = new Light();
	light->setDiffuseColour(1, 1, 1, 1.0f);
	light->setDirection(0.f, -0.7f, -0.8f);

	light->setPosition(0, 30, 25.f);
	light->generateOrthoMatrix(200, 150, 0.1f, 100.f);

	lightSpot = new Light();
	lightSpot->setDiffuseColour(0.0f, 1.0f, 0.0f, 1.0f);
	lightSpot->setDirection(0.0f, -1.f, 0.f);
	lightSpot->setPosition(28.f, 30.f, 30.0);
	lightSpot->setAmbientColour(0., 0., 0., 1);
	lightSpot->generateOrthoMatrix(screenWidth, screenHeight, 10, 100);
	lightSpot->generateProjectionMatrix(SCREEN_NEAR, 200);
	
	lightSpotCutoff = 15;

	lightSpotPosition.x = 0;
	lightSpotPosition.y = 37;
	lightSpotPosition.z = 30;

	lightSpotDir.x = 1;
	lightSpotDir.y = -1;
	lightSpotDir.z = 0;
	
	light1Position.x = 0;
	light1Position.y = 30;
	light1Position.z = 25;

	light1Dir.x = 0;
	light1Dir.y = -.7;
	light1Dir.z = 1;

}


App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.
	if (mesh)
	{
		delete mesh;
		mesh = 0;
	}

	if (shader)
	{
		delete shader;
		shader = 0;
	}
}


bool App1::frame()
{
	bool result;
	
	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}

	//update the lights
	lightSpot->setPosition(lightSpotPosition.x, lightSpotPosition.y, lightSpotPosition.z);
	lightSpot->setDirection(lightSpotDir.x, lightSpotDir.y, lightSpotDir.z);
	
	light->setPosition(light1Position.x, light1Position.y, light1Position.z);
	light->setDirection(light1Dir.x, light1Dir.y, light1Dir.z);



	lightSpot->cutoff = (lightSpotCutoff / 57.2957795);

	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}
	rotate += 0.01;
	return true;
}

void App1::wireframeRender() {

	//regular render without render texture to allow wireframe to work
	//check commentary for initial pass for full explanation of below

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	XMMATRIX riverScaleMatrix;
	XMMATRIX riverTranslateMatrix;
	riverScaleMatrix = XMMatrixScaling(1, 1, .4);
	riverTranslateMatrix = XMMatrixTranslation(1, .6, 57);

	mesh->sendData(renderer->getDeviceContext());
	shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"texture2"), light, lightSpot, timer, camera, shadowMap->getDepthMapSRV(), shadowMap2->getDepthMapSRV(), lightSpot);
	shader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	worldMatrix = XMMatrixMultiply(worldMatrix, riverTranslateMatrix);
	worldMatrix = XMMatrixMultiply(worldMatrix, riverScaleMatrix);

	river->sendData(renderer->getDeviceContext());
	lavaShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"lava"), lightSpot, timer);
	lavaShader->render(renderer->getDeviceContext(), river->getIndexCount());

	worldMatrix = XMMatrixTranslation(0.f, 0.f, 0.f);

	XMVECTORF32 axisrotate = { 0.,	1, 0.f, 1.f };
	XMMATRIX rotateMatrix = XMMatrixRotationAxis(axisrotate, rotate);

	XMMATRIX movingMatrix = XMMatrixTranslation(30.f, 7.f, 30.f);
	worldMatrix = XMMatrixMultiply(worldMatrix, rotateMatrix);
	worldMatrix = XMMatrixMultiply(worldMatrix, movingMatrix);

	cubeMesh->sendData(renderer->getDeviceContext());
	lavaShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"lava"), lightSpot, timer);
	lavaShader->render(renderer->getDeviceContext(), cubeMesh->getIndexCount());
}

void App1::bloomInitialPass() {

	//set render texture
	renderTexture->setRenderTarget(renderer->getDeviceContext());
	renderTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.0f, 0.0f);

	//matrices
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	//matrices to move and size river
	XMMATRIX riverScaleMatrix;
	XMMATRIX riverTranslateMatrix;
	riverScaleMatrix = XMMatrixScaling(1, 1, .4);
	riverTranslateMatrix = XMMatrixTranslation(1, 1, 57);

	//render terrain through manipulation shader (does lights and shadows)
	mesh->sendData(renderer->getDeviceContext());
	shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"texture2"), light, lightSpot, timer, camera, shadowMap->getDepthMapSRV(), shadowMap2->getDepthMapSRV(), lightSpot);
	shader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	//multiplying river matrices
	worldMatrix = XMMatrixMultiply(worldMatrix, riverTranslateMatrix);
	worldMatrix = XMMatrixMultiply(worldMatrix, riverScaleMatrix);

	//rendering river with lava matrix
	river->sendData(renderer->getDeviceContext());
	lavaShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"lava"), lightSpot, timer);
	lavaShader->render(renderer->getDeviceContext(), river->getIndexCount());

	worldMatrix = XMMatrixTranslation(0.f, 0.f, 0.f);

	//matrices to move cube in the right position
	XMVECTORF32 axisrotate = { 0.,	1, 0.f, 1.f };
	XMMATRIX rotateMatrix = XMMatrixRotationAxis(axisrotate, rotate);
	XMMATRIX movingMatrix = XMMatrixTranslation(30.f, 7.f, 30.f);
	worldMatrix = XMMatrixMultiply(worldMatrix, rotateMatrix);
	worldMatrix = XMMatrixMultiply(worldMatrix, movingMatrix);

	//render cube with lava renderer
	cubeMesh->sendData(renderer->getDeviceContext());
	lavaShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"lava"), lightSpot, timer);
	lavaShader->render(renderer->getDeviceContext(), cubeMesh->getIndexCount());

	renderer->setBackBufferRenderTarget();
}

void App1::depthPass()
{
	// Set the render target to be the shadowmap.
	shadowMap->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

	//get view ortho matrices of light and world matrix of world
	light->generateViewMatrix();
	XMMATRIX lightViewMatrix = light->getViewMatrix();
	XMMATRIX lightProjectionMatrix = light->getOrthoMatrix();
	XMMATRIX worldMatrix = renderer->getWorldMatrix();

	//same interal commentary as bloom initial pass

	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	XMMATRIX riverScaleMatrix;
	XMMATRIX riverTranslateMatrix;
	riverScaleMatrix = XMMatrixScaling(1, 1, .4);
	riverTranslateMatrix = XMMatrixTranslation(1, .6, 57);

	worldMatrix = XMMatrixMultiply(worldMatrix, riverTranslateMatrix);
	worldMatrix = XMMatrixMultiply(worldMatrix, riverScaleMatrix);

	worldMatrix = XMMatrixTranslation(0.f, 0.f, 0.f);

	XMVECTORF32 axisrotate = { 0.,	1, 0.f, 1.f };
	XMMATRIX rotateMatrix = XMMatrixRotationAxis(axisrotate, rotate);

	XMMATRIX movingMatrix = XMMatrixTranslation(30.f, 7.f, 30.f);
	worldMatrix = XMMatrixMultiply(worldMatrix, rotateMatrix);
	worldMatrix = XMMatrixMultiply(worldMatrix, movingMatrix);

	//render the cube with the depth pass from the pov of the camera
	cubeMesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix, textureMgr->getTexture(L"height2"));
	depthShader->render(renderer->getDeviceContext(), cubeMesh->getIndexCount());


	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::depthSecondPass() {
	// Set the render target to be shadow map 2**.
	shadowMap2->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

	lightSpot->generateViewMatrix();

	XMMATRIX lightViewMatrix = lightSpot->getViewMatrix();
	XMMATRIX lightProjectionMatrix = lightSpot->getProjectionMatrix();	
	XMMATRIX worldMatrix = renderer->getWorldMatrix();

	//same interal commentary as bloom initial pass

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixTranslation(0.f, 7.f, 5.f);
	XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	// Render model
	
	worldMatrix = XMMatrixTranslation(0.f, 0.f, 0.f);
	XMVECTORF32 axisrotate = { 0.,	1, 0.f, 1.f };
	XMMATRIX rotateMatrix = XMMatrixRotationAxis(axisrotate, rotate);

	worldMatrix = XMMatrixMultiply(worldMatrix, rotateMatrix);

	XMMATRIX movingMatrix = XMMatrixTranslation(30.f, 7.f, 30.f);

	worldMatrix = XMMatrixMultiply(worldMatrix, movingMatrix);
	cubeMesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix, textureMgr->getTexture(L"height2"));
	depthShader->render(renderer->getDeviceContext(), cubeMesh->getIndexCount());


	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::bloomCull() {


	//bring normal rendered texture into bloom shader to make mask and render to cull bloom texture
	XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;
	renderer->setZBuffer(false);
	cullBloomTexture->setRenderTarget(renderer->getDeviceContext());
	cullBloomTexture->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 1.0f, 0.0f, 1.0f);

	worldMatrix = renderer->getWorldMatrix();
	baseViewMatrix = camera->getOrthoViewMatrix();
	orthoMatrix = cullBloomTexture->getOrthoMatrix();


	orthoMesh->sendData(renderer->getDeviceContext());
	bloom->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, renderTexture->getShaderResourceView(), bloomCutoff);
	bloom->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
	renderer->setZBuffer(true);
}

void App1::downscaleBloom(RenderTexture* output, RenderTexture* toDownscale) {
	//downscaled toDownscale texture by rendering it to the size of output
	XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;
	renderer->setZBuffer(false);
	output->setRenderTarget(renderer->getDeviceContext());
	worldMatrix = renderer->getWorldMatrix();

	baseViewMatrix = camera->getOrthoViewMatrix();
	orthoMatrix = toDownscale->getOrthoMatrix();

	orthoMesh->sendData(renderer->getDeviceContext());
	texture_Shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, toDownscale->getShaderResourceView(), NULL);
	texture_Shader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
	renderer->setZBuffer(true);
}

void App1::bloomBlurHorizontalPass(RenderTexture* output, RenderTexture* toBlur) {
	//blurs toBlur through the horizontal blur shader and renders it to the output texture
	XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;

	float screenSizeX = (float)toBlur->getTextureWidth();
	output->setRenderTarget(renderer->getDeviceContext());
	output->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 1.0f, 0.0f, 1.0f);

	worldMatrix = renderer->getWorldMatrix();
	baseViewMatrix = camera->getOrthoViewMatrix();
	orthoMatrix = output->getOrthoMatrix();

	// Render for Horizontal Blur
	renderer->setZBuffer(false);
	orthoMesh->sendData(renderer->getDeviceContext());
	horizontalBlurShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, toBlur->getShaderResourceView(), screenSizeX);
	horizontalBlurShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
	renderer->setZBuffer(true);

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
}

void App1::bloomBlurVerticalPass(RenderTexture* output, RenderTexture* toBlur) {
	//blurs toBlur through the vertical blur shader and renders it to the output texture
		XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;

		float screenSizeY = (float)toBlur->getTextureHeight();
		output->setRenderTarget(renderer->getDeviceContext());
		output->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 1.0f, 1.0f, 1.0f);

		worldMatrix = renderer->getWorldMatrix();
		baseViewMatrix = camera->getOrthoViewMatrix();
		// Get the ortho matrix from the render to texture since texture has different dimensions being that it is smaller.
		orthoMatrix = output->getOrthoMatrix();

		// Render for Vertical Blur
		renderer->setZBuffer(false);
		orthoMesh->sendData(renderer->getDeviceContext());
		verticalBlurShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, toBlur->getShaderResourceView(), screenSizeY);
		verticalBlurShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
		renderer->setZBuffer(true);

		// Reset the render target back to the original back buffer and not the render to texture anymore.
		renderer->setBackBufferRenderTarget();

}

void App1::upscaleBloom(RenderTexture* output, RenderTexture* toUpscale) {
	//upscales toUpscale texture by rendering it to the size of output

	XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;
	renderer->setZBuffer(false);
	output->setRenderTarget(renderer->getDeviceContext());
	worldMatrix = renderer->getWorldMatrix();

	baseViewMatrix = camera->getOrthoViewMatrix();
	orthoMatrix = output->getOrthoMatrix();

	orthoMesh->sendData(renderer->getDeviceContext());
	texture_Shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, toUpscale->getShaderResourceView(), NULL);
	texture_Shader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
	renderer->setZBuffer(true);
}

void App1::addTextures(RenderTexture* add1, RenderTexture* add2) {

	//adds two textures together through the texture shader and saves it in the add1 texture
	XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;
	renderer->setZBuffer(false);
	add1->setRenderTarget(renderer->getDeviceContext());
	worldMatrix = renderer->getWorldMatrix();

	baseViewMatrix = camera->getOrthoViewMatrix();
	orthoMatrix = add1->getOrthoMatrix();

	orthoMesh->sendData(renderer->getDeviceContext());

	//texture_Shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, camera->getOrthoViewMatrix(), orthoMatrix, renderTexture->getShaderResourceView(), upscaleTexture->getShaderResourceView());
	texture_Shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, add1->getShaderResourceView(), add2->getShaderResourceView());
	texture_Shader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
	renderer->setZBuffer(true);
}


bool App1::render()
{

	depthPass();

	depthSecondPass();


	//original
	bloomInitialPass();

	//mask
	bloomCull();
	//ds
	downscaleBloom(downscaleTexture8x, cullBloomTexture);
	//blur
	bloomBlurHorizontalPass(horizontalBlurTexture8x, downscaleTexture8x);
	bloomBlurVerticalPass(verticalBlurTexture8x, horizontalBlurTexture8x);

	
	//upscale 
	upscaleBloom(upscaleTexture, verticalBlurTexture8x);

	//matrices
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	XMMATRIX riverScaleMatrix;
	XMMATRIX riverTranslateMatrix;
	riverScaleMatrix = XMMatrixScaling(1, 1, .4);
	riverTranslateMatrix = XMMatrixTranslation(1, .6, 57);
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();  // ortho matrix for 2D rendering
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	// Default camera position for orthographic rendering

	// Clear the scene. (default blue colour)
	renderer->beginScene(0.03f, 0.18f, 0.03f, 1.0f);

	// Generate the view matrix based on the camera's position.
	camera->update();

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	worldMatrix = renderer->getWorldMatrix();
	viewMatrix = camera->getViewMatrix();
	projectionMatrix = renderer->getProjectionMatrix();

	
	//clear texture holding bloom
	if (bloomToggle == false) {
		upscaleTexture->clearRenderTarget(renderer->getDeviceContext(), 0, 0, 0, 0);
	}

	//render with wireframe instead
	if (wireframeToggle == true) {
		wireframeRender();
	}
	else {
		//final render to full sized ortho mesh doing the final combination of basic render and bloom
		renderer->setZBuffer(false);
		worldMatrix = renderer->getWorldMatrix();
		orthoMatrix = renderer->getOrthoMatrix();  // ortho matrix for 2D rendering
		orthoViewMatrix = camera->getOrthoViewMatrix();
		orthoMesh->sendData(renderer->getDeviceContext());
		texture_Shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, renderTexture->getShaderResourceView(), upscaleTexture->getShaderResourceView());
		texture_Shader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
		renderer->setZBuffer(true);
	}
	// Swap the buffers
	gui();
	renderer->endScene();


	return true;
}

void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);
	ImGui::Text("--------BLOOM---------");
	ImGui::Text("consider disabling to see cube shadows");
	ImGui::Checkbox("Bloom", &bloomToggle);
	ImGui::SliderFloat("Bloom cutoff", &bloomCutoff, 0, 1);
	ImGui::Text("--------SPOT LIGHT---------");
	ImGui::SliderFloat3("Spot Light Pos", &lightSpotPosition.x, 0, 100);
	ImGui::SliderFloat3("Spot Direction", &lightSpotDir.x, -1, 1);
	ImGui::SliderFloat("Spot Cutoff", &lightSpotCutoff, 0, 40);
	ImGui::Text("--------DIRECTIONAL LIGHT---------");
	ImGui::Text("easiest to only alter Z direction");
	ImGui::SliderFloat3("Directional light Direction", &light1Dir.x, -1, 1);
	ImGui::SliderFloat3("Directional light Position", &light1Position.x, -100, 100);



	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

