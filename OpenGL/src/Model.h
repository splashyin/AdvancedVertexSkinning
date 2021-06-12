#pragma once

#include <GL/glew.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_CTOR_INIT
#include <gtx/quaternion.hpp>
#include <gtx/dual_quaternion.hpp>
#include "Shader.h"
#include "Mesh.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

#include "assimp\scene.h"
#include "assimp\Importer.hpp"
#include "assimp\postprocess.h"

unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma = false);
glm::mat3x4 convertMatrix(glm::mat4 s);
glm::quat quatcast(glm::mat4 t);

typedef std::map< std::string, std::map< std::string, const aiNodeAnim* > > AnimationMap;

class Model
{
public:

	/*  Model Data */
	std::vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
	std::string directory;
	bool gammaCorrection;

	unsigned int total_vertices = 0;

	/*Bone Data*/
	unsigned int m_NumBones = 0;
	std::vector<VertexBoneData> Bones;
	std::map<std::string, unsigned int> Bone_Mapping;
	AnimationMap Animations;
	std::map<unsigned int, glm::vec3> skeleton_pose;
	std::map<std::string, unsigned int> Node_Mapping;
	std::vector<BoneInfo> m_BoneInfo;
	unsigned int NumVertices = 0;

	glm::fdualquat IdentityDQ = glm::fdualquat(glm::quat(1.f, 0.f, 0.f, 0.f), glm::quat(0.f, 0.f, 0.f, 0.f));

	// Ctor
	Model( const std::string& path, bool gamma );

	// Draws the model, and thus all its meshes
	void Draw( const Shader& i_shader );

	void BoneTransform( const float&  i_timeInSeconds, std::vector<glm::mat4>& i_transforms, std::vector<glm::fdualquat>& io_dqs );


private:

	std::vector< Mesh > m_meshes;

	const aiScene* scene;
	Assimp::Importer importer;

	// loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
	void loadModel( const std::string& i_path );

	void loadBones( aiNode* node, const aiScene* scene );

	// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	void processNode( aiNode* node, const aiScene* scene );

	// process a mesh object (does a copy)
	void processMesh( aiMesh* mesh, const aiScene* scene );

	// checks all material textures of a given type and loads the textures if they're not loaded yet.
	// the required info is returned as a Texture struct.
	std::vector<Texture> loadMaterialTextures( aiMaterial* mat, aiTextureType type, const std::string& typeName );

	void loadMeshBones( aiMesh* mesh, std::vector<VertexBoneData>& VertexBoneData );

	//get animation from the bone
	//populate the animation map : animation_map[animation_name][bone_name] -> animation
	void loadAnimations( const aiScene* scene, std::string BoneName, AnimationMap& o_animations );

	void ReadNodeHeirarchy( const aiScene* scene, float AnimationTime, const aiNode* pNode,
		const glm::mat4& ParentTransform, const glm::fdualquat& ParentDQ, glm::vec3 startpos );

	void CalcInterpolatedScaling( aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim );

	void CalcInterpolatedRotaion( aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim );

	void CalcInterpolatedPosition( aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim );

	unsigned int FindScaling( float AnimationTime, const aiNodeAnim* pNodeAnim );

	unsigned int FindRotation( float AnimationTime, const aiNodeAnim* pNodeAnim );

	unsigned int FindPosition( float AnimationTime, const aiNodeAnim* pNodeAnim );
};