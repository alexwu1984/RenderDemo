add_compile_options("/std:c++17")
include_directories(../ThirdParty/Assimp/include)
include_directories(./GLTFModel)

set(HEADER_FILES
	include/ApplicationWin32.h
	include/Camera.h
	include/ColorBuffer.h
	include/CommandContext.h
	include/CommandListManager.h
	include/CommandQueue.h
	include/Common.h
	include/D3D12Resource.h
	include/D3D12RHI.h
	include/d3dx12.h
	include/DepthBuffer.h
	include/DescriptorAllocator.h
	include/DynamicDescriptorHeap.h
	include/Game.h
	include/GpuBuffer.h
	include/LinearAllocator.h
	include/MathLib.h
	include/MeshData.h
	include/ObjLoader.h
	include/PipelineState.h
	include/PixelBuffer.h
	include/RenderWindow.h
	include/RootSignature.h
	include/SamplerManager.h
	include/Texture.h
	include/Timer.h
	include/WindowWin32.h
	include/Model.h
	include/Material.h
	include/Shader.h
	include/ShadowMap.h
	include/RenderPipelineInfo.h
	include/Geometry.h
	include/BlurFilter.h
	include/AiObjLoader.h
	include/StringUnit.h
	include/AiMeshData.h
	include/Inc.h
	include/ImguiManager.h
	include/GameInput.h
	include/SimplePostProcessPass.h
	include/GBufferRenderPass.h
	include/CubeMapCross.h
	include/SkyBox.h
	include/CubeBuffer.h
	include/GenerateMips.h
    include/PostProcessing.h
	include/BufferManager.h
	include/MotionBlur.h
	include/TemporalEffects.h
	include/UserMarkers.h
	include/GenCubePass.h
	include/SkyBoxPass.h					  
	include/PreIntegratedBRDFPass.h
	include/EnvironmentContanstBuffer.h
)

set(SOURCES
	src/ApplicationWin32.cpp
	src/Camera.cpp
	src/ColorBuffer.cpp
	src/CommandContext.cpp
	src/CommandListManager.cpp
	src/CommandQueue.cpp
	src/Common.cpp
	src/D3D12RHI.cpp
	src/DepthBuffer.cpp
	src/DescriptorAllocator.cpp
	src/DynamicDescriptorHeap.cpp
	src/Game.cpp
	src/GpuBuffer.cpp
	src/LinearAllocator.cpp
	src/MathLib.cpp
	src/MeshData.cpp
	src/ObjLoader.cpp
	src/PipelineState.cpp
	src/PixelBuffer.cpp
	src/RenderWindow.cpp
	src/RootSignature.cpp
	src/Texture.cpp
	src/Timer.cpp
	src/WindowWin32.cpp
	src/Model.cpp
	src/Material.cpp
	src/Shader.cpp
	src/ShadowMap.cpp
	src/RenderPipelineInfo.cpp
	src/Geometry.cpp
	src/BlurFilter.cpp
	src/AiObjLoader.cpp
	src/StringUnit.cpp
	src/AiMeshData.cpp
	src/ImguiManager.cpp
	src/GameInput.cpp
	src/SimplePostProcessPass.cpp
	src/GBufferRenderPass.cpp
	src/CubeMapCross.cpp
	src/SkyBox.cpp
	src/CubeBuffer.cpp
	src/GenerateMips.cpp
	src/PostProcessing.cpp
	src/BufferManager.cpp
	src/MotionBlur.cpp
	src/TemporalEffects.cpp
	src/UserMarkers.cpp
	src/GenCubePass.cpp
	src/SkyBoxPass.cpp
	src/PreIntegratedBRDFPass.cpp
	src/EnvironmentContanstBuffer.cpp
)

set( IMGUI_HEADERS
    include/imgui/imconfig.h
    include/imgui/imgui.h
    include/imgui/imgui_impl_win32.h
    include/imgui/imgui_internal.h
    include/imgui/imstb_rectpack.h
    include/imgui/imstb_textedit.h
    include/imgui/imstb_truetype.h
)


set( IMGUI_SOURCE
    src/imgui/imgui.cpp
    src/imgui/imgui_demo.cpp
    src/imgui/imgui_draw.cpp
    src/imgui/imgui_impl_win32.cpp
    src/imgui/imgui_widgets.cpp
)


set( GLTFModel_HEADERS
    include/GLTFModel/tiny_gltf.h
    include/GLTFModel/GltfModel.h
	include/GLTFModel/GltfNode.h
	include/GLTFModel/GltfMaterial.h
	include/GLTFModel/PBRMaterial.h
	include/GLTFModel/GltfMesh.h
	include/GLTFModel/GltfMeshBuffer.h
)

set( GLTFModel_SOURCE
    src/GLTFModel/tiny_gltf.cpp
    src/GLTFModel/GltfModel.cpp
	src/GLTFModel/GltfNode.cpp
	src/GLTFModel/GltfMaterial.cpp
	src/GLTFModel/PBRMaterial.cpp
	src/GLTFModel/GltfMesh.cpp
	src/GLTFModel/GltfMeshBuffer.cpp
)

source_group("Header Files\\ImGUI" FILES ${IMGUI_HEADERS} )
source_group("Source Files\\ImGUI" FILES ${IMGUI_SOURCE} )
source_group("Header Files\\GLTFModel" FILES ${GLTFModel_HEADERS} )
source_group("Source Files\\GLTFModel" FILES ${GLTFModel_SOURCE} )

add_library(DirectX12Lib
	${HEADER_FILES}
	${SOURCES}
	${IMGUI_HEADERS}
	${IMGUI_SOURCE}
	${GLTFModel_HEADERS}
	${GLTFModel_SOURCE}
	)

target_include_directories(DirectX12Lib PUBLIC include include/imgui include/GLTFModel)
target_include_directories(DirectX12Lib PUBLIC include)
target_include_directories(DirectX12Lib PUBLIC ../ThirdParty)

target_link_libraries(DirectX12Lib LINK_PUBLIC DirectXTex)
target_link_libraries(DirectX12Lib LINK_PUBLIC amd_ags)
