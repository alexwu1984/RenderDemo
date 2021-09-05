project(Tutorial03)
set(LIBRARY_NAME "Tutorial03")
add_compile_options("/std:c++17")

set(SOURCE_TOP_DIR ${PROJECT_SOURCE_DIR}/../)

include_directories(${SOURCE_TOP_DIR}/ThirdParty/BULLET)
include_directories(${SOURCE_TOP_DIR}/DirectX12Lib)

set(SOURCES
	tutorial3.cpp
	BulletPhysics.cpp
	BulletPhysics.h
	BulletRenderItem.cpp
	BulletRenderItem.h
)


add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(Tutorial03 LINK_PUBLIC DirectX12Lib Bullet ${SOURCE_TOP_DIR}/ThirdParty/Assimp/lib/x64/assimp-vc140-mt.lib)
set_target_properties(Tutorial03 PROPERTIES
	PROJECT_LABEL "Tutorial03 - Bullet")
