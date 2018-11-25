#pragma once

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/dual_quaternion.hpp>
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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


using std::string;

unsigned int TextureFromFile(const char *path, const string &directory, bool gamma = false);

void debuggingMatrix(glm::mat4 array);
void debugVertexBoneData(unsigned int total_vertices, vector<VertexBoneData> Bones);
void debugSkeletonPose(map<unsigned int, glm::vec3> skeletonPos);
class Model
{
public:
	
	/*  Model Data */
	vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
	vector<Mesh> meshes;
	string directory;
	bool gammaCorrection;

	unsigned int total_vertices = 0;
	
	/*Bone Data*/
	unsigned int m_NumBones = 0;
	vector<VertexBoneData> Bones;
	map<string, unsigned int> Bone_Mapping;
	map<string, map<string, const aiNodeAnim*>> Animations;
	map<unsigned int, glm::vec3> skeleton_pose;
	map<string, unsigned int> Node_Mapping;
	vector<BoneInfo> m_BoneInfo;
	unsigned int NumVertices = 0;


	/*  Functions   */
	// constructor, expects a filepath to a 3D model.
	Model(string const &path, bool gamma = false) : gammaCorrection(gamma)
	{
		loadModel(path);	
	}

	// draws the model, and thus all its meshes
	void Draw(Shader shader)
	{
		for (unsigned int i = 0; i < meshes.size(); i++)
			meshes[i].Draw(shader);
	}

	void BoneTransform(float TimeInSeconds, vector<glm::mat4>& Transforms, vector<glm::fdualquat>& dqs) {
		glm::mat4 Identity = glm::mat4(1.0f);

		unsigned int numPosKeys = scene->mAnimations[0]->mChannels[0]->mNumPositionKeys;

		float TicksPerSecond = scene->mAnimations[0]->mTicksPerSecond != 0 ?
			scene->mAnimations[0]->mTicksPerSecond : 25.0f;

		float TimeInTicks = TimeInSeconds * TicksPerSecond;
		float AnimationTime = fmod(TimeInTicks, scene->mAnimations[0]->mChannels[0]->mPositionKeys[numPosKeys - 1].mTime);

		ReadNodeHeirarchy(scene, AnimationTime, scene->mRootNode, Identity, glm::vec3(0.0f, 0.0f, 0.0f));
		
		//debugSkeletonPose(skeleton_pose);
		
		Transforms.resize(m_NumBones);
		dqs.resize(m_NumBones);
		for (unsigned int i = 0; i < m_NumBones; i++) {
			Transforms[i] = m_BoneInfo[i].FinalTransformation;
		}
		for (unsigned int i = 0; i < dqs.size(); i++) {
			dqs[i] = glm::normalize(m_BoneInfo[i].FinalTransDQ);
		}
	}
	
private:
	const aiScene* scene;
	Assimp::Importer importer;
	// loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
	void loadModel(string const &path)
	{
		scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
		// check for errors
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
		{
			cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
			return;
		}
		// retrieve the directory path of the filepath
		directory = path.substr(0, path.find_last_of('/'));

		loadBones(scene->mRootNode, scene);
		m_BoneInfo.resize(Bone_Mapping.size());
		// process ASSIMP's root node recursively
		processNode(scene->mRootNode, scene);
	}

