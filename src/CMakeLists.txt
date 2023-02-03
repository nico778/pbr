SET(EXAMPLE_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/pbrbasic)
SET(MAIN_CPP ${EXAMPLE_FOLDER}/pbrbasic.cpp)

set(SHADER_DIR_GLSL "../data/shaders/glsl/pbrbasic")
file(GLOB SHADERS_GLSL "${SHADER_DIR_GLSL}/*.vert" "${SHADER_DIR_GLSL}/*.frag" "${SHADER_DIR_GLSL}/*.comp" "${SHADER_DIR_GLSL}/*.geom" "${SHADER_DIR_GLSL}/*.tesc" "${SHADER_DIR_GLSL}/*.tese" "${SHADER_DIR_GLSL}/*.mesh" "${SHADER_DIR_GLSL}/*.task" "${SHADER_DIR_GLSL}/*.rgen" "${SHADER_DIR_GLSL}/*.rchit" "${SHADER_DIR_GLSL}/*.rmiss" "${SHADER_DIR_GLSL}/*.rcall")
set(SHADER_DIR_HLSL "../data/shaders/hlsl/pbrbasic")
file(GLOB SHADERS_HLSL "${SHADER_DIR_HLSL}/*.vert" "${SHADER_DIR_HLSL}/*.frag" "${SHADER_DIR_HLSL}/*.comp" "${SHADER_DIR_HLSL}/*.geom" "${SHADER_DIR_HLSL}/*.tesc" "${SHADER_DIR_HLSL}/*.tese" "${SHADER_DIR_HLSL}/*.mesh" "${SHADER_DIR_HLSL}/*.task" "${SHADER_DIR_HLSL}/*.rgen" "${SHADER_DIR_HLSL}/*.rchit" "${SHADER_DIR_HLSL}/*.rmiss" "${SHADER_DIR_HLSL}/*.rcall")
source_group("Shaders\\GLSL" FILES ${SHADERS_GLSL})
source_group("Shaders\\HLSL" FILES ${SHADERS_HLSL})

add_executable(pbrbasic ${MAIN_CPP} ${SOURCE} ${SHADERS_GLSL} ${SHADERS_HLSL})
target_link_libraries(pbrbasic base )
set_target_properties(pbrbasic PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

if(RESOURCE_INSTALL_DIR)
	install(TARGETS ${pbrbasic} DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()