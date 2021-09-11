add_subdirectory(DirectXTex)
add_subdirectory(AGS)
add_subdirectory(Assimp)

set_target_properties(
	DirectXTex
	texassemble
	texconv
	texdiag
	PROPERTIES
		FOLDER ThirdParty
)