	void loadBones(aiNode *node, const aiScene *scene)
	{
		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			string NodeName(node->mChildren[i]->mName.data);
			if (NodeName.find(":") != string::npos) {
				string BoneName = NodeName;
				unsigned int BoneIndex = 0;

				if (Bone_Mapping.find(BoneName) == Bone_Mapping.end()) {
					BoneIndex = m_NumBones;
					m_NumBones++;
					Bone_Mapping[BoneName] = BoneIndex;
				}
			}

			if (NodeName != "Body" &&NodeName != "metarig"&& NodeName != "parasiteZombie" && NodeName != "Armature"  && NodeName != "MutantMesh" && NodeName != "Cylinder") {
				string BoneName = NodeName;
				unsigned int BoneIndex = 0;

				if (Bone_Mapping.find(BoneName) == Bone_Mapping.end()) {
					BoneIndex = m_NumBones;
					m_NumBones++;
					Bone_Mapping[BoneName] = BoneIndex;
				}
			}

						//only uncomment if we need to load cylinder model
			/*else {
				string BoneName(node->mChildren[i]->mName.data);
				unsigned int BoneIndex = 0;
				if (NodeName != "parasiteZombie" || NodeName != "Armature") {
					if (Bone_Mapping.find(BoneName) == Bone_Mapping.end()) {
						BoneIndex = m_NumBones;
						m_NumBones++;
						Bone_Mapping[BoneName] = BoneIndex;
					}
				}
				
			}*/

		}
		
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			loadBones(node->mChildren[i], scene);
		}

		
	}

	// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	void processNode(aiNode *node, const aiScene *scene)
	{
		// process each mesh located at the current node
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			// the node object only contains indices to index the actual objects in the scene. 
			// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			total_vertices += mesh->mNumVertices;
			meshes.push_back(processMesh(mesh, scene));
		}
		// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}

	}

	// process a mesh object
	Mesh processMesh(aiMesh *mesh, const aiScene *scene)
	{
		// data to fill
		vector<Vertex> vertices;
		vector<unsigned int> indices;
		vector<Texture> textures;

		Bones.resize(total_vertices);

		// Walk through each of the mesh's vertices
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			glm::vec3 vector;

			//retreive positions
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;

			//retreive normals
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.Normal = vector;
			
			//retreive texture coordinates
			if (mesh->mTextureCoords[0])
			{
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
			}
			else {
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);
			}
			vertices.push_back(vertex);
		}
		
		//retreive indices
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
			vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		}
		
		// process materials
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		// 1. diffuse maps
		vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		// 2. specular maps
		vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		// 3. normal maps
		std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
		// 4. height maps
		std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
		//retreive bone information
		loadMeshBones(mesh, Bones);
		// return a mesh object created from the extracted mesh data
		return Mesh(vertices, indices, textures, m_BoneInfo, Bones);
	}

	// checks all material textures of a given type and loads the textures if they're not loaded yet.
	// the required info is returned as a Texture struct.
	vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName)
	{
		vector<Texture> textures;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			// check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
			bool skip = false;
			for (unsigned int j = 0; j < textures_loaded.size(); j++)
			{
				if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
				{
					textures.push_back(textures_loaded[j]);
					skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
					break;
				}
			}
			if (!skip)
			{   // if texture hasn't been loaded already, load it
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), this->directory);
				texture.type = typeName;
				texture.path = str.C_Str();
				textures.push_back(texture);
				textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
			}
		}
		return textures;
	}

	void loadMeshBones(aiMesh *mesh, vector<VertexBoneData>& VertexBoneData) 
	{		
		for (unsigned int i = 0; i < mesh->mNumBones; i++) 
		{	
			unsigned int BoneIndex = 0;
			string BoneName(mesh->mBones[i]->mName.data);

			if (Bone_Mapping.find(BoneName) != Bone_Mapping.end())
			{
				BoneIndex = Bone_Mapping[BoneName];
				//BoneInfo bi;
				//m_BoneInfo.push_back(bi);

				aiMatrix4x4 tp1 = mesh->mBones[i]->mOffsetMatrix;
				m_BoneInfo[BoneIndex].offset = glm::transpose(glm::make_mat4(&tp1.a1));
			}
		
			for (unsigned int n = 0; n < mesh->mBones[i]->mNumWeights; n++) {
				unsigned int vid = mesh->mBones[i]->mWeights[n].mVertexId + NumVertices;//absolute index
				float weight = mesh->mBones[i]->mWeights[n].mWeight;
				VertexBoneData[vid].AddBoneData(BoneIndex, weight);
			}
			loadAnimations(scene, BoneName, Animations);
		}
		NumVertices += mesh->mNumVertices;
	}
	
	//get animation from the bone
	//populate the animation map : animation_map[animation_name][bone_name] -> animation
	void loadAnimations(const aiScene *scene, string BoneName, map<string, map<string, const aiNodeAnim*>>& animations)
	{
		for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
			const aiAnimation* pAnimation = scene->mAnimations[i];
			string animName = pAnimation->mName.data;
			for (unsigned int j = 0; j < pAnimation->mNumChannels; j++) {
				if (pAnimation->mChannels[j]->mNodeName.data == BoneName) {
					animations[animName][BoneName] = pAnimation->mChannels[j];
					break;
				}
			}
		}
	}

	void ReadNodeHeirarchy(	const aiScene *scene, float AnimationTime, const aiNode* pNode,
		const glm::mat4& ParentTransform, glm::vec3 startpos)
	{
		string NodeName(pNode->mName.data);
		const aiAnimation* pAnimation = scene->mAnimations[0];
		glm::mat4 NodeTransformation = glm::mat4(1.0f);

		aiMatrix4x4 tp1 = pNode->mTransformation;
		NodeTransformation = glm::transpose(glm::make_mat4(&tp1.a1));
			
		const aiNodeAnim* pNodeAnim = nullptr;
		pNodeAnim = Animations[pAnimation->mName.data][NodeName];
		if (pNodeAnim) {

			//Interpolate rotation and generate rotation transformation matrix
			aiQuaternion RotationQ;
			CalcInterpolatedRotaion(RotationQ, AnimationTime, pNodeAnim);
			glm::fquat rotation = glm::fquat(1.0f, 1.0f, 1.0f, 1.0f);
			rotation.w = RotationQ.w;
			rotation.x = RotationQ.x;
			rotation.y = RotationQ.y;
			rotation.z = RotationQ.z;
			glm::mat4 RotationM = glm::toMat4(rotation);

			/*aiMatrix3x3 tp = RotationQ.GetMatrix();
			glm::mat4 RotationM = glm::transpose(glm::make_mat3(&tp.a1));
*/
			//Interpolate translation and generate translation transformation matrix
			aiVector3D Translation;
			CalcInterpolatedPosition(Translation, AnimationTime, pNodeAnim);
			glm::mat4 TranslationM = glm::mat4(1.0f);
			TranslationM = glm::translate(TranslationM, glm::vec3(Translation.x, Translation.y, Translation.z));

			NodeTransformation = TranslationM * RotationM;
			
		}

		glm::mat4 GlobalTransformation = ParentTransform * NodeTransformation;


		unsigned int ID = 0;
		

		if (Bone_Mapping.find(NodeName) != Bone_Mapping.end()) {
			startpos.x = GlobalTransformation[3][0];
			startpos.y = GlobalTransformation[3][1];
			startpos.z = GlobalTransformation[3][2];
			ID = Bone_Mapping[NodeName];
			skeleton_pose[ID] = startpos;
		}

		if (Bone_Mapping.find(NodeName) != Bone_Mapping.end()) {
			unsigned int NodeIndex = Bone_Mapping[NodeName];
			m_BoneInfo[NodeIndex].FinalTransformation = GlobalTransformation * m_BoneInfo[NodeIndex].offset;	

			//dual quaternion 
			glm::fquat FinalQuat = glm::quat_cast(m_BoneInfo[NodeIndex].FinalTransformation);
			
			glm::fquat FinalQuatNorm = glm::normalize(FinalQuat);

			m_BoneInfo[NodeIndex].FinalTransDQ = glm::fdualquat(FinalQuatNorm, glm::vec3(m_BoneInfo[NodeIndex].FinalTransformation[3][0],
																					 m_BoneInfo[NodeIndex].FinalTransformation[3][1],
																					 m_BoneInfo[NodeIndex].FinalTransformation[3][2]));
		}
		for (unsigned int i = 0; i < pNode->mNumChildren; i++) {
			ReadNodeHeirarchy(scene, AnimationTime, pNode->mChildren[i], GlobalTransformation, startpos);
		}
	}	


	void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		if (pNodeAnim->mNumScalingKeys == 1) {
			Out = pNodeAnim->mScalingKeys[0].mValue;
			return;
		}

		unsigned int ScalingIndex = FindScaling(AnimationTime, pNodeAnim);
		unsigned int NextScalingIndex = (ScalingIndex + 1);
		assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
		float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
		float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
		assert(Factor >= 0.0f && Factor <= 1.0f);
		const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
		const aiVector3D& End = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
		aiVector3D Delta = End - Start;
		Out = Start + Factor * Delta;
	}

	void CalcInterpolatedRotaion(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		// we need at least two values to interpolate...
		if (pNodeAnim->mNumRotationKeys == 1) {
			Out = pNodeAnim->mRotationKeys[0].mValue;
			return;
		}

		unsigned int RotationIndex = FindRotation(AnimationTime, pNodeAnim);
		unsigned int NextRotationIndex = (RotationIndex + 1);
		assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
		float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
		float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
		assert(Factor >= 0.0f && Factor <= 1.0f);
		const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
		const aiQuaternion& EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
		aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
		Out = Out.Normalize();//normalized
	}

	void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
	{	
		if (pNodeAnim->mNumPositionKeys == 1) {
			Out = pNodeAnim->mPositionKeys[0].mValue;
			return;
		}

		unsigned int PositionIndex = FindPosition(AnimationTime, pNodeAnim);
		unsigned int NextPositionIndex = (PositionIndex + 1);
		assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
		float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
		float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
		assert(Factor >= 0.0f && Factor <= 1.0f);
		const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
		const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
		aiVector3D Delta = End - Start;
		Out = Start + Factor * Delta;
	}

	unsigned int FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		assert(pNodeAnim->mNumScalingKeys > 0);
		for (unsigned int i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++) {
			if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
				return i;
			}
		}
		assert(0);
		return 0;
	}

	unsigned int FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		assert(pNodeAnim->mNumRotationKeys > 0);

		for (unsigned int i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
			if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
				return i;
			}
		}

		assert(0);
		return 0;
	}

	unsigned int FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		for (unsigned int i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++) {
			if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
				return i;
			}
		}
		assert(0);
		return 0;
	}
};

