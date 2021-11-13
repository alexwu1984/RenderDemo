#pragma once
#include "GLTFModel/PBRMaterial.h"


template<class T>
struct TGltfResourceTraits
{
};

template<>
struct TGltfResourceTraits<FGltfMaterial>
{
	typedef FPBRMaterial TConcreteType;
};

template<typename TRHIType>
static FORCEINLINE typename TGltfResourceTraits<TRHIType>::TConcreteType* GLTFResourceCast(TRHIType* Resource)
{
	return static_cast<typename TGltfResourceTraits<TRHIType>::TConcreteType*>(Resource);
}