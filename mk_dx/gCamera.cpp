#include "gCamera.h"




gCamera::gCamera()
	:mPosition(0.0f, 0.0f, 5.0f), mRight(1.0f, 0.0f, 0.0f), mUp(0.0f, 1.0f, 0.0f), mLook(0.0f, 0.0f, 1.0f)
{
	InitLens(0.25f * MathHelper::Pi, 1.0f, 1.0f, 1000.0f);
}

gCamera::~gCamera() {}


XMVECTOR gCamera::GetPositionVector()const {
	return XMLoadFloat3(&mPosition);
}

XMFLOAT3 gCamera::GetPosition()const {
	return mPosition;
}

void gCamera::SetPosition(float x, float y, float z) {
	mPosition = XMFLOAT3(x, y, z);
}
void gCamera::SetPosition(const XMFLOAT3& v) {
	mPosition = v;
}

XMVECTOR gCamera::GetRightVector()const {
	return XMLoadFloat3(&mRight);
}

XMFLOAT3 gCamera::GetRight()const {
	return mRight;
}

XMVECTOR gCamera::GetLookVector() const {
	return XMLoadFloat3(&mLook);
}

XMFLOAT3 gCamera::GetLook() const {
	return mLook;
}

XMVECTOR gCamera::GetUpVector()const {
	return XMLoadFloat3(&mUp);
}
XMFLOAT3 gCamera::GetUp()const {
	return mUp;
}

void gCamera::InitLens(float fovY, float aspect, float zn, float zf) {
	mFovY = fovY;
	mAspect = aspect;
	mNearZ = zn;
	mFarZ = zf;

	XMMATRIX P = XMMatrixPerspectiveFovLH(mFovY, mAspect, mNearZ, mFarZ);
	XMStoreFloat4x4(&mProj, P);
}


XMMATRIX gCamera::view()const {
	return XMLoadFloat4x4(&mView);
}

XMMATRIX gCamera::proj()const {
	return XMLoadFloat4x4(&mProj);
}

XMMATRIX gCamera::viewProj() const {
	return XMLoadFloat4x4(&mView) * XMLoadFloat4x4(&mProj);
}

void gCamera::LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp)
{
	XMVECTOR L = XMVector3Normalize(XMVectorSubtract(target, pos));
	XMVECTOR R = XMVector3Normalize(XMVector3Cross(worldUp, L));
	XMVECTOR U = XMVector3Cross(L, R);

	XMStoreFloat3(&mPosition, pos);
	XMStoreFloat3(&mLook, L);
	XMStoreFloat3(&mRight, R);
	XMStoreFloat3(&mUp, U);

	Update();
}

void gCamera::LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up)
{
	XMVECTOR P = XMLoadFloat3(&pos);
	XMVECTOR T = XMLoadFloat3(&target);
	XMVECTOR U = XMLoadFloat3(&up);

	LookAt(P, T, U);
}


void gCamera::Update() {

	XMVECTOR R = XMLoadFloat3(&mRight);
	XMVECTOR U = XMLoadFloat3(&mUp);
	XMVECTOR L = XMLoadFloat3(&mLook);
	XMVECTOR P = XMLoadFloat3(&mPosition);

	// Keep camera's axes orthogonal to each other and of unit length.
	L = XMVector3Normalize(L);
	U = XMVector3Normalize(XMVector3Cross(L, R));

	// U, L already ortho-normal, so no need to normalize cross product.
	R = XMVector3Cross(U, L);

	// Fill in the view matrix entries.
	float x = -XMVectorGetX(XMVector3Dot(P, R));
	float y = -XMVectorGetX(XMVector3Dot(P, U));
	float z = -XMVectorGetX(XMVector3Dot(P, L));

	XMStoreFloat3(&mRight, R);
	XMStoreFloat3(&mUp, U);
	XMStoreFloat3(&mLook, L);

	mView(0, 0) = mRight.x;
	mView(1, 0) = mRight.y;
	mView(2, 0) = mRight.z;
	mView(3, 0) = x;

	mView(0, 1) = mUp.x;
	mView(1, 1) = mUp.y;
	mView(2, 1) = mUp.z;
	mView(3, 1) = y;

	mView(0, 2) = mLook.x;
	mView(1, 2) = mLook.y;
	mView(2, 2) = mLook.z;
	mView(3, 2) = z;

	mView(0, 3) = 0.0f;
	mView(1, 3) = 0.0f;
	mView(2, 3) = 0.0f;
	mView(3, 3) = 1.0f;

	return;
}

void gCamera::Strafe(float d)
{
	// mPosition += d*mRight
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR r = XMLoadFloat3(&mRight);
	XMVECTOR p = XMLoadFloat3(&mPosition);
	XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, r, p));
}

void gCamera::Walk(float d)
{
	// mPosition += d*mLook
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR l = XMLoadFloat3(&mLook);
	XMVECTOR p = XMLoadFloat3(&mPosition);
	XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, l, p));
}


void gCamera::Pitch(float angle)
{
	// Rotate up and look vector about the right vector.

	XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&mRight), angle);

	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
	XMStoreFloat3(&mLook, XMVector3TransformNormal(XMLoadFloat3(&mLook), R));
}

void gCamera::RotateY(float angle)
{
	// Rotate the basis vectors about the world y-axis.

	XMMATRIX R = XMMatrixRotationY(angle);

	XMStoreFloat3(&mRight, XMVector3TransformNormal(XMLoadFloat3(&mRight), R));
	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
	XMStoreFloat3(&mLook, XMVector3TransformNormal(XMLoadFloat3(&mLook), R));
}