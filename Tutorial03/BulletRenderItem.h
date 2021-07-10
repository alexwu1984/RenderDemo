#pragma once
#include "GpuBuffer.h"
#include "Geometry.h"
#include "Camera.h"
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <chrono>

struct BulletRenderItem
{
	BulletRenderItem() {}
	~BulletRenderItem();

	void Init();
	void UpdateState(DirectX::BoundingFrustum& Frustum,const FCamera& cam);

	struct BasePassInfoWrapper
	{
		struct
		{
			FMatrix projectionMatrix;
			FMatrix modelMatrix;
			FMatrix viewMatrix;
		} BasePassInfo;

		FConstBuffer BasePassConstBuf;
		D3D12_CPU_DESCRIPTOR_HANDLE BasePassCpuHandle;
	};

	BasePassInfoWrapper PassInfo;
	int CBVRootIndex = 0;
	FGeometry Geo;
	bool IsDelete = false;

	DirectX::BoundingBox Box;

	enum State
	{
		UnKnown,
		In,
		Out,
	};
	State RunState = UnKnown;
	std::chrono::high_resolution_clock::time_point tStart, tEnd;
};