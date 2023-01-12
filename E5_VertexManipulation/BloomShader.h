#pragma once
#pragma once

#include "DXF.h"

using namespace std;
using namespace DirectX;

class BloomShader : public BaseShader
{
private:
	struct BloomBufferType
	{
		float cutoff;
		XMFLOAT3 padding;
	};
public:
	BloomShader(ID3D11Device* device, HWND hwnd);
	~BloomShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, float cutoff);

private:
	void initShader(const wchar_t* cs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleHeightState;
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* lightBufferSpot;
	ID3D11Buffer* timeBuffer;
	ID3D11Buffer* cameraBuffer;
	ID3D11Buffer* bloomBuffer;
	float passedTime = 0;
};

