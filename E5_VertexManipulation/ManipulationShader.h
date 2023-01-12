#pragma once

#include "DXF.h"

using namespace std;
using namespace DirectX;

class ManipulationShader : public BaseShader
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
		XMMATRIX lightView;
		XMMATRIX lightProjection;
		XMMATRIX lightView2;
		XMMATRIX lightProjection2;
	};
	struct LightBufferType
	{
		XMFLOAT4 diffuse;
		XMFLOAT3 direction;
		float padding;
		XMFLOAT4 ambient2;
		XMFLOAT4 diffuse2;
		XMFLOAT3 direction2;
		float padding2;

	};

	struct CameraBufferType {
		XMFLOAT3 cameraPos;
		float padding;
	};

	struct LightSpotBufferType
	{
		XMFLOAT4 diffuse;
		XMFLOAT3 direction;
		float padding;
		XMFLOAT4 ambient;
		XMFLOAT3 lightPos;
		float cutoff;

	};

	struct TimeBufferType
	{
		//make time delta time passed
		float time;
		XMFLOAT3 padding;
	};
public:
	ManipulationShader(ID3D11Device* device, HWND hwnd);
	~ManipulationShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* texture, Light* light, Light* lightSpot, Timer* timer, Camera* camera, ID3D11ShaderResourceView* depthMap, ID3D11ShaderResourceView* depthMap2, Light* light2);

private:
	void initShader(const wchar_t* cs, const wchar_t* ps);

private:
	ID3D11Buffer * matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleHeightState;
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* lightBufferSpot;
	ID3D11Buffer* timeBuffer;
	ID3D11Buffer* cameraBuffer;
	
	ID3D11SamplerState* sampleStateShadow;
	float passedTime = 0;
};

