#include "BulletRenderItem.h"


void ConvertFMatrix(const FMatrix& in, DirectX::XMMATRIX& out)
{
	out = DirectX::XMMATRIX(in._00, in._01, in._02, in._03,
		in._10, in._11, in._12, in._13,
		in._20, in._21, in._22, in._23,
		in._30, in._31, in._32, in._33);
}

BulletRenderItem::~BulletRenderItem()
{

}


void BulletRenderItem::Init()
{
	PassInfo.BasePassConstBuf.CreateUpload(L"BasePassInfo", sizeof(PassInfo.BasePassInfo));
	PassInfo.BasePassCpuHandle = PassInfo.BasePassConstBuf.CreateConstantBufferView(0, sizeof(PassInfo.BasePassInfo));
}

void BulletRenderItem::UpdateState(DirectX::BoundingFrustum& Frustum, const FCamera& cam)
{
	FMatrix InvWorld = Geo.GetModelMatrix().Inverse();
	FMatrix ViewToLocal = cam.GetViewMatrix().Inverse();

	DirectX::XMMATRIX viewOutMat;
	ConvertFMatrix(ViewToLocal * InvWorld, viewOutMat);

	// Transform the camera frustum from view space to the object's local space.
	DirectX::BoundingFrustum localSpaceFrustum;
	Frustum.Transform(localSpaceFrustum, viewOutMat);

	if (localSpaceFrustum.Contains(Box) != DirectX::DISJOINT)
	{
		if (RunState != State::In)
		{
			RunState = State::In;
			tStart = std::chrono::high_resolution_clock::now();
		}
	}
	else
	{
		if (RunState == State::In)
		{
			RunState = State::Out;
			IsDelete = true;
		}
	}

	if (RunState == State::In)
	{
		tEnd = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::milli>(tEnd - tStart).count();
		if (time > 3000)
		{
			IsDelete = true;
		}
	}
}
