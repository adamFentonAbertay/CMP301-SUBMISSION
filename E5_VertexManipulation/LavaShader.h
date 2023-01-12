#pragma once

#include "BaseShader.h"
#include "DXF.h"

using namespace std;
using namespace DirectX;

class LavaShader : public BaseShader
{
private:
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
	LavaShader(ID3D11Device* device, HWND hwnd);
	~LavaShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, Light* lightSpot, Timer* timer);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);
	
private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11Buffer* lightBufferSpot;
	ID3D11Buffer* timeBuffer;
	float passedTime = 0;
};

