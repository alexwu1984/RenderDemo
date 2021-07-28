add_compile_options("/std:c++17")
include_directories(../ThirdParty/Assimp/include)

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
	include/SamplePostProcessPass.h
	include/GBufferRenderPass.h
	include/CubeMapCross.h
	include/SkyBox.h
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
	src/SamplePostProcessPass.cpp
	src/GBufferRenderPass.cpp
	src/CubeMapCross.cpp
	src/SkyBox.cpp
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

source_group("Header Files\\ImGUI" FILES ${IMGUI_HEADERS} )
source_group("Source Files\\ImGUI" FILES ${IMGUI_SOURCE} )

add_library(DirectX12Lib
	${HEADER_FILES}
	${SOURCES}
	${IMGUI_HEADERS}
	${IMGUI_SOURCE}
	)

target_include_directories(DirectX12Lib PUBLIC include include/imgui)
target_include_directories(DirectX12Lib PUBLIC include)

target_link_libraries(DirectX12Lib LINK_PUBLIC DirectXTex)
