#pragma once

#include "DXF.h"

using namespace std;
using namespace DirectX;

class HeightmapShader : public BaseShader
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
		XMMATRIX lightView;
		XMMATRIX lightProjection;
	};
	struct LightBufferType
	{
		XMFLOAT4 ambient[2];
		XMFLOAT4 diffuse[2];
		XMFLOAT3 direction[2];
		float padding[2];
		XMFLOAT3 position[2];
		float padding2[2];

	};

	struct HeightmapBufferType
	{
		float maxHeight;
		XMFLOAT3 padding;
	};

public:
	HeightmapShader(ID3D11Device* device, HWND hwnd);
	~HeightmapShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* heightmapTexture, float maxHeight);



private:
	void initShader(const wchar_t* cs, const wchar_t* ps);


private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11Buffer* lightBuffer;
	ID3D11SamplerState* sampleStateShadow;
	ID3D11Buffer* heightmapBuffer;
};