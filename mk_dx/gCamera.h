#pragma once
#include "d3dUtil.h"


class gCamera
{
public:
	gCamera();
	~gCamera();
public:
	XMVECTOR GetPositionVector()const;
	XMFLOAT3 GetPosition()const;
	void SetPosition(float x, float y, float z);
	void SetPosition(const XMFLOAT3& v);


	XMVECTOR GetRightVector()const;
	XMFLOAT3 GetRight()const;


	XMVECTOR GetUpVector()const;
	XMFLOAT3 GetUp()const;


	XMVECTOR GetLookVector()const;
	XMFLOAT3 GetLook()const;


	void LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp);
	void LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up);
	 

	void InitLens(float fovY, float aspect, float nearZ, float farZ);



	XMMATRIX view() const;
	XMMATRIX proj() const;
	XMMATRIX viewProj() const;


	void Update();


	void Strafe(float d);
	void Walk(float d);

	// Rotate the camera.
	void Pitch(float angle);
	void RotateY(float angle);




private:
	
	XMFLOAT3 mPosition;
	XMFLOAT3 mRight;
	XMFLOAT3 mUp;
	XMFLOAT3 mLook;

	float mNearZ;
	float mFarZ;
	float mAspect;
	float mFovY;

	float mNearWindowHeight;
	float mFarWindowHeight;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;
};

