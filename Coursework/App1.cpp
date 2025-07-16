// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"

App1::App1()
{

}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// meshes 
	mesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	sea = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	model = new AModel(renderer->getDevice(), "res/models/teapot.obj");
	cubeMesh = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext());
	cubePos[0] = 50.f; cubePos[1] = 5.f; cubePos[2] = 50.f; 
	orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth, screenHeight);

	// textures 
	textureMgr->loadTexture(L"brick", L"res/brick1.dds");
	textureMgr->loadTexture(L"sea", L"res/sea.jpg");
	textureMgr->loadTexture(L"sand", L"res/sand.jpg");
	textureMgr->loadTexture(L"bunny", L"res/bunny.png");
	textureMgr->loadTexture(L"height", L"res/height.png");
	textureMgr->loadTexture(L"beach_heightmap", L"res/beach_heightmap.png");

	// initial shaders
	textureShader = new TextureShader(renderer->getDevice(), hwnd);
	depthShader = new DepthShader(renderer->getDevice(), hwnd);
	shadowShader = new ShadowShader(renderer->getDevice(), hwnd);
	hBlurShader = new HorizontalBlurShader(renderer->getDevice(), hwnd);
	vBlurShader = new VerticalBlurShader(renderer->getDevice(), hwnd);
	dofShader = new DoFShader(renderer->getDevice(), hwnd);

	// renderTextures
	povRenderTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	renderTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	hBlurTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	vBlurTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	dofTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);

	// Variables for defining shadow map
	int shadowmapWidth = 4096;
	int shadowmapHeight = 4096;
	int sceneWidth = 250;
	int sceneHeight = 250;

	for (int i = 0; i < 2; i++) {
		shadowMaps[i] = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);

	}

	// Configure first directional light (orange)
	lights[0] = new Light();
	lights[0]->setAmbientColour(0.0f, 0.0f, 0.0f, 1.0f);
	lights[0]->setDiffuseColour(1.f, 0.5f, 0.0f, 1.0f);
	lightDirection[0][0] = -3.5f; lightDirection[0][1] = -3.5f; lightDirection[0][2] = 3.5f; 
	lights[0]->setDirection(lightDirection[0][0], lightDirection[0][1], lightDirection[0][2]);
	lights[0]->setPosition(0.f, 10.75f, 0.f);
	lights[0]->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);

	// Configure second directional light (blue) 
	lights[1] = new Light();
	lights[1]->setAmbientColour(0.0f, 0.0f, 0.0f, 1.0f);
	lights[1]->setDiffuseColour(0.0f, 0.0f, 1.0f, 1.0f);
	lightDirection[1][0] = 3.5f; lightDirection[1][1] = -3.5f; lightDirection[1][2] = 3.5f; 
	lights[1]->setDirection(lightDirection[1][0], lightDirection[1][1], lightDirection[1][2]);
	lights[1]->setPosition(0.f, 10.0f, 0.f);
	lights[1]->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);
}

App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.

}


bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}

	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	// update position of lights 
	for (int i = 0; i < 2; i++) {
		lights[i]->setDirection(lightDirection[i][0], lightDirection[i][1], lightDirection[i][2]);
	}

	return true;
}

bool App1::render()
{

	
	depthPass(); // depth map 
	povDepthPass(); // gets camera depth for depth of field 
	firstPass(); // geometry
	hBlurPass(); // gaussian blur pt 1
	vBlurPass(); // gaussian blur pt 2
	dofPass(); // depth of field pass
	finalPass(); // render scene 

	return true;
}

void App1::povDepthPass() // used for depth of field 
{
	// mapped to depth scene texture in dofPass()
	povRenderTexture->setRenderTarget(renderer->getDeviceContext());
	povRenderTexture->clearRenderTarget(renderer->getDeviceContext(), 0.39f, 0.58f, 0.92f, 1.0f);

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix(); // gets view matrix from camera. this is the most important step 
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	// sea
	mesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix);
	depthShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	// sand 
	worldMatrix *= XMMatrixTranslation(0.0, 0.0, -100.0);
	sea->sendData(renderer->getDeviceContext()); // handle geometry 
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix);
	depthShader->render(renderer->getDeviceContext(), sea->getIndexCount());

	// teapot
	worldMatrix = XMMatrixTranslation(50.f, 10.f, 50.f);
	XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	model->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix);
	depthShader->render(renderer->getDeviceContext(), model->getIndexCount());

	// cube
	worldMatrix = XMMatrixTranslation(cubePos[0], cubePos[1], cubePos[2]); 
	cubeMesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix);
	depthShader->render(renderer->getDeviceContext(), cubeMesh->getIndexCount());

	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();

}

