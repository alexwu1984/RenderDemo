cmake_minimum_required(VERSION 3.10)

project(DirectX12Tutorial)

if(WIN32)
	add_compile_definitions(_UNICODE UNICODE)
endif()

include_directories(${PROJECT_SOURCE_DIR}/ThirdParty)

add_subdirectory(DirectX12Lib)
add_subdirectory(ThirdParty/DirectXTex)
add_subdirectory(ThirdParty/BULLET)
add_subdirectory(ThirdParty/glm)
add_compile_options("/std:c++17")

add_executable(Tutorial01 Tutorial01/tutorial1.cpp)
target_link_libraries(Tutorial01 LINK_PUBLIC DirectX12Lib)
set_target_properties(Tutorial01 PROPERTIES
	PROJECT_LABEL "Tutorial01 - Empty Project")

add_executable(Tutorial02 Tutorial02/tutorial2.cpp)
target_link_libraries(Tutorial02 LINK_PUBLIC DirectX12Lib)
set_target_properties(Tutorial02 PROPERTIES
	PROJECT_LABEL "Tutorial02 - Triangle")

add_subdirectory(Tutorial03)

add_executable(Tutorial04 Tutorial04/tutorial4.cpp)
target_link_libraries(Tutorial04 LINK_PUBLIC DirectX12Lib)
set_target_properties(Tutorial04 PROPERTIES
	PROJECT_LABEL "Tutorial04 - Texture")

add_executable(Tutorial05 Tutorial05/tutorial5.cpp)
target_link_libraries(Tutorial05 LINK_PUBLIC DirectX12Lib)
set_target_properties(Tutorial05 PROPERTIES
	PROJECT_LABEL "Tutorial05 - Multiple vertex stream")

add_executable(Tutorial06 Tutorial06/tutorial6.cpp)
target_link_libraries(Tutorial06 LINK_PUBLIC DirectX12Lib ../ThirdParty/Assimp/lib/x64/assimp-vc140-mt)
set_target_properties(Tutorial06 PROPERTIES
	PROJECT_LABEL "Tutorial06 - Mesh Loader")

add_executable(Tutorial07 Tutorial07/tutorial7.cpp)
target_link_libraries(Tutorial07 LINK_PUBLIC DirectX12Lib ../ThirdParty/Assimp/lib/x64/assimp-vc140-mt)
set_target_properties(Tutorial07 PROPERTIES
	PROJECT_LABEL "Tutorial07 - Spherical Harmonic")

add_executable(Tutorial08 Tutorial08/tutorial8.cpp)
target_link_libraries(Tutorial08 LINK_PUBLIC DirectX12Lib ../ThirdParty/Assimp/lib/x64/assimp-vc140-mt)
set_target_properties(Tutorial08 PROPERTIES
	PROJECT_LABEL "Tutorial08 - Reflective shadow map")

add_executable(Tutorial09 Tutorial09/tutorial9.cpp 
                          Tutorial09/normalrenderpass.cpp
						  Tutorial09/normalrenderpass.h
						  Tutorial09/screenquadrenderpass.cpp
						  Tutorial09/screenquadrenderpass.h
						  Tutorial09/ssaopass.cpp
						  Tutorial09/ssaopass.h
						  Tutorial09/ssdopass.cpp
						  Tutorial09/ssdopass.h
)
target_link_libraries(Tutorial09 LINK_PUBLIC DirectX12Lib ../ThirdParty/Assimp/lib/x64/assimp-vc140-mt)
set_target_properties(Tutorial09 PROPERTIES
	PROJECT_LABEL "Tutorial09 - Approximating Dynamic Global Illumination in Image Space")
	
add_executable(Tutorial10 Tutorial10/tutorial10.cpp 
						  Tutorial10/hbaopass.cpp
						  Tutorial10/hbaopass.h
						  Tutorial10/screenquadrenderpass.cpp
						  Tutorial10/screenquadrenderpass.h
)
target_link_libraries(Tutorial10 LINK_PUBLIC DirectX12Lib 
                      glm_static
                      ../ThirdParty/Assimp/lib/x64/assimp-vc140-mt)
set_target_properties(Tutorial10 PROPERTIES
	PROJECT_LABEL "Tutorial10 - Image Space Horizon Based Ambient Occlusion")
	
add_subdirectory(SHSampler)

add_executable(Tutorial11 Tutorial11/tutorial11.cpp 
						  Tutorial11/screenquadrenderpass.cpp
						  Tutorial11/screenquadrenderpass.h
						  Tutorial11/ScreenSpaceRayTracingPass.cpp
						  Tutorial11/ScreenSpaceRayTracingPass.h
						  Tutorial11/baserenderpass.cpp
						  Tutorial11/baserenderpass.h
)
target_link_libraries(Tutorial11 LINK_PUBLIC DirectX12Lib 
                      ../ThirdParty/Assimp/lib/x64/assimp-vc140-mt)
set_target_properties(Tutorial11 PROPERTIES
	PROJECT_LABEL "Tutorial11 - Efficient GPU Screen-Space Ray Tracing")
	

add_executable(Tutorial12 Tutorial12/tutorial12.cpp 
						  Tutorial12/pbrrenderpass.cpp
						  Tutorial12/pbrrenderpass.h
						  Tutorial12/Show2DTexturePass.h
						  Tutorial12/Show2DTexturePass.cpp
						  Tutorial12/SkyBoxPass.h
						  Tutorial12/SkyBoxPass.cpp
)
target_link_libraries(Tutorial12 LINK_PUBLIC DirectX12Lib 
                      ../ThirdParty/Assimp/lib/x64/assimp-vc140-mt)
set_target_properties(Tutorial12 PROPERTIES
	PROJECT_LABEL "Tutorial12 - PBR")