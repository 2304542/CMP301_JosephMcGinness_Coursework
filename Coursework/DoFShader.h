#include "DXF.h"

using namespace std;
using namespace DirectX;
class DoFShader : public BaseShader
{
	//struct passed into shader
	struct DepthBufferType
	{
		float range;
		float nearVal;
		float farVal;
		float offsetVal;
	};
public:
	DoFShader(ID3D11Device* device, HWND hwnd);
	~DoFShader();

	// 
	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* normalSceneTex, ID3D11ShaderResourceView* blurSceneTex, ID3D11ShaderResourceView* depthSceneTex, float nearV, float farV, float range);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:

	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* depthBuffer;
	ID3D11SamplerState* sampleState;
};