void App1::depthPass()
{
	// allows shadow map to be drawn from the depth data 
	shadowMaps[0]->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

	// get view and projection matrices from the light's perspective
	lights[0]->generateViewMatrix();
	XMMATRIX lightViewMatrix = lights[0]->getViewMatrix();
	XMMATRIX lightProjectionMatrix = lights[0]->getOrthoMatrix();
	XMMATRIX worldMatrix = renderer->getWorldMatrix();

	// sea
	mesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	// sand 
	worldMatrix *= XMMatrixTranslation(0.0, 0.0, -100.0);
	sea->sendData(renderer->getDeviceContext()); // handle geometry 
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), sea->getIndexCount());
	// teapot 
	worldMatrix = XMMatrixTranslation(50.f, 10.f, 50.f);
	XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	model->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), model->getIndexCount());

	// cube
	worldMatrix = XMMatrixTranslation(cubePos[0], cubePos[1], cubePos[2]); 
	cubeMesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), cubeMesh->getIndexCount());

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();

	shadowMaps[1]->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

	lights[1]->generateViewMatrix();
	lightViewMatrix = lights[1]->getViewMatrix();
	lightProjectionMatrix = lights[1]->getOrthoMatrix();
	worldMatrix = renderer->getWorldMatrix();

	sea->sendData(renderer->getDeviceContext()); // handle geometry 
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), sea->getIndexCount());


	worldMatrix *= XMMatrixTranslation(0.0, 0.0, -100.0);
	mesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	// Render teapot model
	worldMatrix = XMMatrixTranslation(50.f, 10.f, 50.f);
	scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	model->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), model->getIndexCount());

	// Set back buffer as render target and reset viewport
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::firstPass() // normal, unblurred scene. this is where the scene is lit and shaded 
{

	
	renderTexture->setRenderTarget(renderer->getDeviceContext());
	renderTexture->clearRenderTarget(renderer->getDeviceContext(), 0.39f, 0.58f, 0.92f, 1.0f);

	camera->update();

	// get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	// sea
	mesh->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix,
		textureMgr->getTexture(L"sand"), textureMgr->getTexture(L"beach_heightmap"), 3.0f, true, shadowMaps, lights);
	shadowShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	// sand
	worldMatrix *= XMMatrixTranslation(0.0, 0.0, -100.0);
	sea->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix,
		textureMgr->getTexture(L"sea"), textureMgr->getTexture(L"height"), 3.0f, false, shadowMaps, lights);
	shadowShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	// teapot
	worldMatrix = XMMatrixTranslation(50.f, 10.f, 50.f);
	XMMATRIX scaleMatrix = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	model->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix,
		textureMgr->getTexture(L"brick"), textureMgr->getTexture(L"brick"), 50.0f, false, shadowMaps, lights);
	shadowShader->render(renderer->getDeviceContext(), model->getIndexCount());

	// cube
	worldMatrix = XMMatrixTranslation(cubePos[0], cubePos[1], cubePos[2]);
	cubeMesh->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix,
		textureMgr->getTexture(L"bunny"), textureMgr->getTexture(L"bunny"), 10.0f, false, shadowMaps, lights);
	shadowShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	renderer->setBackBufferRenderTarget();
}

void App1::hBlurPass() { // blurs scene along the width 
	

	float screenSizeX = (float)hBlurTexture->getTextureWidth();
	hBlurTexture->setRenderTarget(renderer->getDeviceContext());
	hBlurTexture->clearRenderTarget(renderer->getDeviceContext(), 0.39f, 0.58f, 0.92f, 1.0f);

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX baseViewMatrix = camera->getOrthoViewMatrix();
	XMMATRIX orthoMatrix = hBlurTexture->getOrthoMatrix();

	// Render for Horizontal Blur
	renderer->setZBuffer(false);
	orthoMesh->sendData(renderer->getDeviceContext());
	hBlurShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, renderTexture->getShaderResourceView(), screenSizeX);
	hBlurShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
	renderer->setZBuffer(true);

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
}
void App1::vBlurPass() { // blurs horizontally blurred scene along the height


	float screenSizeY = (float)vBlurTexture->getTextureHeight();
	vBlurTexture->setRenderTarget(renderer->getDeviceContext());
	vBlurTexture->clearRenderTarget(renderer->getDeviceContext(), 0.39f, 0.58f, 0.92f, 1.0f);

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX baseViewMatrix = camera->getOrthoViewMatrix();
	 // Get the ortho matrix from the render to texture since texture has different dimensions being that it is smaller.
	XMMATRIX orthoMatrix = vBlurTexture->getOrthoMatrix();

	// Render for Vertical Blur
	renderer->setZBuffer(false);
	orthoMesh->sendData(renderer->getDeviceContext());
	vBlurShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, hBlurTexture->getShaderResourceView(), screenSizeY);
	vBlurShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
	renderer->setZBuffer(true);

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
}

void App1::dofPass() { // makes it so objects that are in focus remain unblurred, and blurs increasingly outward from a circle 
	dofTexture->setRenderTarget(renderer->getDeviceContext());
	dofTexture->clearRenderTarget(renderer->getDeviceContext(), 0.5f, 0.5f, 0.5f, 1.0f);

	
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX baseViewMatrix = camera->getOrthoViewMatrix();
	XMMATRIX orthoMatrix = dofTexture->getOrthoMatrix();

	renderer->setZBuffer(false);
	orthoMesh->sendData(renderer->getDeviceContext());
	dofShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, renderTexture->getShaderResourceView(), vBlurTexture->getShaderResourceView(), povRenderTexture->getShaderResourceView(), SCREEN_NEAR, SCREEN_DEPTH, dofDistance);
	dofShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
	renderer->setZBuffer(true);
	renderer->setBackBufferRenderTarget();
}

void App1::finalPass() {
	renderer->setBackBufferRenderTarget();
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();
	XMMATRIX scaleMatrix = XMMatrixScaling(1.0f, 1.0f, 1.0f);


	// Render ortho mesh with light POV
	renderer->setZBuffer(false);
	worldMatrix = XMMatrixTranslation(1.f, 1.f, 0.f); // final val irrelevant as zbuffer disabled
	worldMatrix = XMMatrixMultiply(worldMatrix, scaleMatrix);
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();
	orthoMesh->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, dofTexture->getShaderResourceView()); // renders fully blurred scene with depth of field 
	textureShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
	renderer->setZBuffer(true);

	// Render GUI and present rendered scene to screen
	gui();
	renderer->endScene();
}

void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.f", timer->getFPS());
	ImGui::SliderFloat3("Cube Position", cubePos, -50, 50, "% .1f");
	ImGui::SliderFloat3("Orange light direction", lightDirection[0], -50, 50, "%.2f");
	ImGui::SliderFloat3("Blue light direction", lightDirection[1], -50, 50, "%.2f");

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
