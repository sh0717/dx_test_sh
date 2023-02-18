#pragma once

#include "d3dUtil.h"
#include "Effects.h"
#include "GeometryGenerator.h"
#include "InputLayouts.h"
using namespace std;
class Screen
{
public:
	Screen();
	~Screen();

	void Initialize(ID3D11Device* device);
	void DrawScreen(ID3D11DeviceContext* context, ID3D11ShaderResourceView* SRV, CXMMATRIX worldviewproj);

private:

private:

	ComPtr<ID3D11Buffer> mVB;
	ComPtr<ID3D11Buffer> mIB;



};

