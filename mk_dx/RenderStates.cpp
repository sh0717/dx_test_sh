#include "RenderStates.h"

ID3D11RasterizerState* RenderStates::WireframeRS = 0;
ID3D11RasterizerState* RenderStates::NoCullRS = 0;
ID3D11RasterizerState* RenderStates::CullClockwiseRS = 0;


ID3D11BlendState* RenderStates::AlphaToCoverageBS = 0;
ID3D11BlendState* RenderStates::TransparentBS = 0;
ID3D11BlendState* RenderStates::NoRenderWriteBS = nullptr;

ID3D11DepthStencilState* RenderStates::MarkMirrorDSS = nullptr;
ID3D11DepthStencilState* RenderStates::DrawReflectionDSS = nullptr;


ID3D11DepthStencilState* RenderStates::LessEqualDSS = nullptr;


void RenderStates::Initialize(ID3D11Device* device) {

	D3D11_RASTERIZER_DESC wireframeDesc;
	D3D11_RASTERIZER_DESC noCullDesc;

	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_BACK;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;

	HR(device->CreateRasterizerState(&wireframeDesc, &WireframeRS));

	ZeroMemory(&noCullDesc, sizeof(D3D11_RASTERIZER_DESC));

	noCullDesc.FillMode = D3D11_FILL_SOLID;
	noCullDesc.CullMode = D3D11_CULL_NONE;
	noCullDesc.FrontCounterClockwise = false;
	noCullDesc.DepthClipEnable = true;

	HR(device->CreateRasterizerState(&noCullDesc, &NoCullRS));


	D3D11_RASTERIZER_DESC cullClockwiseDesc;
	ZeroMemory(&cullClockwiseDesc, sizeof(D3D11_RASTERIZER_DESC));
	cullClockwiseDesc.FillMode = D3D11_FILL_SOLID;
	cullClockwiseDesc.CullMode = D3D11_CULL_BACK;
	cullClockwiseDesc.FrontCounterClockwise = true;
	cullClockwiseDesc.DepthClipEnable = true;

	HR(device->CreateRasterizerState(&cullClockwiseDesc, &CullClockwiseRS));



	D3D11_BLEND_DESC alphaToCoverageDesc = { 0 };
	alphaToCoverageDesc.AlphaToCoverageEnable = true;
	alphaToCoverageDesc.IndependentBlendEnable = false;

	alphaToCoverageDesc.RenderTarget[0].BlendEnable = false;
	alphaToCoverageDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	HR(device->CreateBlendState(&alphaToCoverageDesc, &AlphaToCoverageBS));

	D3D11_BLEND_DESC transparentDesc = { 0 };
	transparentDesc.AlphaToCoverageEnable = false;
	transparentDesc.IndependentBlendEnable = false;

	transparentDesc.RenderTarget[0].BlendEnable = true;
	transparentDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	transparentDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	transparentDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	transparentDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HR(device->CreateBlendState(&transparentDesc, &TransparentBS));


	D3D11_BLEND_DESC NoRendertargetDesc = { 0 };
	NoRendertargetDesc.AlphaToCoverageEnable = false;
	NoRendertargetDesc.IndependentBlendEnable = false;
	NoRendertargetDesc.RenderTarget[0].BlendEnable = false;
	NoRendertargetDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	NoRendertargetDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	NoRendertargetDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	NoRendertargetDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	NoRendertargetDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	NoRendertargetDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	NoRendertargetDesc.RenderTarget[0].RenderTargetWriteMask = 0;

	HR(device->CreateBlendState(&NoRendertargetDesc, &NoRenderWriteBS));

	//////depth stencil 

	D3D11_DEPTH_STENCIL_DESC mirrorDesc;
	mirrorDesc.DepthEnable = true;
	mirrorDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	mirrorDesc.DepthFunc = D3D11_COMPARISON_LESS;

	mirrorDesc.StencilEnable = true;
	mirrorDesc.StencilReadMask = 0xff;
	mirrorDesc.StencilWriteMask = 0xff;

	mirrorDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	mirrorDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	mirrorDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	mirrorDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	HR(device->CreateDepthStencilState(&mirrorDesc, &MarkMirrorDSS));


	D3D11_DEPTH_STENCIL_DESC drawReflectionDesc;

	drawReflectionDesc.DepthEnable = true;
	drawReflectionDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	drawReflectionDesc.DepthFunc = D3D11_COMPARISON_LESS;
	drawReflectionDesc.StencilEnable = true;
	drawReflectionDesc.StencilReadMask = 0xff;
	drawReflectionDesc.StencilWriteMask = 0xff;

	drawReflectionDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	// We are not rendering backfacing polygons, so these settings do not matter.
	drawReflectionDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	HR(device->CreateDepthStencilState(&drawReflectionDesc, &DrawReflectionDSS));


	D3D11_DEPTH_STENCIL_DESC lessEqualDesc;
	lessEqualDesc.DepthEnable = true;
	lessEqualDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	lessEqualDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	lessEqualDesc.StencilEnable = false;

	HR(device->CreateDepthStencilState(&lessEqualDesc, &LessEqualDSS));


}


void RenderStates::Shutdown() {
	ReleaseCOM(WireframeRS);
	ReleaseCOM(NoCullRS);
	ReleaseCOM(AlphaToCoverageBS);
	ReleaseCOM(TransparentBS);
	ReleaseCOM(CullClockwiseRS);
	ReleaseCOM(MarkMirrorDSS);
	ReleaseCOM(DrawReflectionDSS);
	ReleaseCOM(LessEqualDSS);
}


void RenderStates::TransparentOn(ID3D11DeviceContext* context) {
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	context->OMSetBlendState(RenderStates::TransparentBS, blendFactor, 0xffffffff);
}
void RenderStates::TransparentOff(ID3D11DeviceContext* context) {
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	context->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
}


void RenderStates::MarkMirrorOn(ID3D11DeviceContext* context) {
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	context->OMSetBlendState(NoRenderWriteBS, blendFactor, 0xffffffff);
	context->OMSetDepthStencilState(MarkMirrorDSS, 1);
}

void RenderStates::MarkMirrorOff(ID3D11DeviceContext* context) {
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	context->OMSetBlendState(0, blendFactor, 0xffffffff);
	context->OMSetDepthStencilState(0, 0);
}

void RenderStates::DrawReflectOn(ID3D11DeviceContext* context) {
	context->RSSetState(CullClockwiseRS);
	context->OMSetDepthStencilState(DrawReflectionDSS, 1);
}

void RenderStates::DrawReflectOff(ID3D11DeviceContext* context) {
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);
}


void RenderStates::AlphaToCoverageOn(ID3D11DeviceContext* context) {
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	context->OMSetBlendState(AlphaToCoverageBS, blendFactor, 0xffffffff);

}
void RenderStates::AlphaToCoverageOff(ID3D11DeviceContext* context) {
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	context->OMSetBlendState(0, blendFactor, 0xffffffff);

}