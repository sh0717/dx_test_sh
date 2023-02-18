#pragma once

#include "d3dUtil.h"
#include "gCamera.h"
class ShadowMap
{
public:
	ShadowMap(ID3D11Device* device, UINT width, UINT height);
	~ShadowMap();

	ID3D11ShaderResourceView* DepthMapSRV();
	void BindDsv_SetNullRenderTarget(ID3D11DeviceContext* context);

private:
	ShadowMap(const ShadowMap& rhs);
	ShadowMap& operator=(const ShadowMap& rhs);

private:
	UINT mWidth;
	UINT mHeight;

	ComPtr<ID3D11ShaderResourceView> mDepthMapSRV;
	ComPtr<ID3D11DepthStencilView> mDepthMapDSV;

	//ID3D11ShaderResourceView* mDepthMapSRV;
	//ID3D11DepthStencilView* mDepthMapDSV;

	D3D11_VIEWPORT mViewport;

};

