#pragma once
#include "d3dUtil.h"


class RenderStates
{
public:
	static void Initialize(ID3D11Device* device);
	static void Shutdown();

	static ID3D11RasterizerState* WireframeRS;
	static ID3D11RasterizerState* NoCullRS;
	static ID3D11RasterizerState* CullClockwiseRS;

	static ID3D11BlendState* AlphaToCoverageBS;
	static ID3D11BlendState* TransparentBS;
	static ID3D11BlendState* NoRenderWriteBS;



	static ID3D11DepthStencilState* MarkMirrorDSS;
	static ID3D11DepthStencilState* DrawReflectionDSS;
	static ID3D11DepthStencilState* LessEqualDSS;

	static void WireOn(ID3D11DeviceContext* context) { context->RSSetState(RenderStates::WireframeRS); }
	static void WireOff(ID3D11DeviceContext* context) { context->RSSetState(nullptr); }


	static void NoCullOn(ID3D11DeviceContext* context) { context->RSSetState(RenderStates::NoCullRS); };
	static void NoCullOff(ID3D11DeviceContext* context) { context->RSSetState(nullptr); };

	static void TransparentOn(ID3D11DeviceContext* context);
	static void TransparentOff(ID3D11DeviceContext* context);

	static void MarkMirrorOn(ID3D11DeviceContext* context);
	static void MarkMirrorOff(ID3D11DeviceContext* context);


	static void DrawReflectOn(ID3D11DeviceContext* context);
	static void DrawReflectOff(ID3D11DeviceContext* context);

	static void AlphaToCoverageOn(ID3D11DeviceContext* context);
	static void AlphaToCoverageOff(ID3D11DeviceContext* context);


};
