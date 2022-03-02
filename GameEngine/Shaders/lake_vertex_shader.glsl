#version 400

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normals;
layout (location = 2) in vec2 texCoord;

out vec2 textureCoord;
out vec3 norm;
out vec3 fragPos;

uniform mat4 MVP;
uniform mat4 model;

void main()
{
	float A, W, t;

	t = 1000.0f;

	// amplitude of the wave
	A = 5.0f;

	//  wave frequency, (w = 2 pi / L), where L = wave length (distance between 2 "mountain" tops)
	W = 1.0f;

	// Wave direction (depends on x and z)
	vec2 D = vec2(1.0f, 1.0f);

	vec3 tmpPos = pos;

	tmpPos.y = 2 * A * pow((sin(dot(D, vec2(tmpPos.x, tmpPos.z))*W+t)+1)/2, 2);

	textureCoord = texCoord;
	fragPos = vec3(model * vec4(tmpPos, 1.0f));
	norm = mat3(transpose(inverse(model))) * normals;
	gl_Position = MVP * vec4(tmpPos, 1.0f);
}