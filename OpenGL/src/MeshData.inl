#include <GL/glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/quaternion.hpp>
#include <gtx/dual_quaternion.hpp>

#define NUM_BONES_PER_VERTEX 4
#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define INVALID_MATERIAL 0xFFFFFFFF
//------------------------------------------------------
// VERTEX
//------------------------------------------------------

struct Vertex
{
	// position
	glm::vec3 Position;
	// normal
	glm::vec3 Normal;
	// texCoords
	glm::vec2 TexCoords;
	// tangent
	glm::vec3 Tangent;
	// bitangent
	glm::vec3 Bitangent;
};

//------------------------------------------------------
// TEXTURE
//------------------------------------------------------

struct Texture
{
	unsigned int id;
	std::string type;
	std::string path;
};

//------------------------------------------------------
// MESH ENTRY
//------------------------------------------------------

struct MeshEntry
{
	MeshEntry()
	{
		Mesh_Index = 0;
		Num_Bones = 0;
		BaseVertex = 0;
		BaseIndices = 0;
	}

	unsigned int Mesh_Index;
	unsigned int Num_Bones;
	unsigned int BaseVertex;
	unsigned int BaseIndices;
};

//------------------------------------------------------
// BONE INFO
//------------------------------------------------------

struct BoneInfo
{
	BoneInfo()
	{
		offset = glm::mat4( 1.0f );
		FinalTransformation = glm::mat4( 1.0f );
		FinalTransDQ.real = glm::quat( 1.0f, 0.0f, 0.0f, 0.0f );
		FinalTransDQ.dual = glm::quat( 0.0f, 0.0f, 0.0f, 0.0f );
		offsetDQ.real = glm::quat( 1.0f, 0.0f, 0.0f, 0.0f );
		offsetDQ.dual = glm::quat( 0.0f, 0.0f, 0.0f, 0.0f );
	}

	glm::mat4 offset;
	glm::mat4 FinalTransformation;
	glm::highp_fdualquat FinalTransDQ;
	glm::highp_fdualquat offsetDQ;
};

//------------------------------------------------------
// VERTEX BONE DATA
//------------------------------------------------------

struct VertexBoneData
{
	VertexBoneData()
	{
		Reset();
	};

	void Reset()
	{
		for (unsigned int i = 0; i < NUM_BONES_PER_VERTEX; ++i)
		{
			BoneIDs[i] = 0;
			Weights[i] = 0;
		}
	}

	void AddBoneData( unsigned int BoneID, float Weight )
	{
		for (unsigned int i = 0; i < NUM_BONES_PER_VERTEX; i++)
		{
			if (Weights[i] == 0.0)
			{
				BoneIDs[i] = BoneID;
				Weights[i] = Weight;
				return;
			}
		}
	}

	//vertex bone data
	unsigned int BoneIDs[NUM_BONES_PER_VERTEX];
	float Weights[NUM_BONES_PER_VERTEX];
};