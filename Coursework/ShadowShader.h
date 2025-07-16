// Light shader.h
// Basic single light shader setup
#ifndef _SHADOWSHADER_H_
#define _SHADOWSHADER_H_

#include "DXF.h"

using namespace std;
using namespace DirectX;


class ShadowShader : public BaseShader
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
		XMMATRIX lightView[2];
		XMMATRIX lightProjection[2];
	};

	struct LightBufferType
	{
		XMFLOAT4 ambient[2];
		XMFLOAT4 diffuse[2];
		XMFLOAT4 direction[2];
		XMFLOAT4 position[2];
	
	};
	struct TimeBufferType
	{
		float time;
		float amplitude;
		float speed;
		float frequency;
	};
	struct HeightmapBufferType
	{
		float maxHeight;
		bool isHeightmap;
		XMFLOAT2 padding;
	};
	
public:

	ShadowShader(ID3D11Device* device, HWND hwnd);
	~ShadowShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* heightmapTexture, float maxHeight, bool isHeightmap, ShadowMap* shadowMaps[2], Light* lights[2]);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleStateShadow;
	
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* timeBuffer;
	ID3D11Buffer* heightmapBuffer;
};

#endif
