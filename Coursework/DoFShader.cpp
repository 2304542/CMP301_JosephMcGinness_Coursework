#include "DoFShader.h"

DoFShader::DoFShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"dof_vs.cso", L"dof_ps.cso");
}

DoFShader::~DoFShader()
{
	// Release the sampler state.
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}

	// Release the matrix buffer.
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}

	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	// Release the depth buffer.
	if (depthBuffer)
	{
		depthBuffer->Release();
		depthBuffer = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void DoFShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);
	//helper functions to streamline buffer/sampler creation
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC depthBufferDesc;
	


	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	depthBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	depthBufferDesc.ByteWidth = sizeof(DepthBufferType);
	depthBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	depthBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	depthBufferDesc.MiscFlags = 0;
	depthBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&depthBufferDesc, NULL, &depthBuffer);


}

void DoFShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* normalSceneTex, ID3D11ShaderResourceView* blurSceneTex, ID3D11ShaderResourceView* depthSceneTex, float nearV, float farV, float range)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	XMMATRIX tworld, tview, tproj;

	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);

	// Send matrix data to the vertex shader.
	deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	// Send depth data to pixel shader
	DepthBufferType* depthPtr;
	deviceContext->Map(depthBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	depthPtr = (DepthBufferType*)mappedResource.pData;
	depthPtr->range = range;
	depthPtr->nearVal = nearV;
	depthPtr->farVal = farV;
	//depthPtr->offsetVal = offset;
	deviceContext->Unmap(depthBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &depthBuffer);

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &normalSceneTex); // firstPass
	deviceContext->PSSetShaderResources(1, 1, &blurSceneTex); // vBlurPass (following hBlurPass)
	deviceContext->PSSetShaderResources(2, 1, &depthSceneTex); // povDepthPass
	deviceContext->PSSetSamplers(0, 1, &sampleState);
}