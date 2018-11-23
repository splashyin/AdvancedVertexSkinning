#version 430 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in ivec4 BoneIDs;
layout(location = 4) in vec4 Weights;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 gBones[100];
uniform mat2x4 dqs[100];

mat4x4 DQtoMat(vec4 real, vec4 dual) {
	mat4x4 m;
	float len2 = dot(real, real);
	float w = real.w, x = real.x, y = real.y, z = real.z;
	float t0 = dual.w, t1 = dual.x, t2 = dual.y, t3 = dual.z;

	m[0][0] = w * w + x * x - y * y - z * z;
	m[1][0] = 2 * x * y - 2 * w * z;
	m[2][0] = 2 * x * z + 2 * w * y;
	m[0][1] = 2 * x * y + 2 * w * z;
	m[1][1] = w * w + y * y - x * x - z * z;
	m[2][1] = 2 * y * z - 2 * w * x;
	m[0][2] = 2 * x * z - 2 * w * y;
	m[1][2] = 2 * y * z + 2 * w * x;
	m[2][2] = w * w + z * z - x * x - y * y;

	m[3][0] = -2 * t0 * x + 2 * w * t1 - 2 * t2 * z + 2 * y * t3;
	m[3][1] = -2 * t0 * y + 2 * t1 * z - 2 * x * t3 + 2 * w * t2;
	m[3][2] = -2 * t0 * z + 2 * x * t2 + 2 * w * t3 - 2 * t1 * y;

	m[0][3] = 0;
	m[1][3] = 0;
	m[2][3] = 0;
	m[3][3] = len2;
	m /= len2;

	return m;
}

void main() {
	

	TexCoord = aTexCoords;
	
	mat4 BoneTransform = gBones[BoneIDs[0]] * Weights[0];
	BoneTransform += gBones[BoneIDs[1]] * Weights[1];
	BoneTransform += gBones[BoneIDs[2]] * Weights[2];
	BoneTransform += gBones[BoneIDs[3]] * Weights[3];
	
	mat2x4 dq0 = dqs[BoneIDs[0]];
	mat2x4 dq1 = dqs[BoneIDs[1]];
	mat2x4 dq2 = dqs[BoneIDs[2]];
	mat2x4 dq3 = dqs[BoneIDs[3]];

	if (dot(dq0[0], dq1[0]) < 0.0) dq1 *= -1.0;
	if (dot(dq0[0], dq2[0]) < 0.0) dq2 *= -1.0;
	if (dot(dq0[0], dq3[0]) < 0.0) dq3 *= -1.0;

	mat2x4 blendDQ = dq0 * Weights[0];
	blendDQ += dq1 * Weights[1];
	blendDQ += dq2 * Weights[2];
	blendDQ += dq3 * Weights[3];

	mat4x4 DQmat = DQtoMat(blendDQ[0], blendDQ[1]);


	vec4 pos = BoneTransform * vec4(aPos, 1.0);
	gl_Position = projection * view * model * pos;

	FragPos = vec3(model* pos);
	Normal = mat3(transpose(inverse(BoneTransform))) * aNormal;
}