//Helpers
unsigned int TextureFromFile(const char *path, const string &directory, bool gamma)
{
	string filename = string(path);
	filename = directory + '/' + filename;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

//Debuggers
void debuggingMatrix(glm::mat4 array)
{
	for (int r = 0; r < 4; ++r)
	{
		for (int c = 0; c < 4; ++c)
		{
			std::cout << array[r][c] << "  ";
		}

		std::cout << std::endl;
	}
}

void debugVertexBoneData(unsigned int total_vertices, vector<VertexBoneData> Bones)
{
	std::ofstream log;
	log.open("VertexBoneData.txt");
	

	unsigned int i, j;
	for (i = 0; i < total_vertices; i++)
	{
		log << "\nBone[" << Bones[i].BoneIDs[0] << "], weight= " << Bones[i].Weights[0];
		log << "\nBone[" << Bones[i].BoneIDs[1] << "], weight= " << Bones[i].Weights[1];
		log << "\nBone[" << Bones[i].BoneIDs[2] << "], weight= " << Bones[i].Weights[2];
		log << "\nBone[" << Bones[i].BoneIDs[3] << "], weight= " << Bones[i].Weights[3];
		log << "---------------\n";
	}

	log.close();
}

void debugSkeletonPose(map<unsigned int, glm::vec3> skeletonPos) {
	for (auto it = skeletonPos.cbegin(); it != skeletonPos.cend(); ++it)
	{
		std::cout << it->first << " " << it->second.x << " " << it->second.y << " " << it->second.z <<"\n";
	}
}

	