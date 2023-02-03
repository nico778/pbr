#version 450

layout (location = 0) in vec3 position_In;
layout (location = 1) in vec3 normal_In;

layout (binding = 0) uniform UBO 
{
	mat4 mapping;
	mat4 mesh;
	mat4 view;
	vec3 camera;
} ubo;

layout (location = 0) out vec3 worldPosition_Out;
layout (location = 1) out vec3 normal_Out;

layout(push_constant) uniform PushConsts {
	vec3 position_Mesh;
} pushConsts;

out gl_PerVertex 
{
	vec4 gl_Position;
};

void main() 
{
	vec3 locPos = vec3(ubo.mesh * vec4(position_In, 1.0));
	worldPosition_Out = locPos + pushConsts.position_Mesh;
	normal_Out = mat3(ubo.mesh) * normal_In;
	gl_Position =  ubo.mapping * ubo.view * vec4(worldPosition_Out, 1.0);
}
