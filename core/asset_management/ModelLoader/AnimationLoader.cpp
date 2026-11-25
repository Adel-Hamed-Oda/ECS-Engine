#include "ModelLoader.h"

#include <assimp/scene.h>
#include <assimp/anim.h>

namespace {

static glm::mat4 ConvertAssimpNodeMatrix(const aiMatrix4x4& from) {
    glm::mat4 to;
    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    return to;
}

static BoneChannel LoadBoneChannel(const aiNodeAnim* channel) {
    BoneChannel boneChannel;
    boneChannel.name = channel->mNodeName.C_Str();

    boneChannel.positions.reserve(channel->mNumPositionKeys);
    for (unsigned int i = 0; i < channel->mNumPositionKeys; ++i) {
        const aiVector3D& aiPos = channel->mPositionKeys[i].mValue;
        KeyPosition data;
        data.position = glm::vec3(aiPos.x, aiPos.y, aiPos.z);
        data.timeStamp = static_cast<float>(channel->mPositionKeys[i].mTime);
        boneChannel.positions.push_back(data);
    }

    boneChannel.rotations.reserve(channel->mNumRotationKeys);
    for (unsigned int i = 0; i < channel->mNumRotationKeys; ++i) {
        const aiQuaternion& aiOrientation = channel->mRotationKeys[i].mValue;
        KeyRotation data;
        data.orientation = glm::normalize(glm::quat(aiOrientation.w, aiOrientation.x, aiOrientation.y, aiOrientation.z));
        data.timeStamp = static_cast<float>(channel->mRotationKeys[i].mTime);
        boneChannel.rotations.push_back(data);
    }

    boneChannel.scales.reserve(channel->mNumScalingKeys);
    for (unsigned int i = 0; i < channel->mNumScalingKeys; ++i) {
        const aiVector3D& scale = channel->mScalingKeys[i].mValue;
        KeyScale data;
        data.scale = glm::vec3(scale.x, scale.y, scale.z);
        data.timeStamp = static_cast<float>(channel->mScalingKeys[i].mTime);
        boneChannel.scales.push_back(data);
    }

    return boneChannel;
}

static void ComputeBindPoseInverseTransforms(
    const SkeletonNode& node,
    const glm::mat4& parentTransform,
    Skeleton& skeleton
) {
    glm::mat4 globalTransform = parentTransform * node.transformation;

    auto it = skeleton.boneInfoMap.find(node.name);
    if (it != skeleton.boneInfoMap.end()) {
        glm::mat4 bindResidual = globalTransform * it->second.offsetMatrix;
        float det = glm::determinant(bindResidual);
        if (std::abs(det) > 1e-8f) {
            it->second.invBindTransform = glm::inverse(bindResidual);
        } else {
            it->second.invBindTransform = skeleton.globalInverseTransform;
        }
    }

    for (const SkeletonNode& child : node.children) {
        ComputeBindPoseInverseTransforms(child, globalTransform, skeleton);
    }
}

} // anonymous namespace

void ModelLoader::BuildSkeletonHierarchy(const aiNode* srcNode, SkeletonNode& outNode) {
    if (!srcNode) return;

    outNode.name = srcNode->mName.C_Str();
    outNode.transformation = ConvertAssimpNodeMatrix(srcNode->mTransformation);
    outNode.children.resize(srcNode->mNumChildren);

    for (unsigned int i = 0; i < srcNode->mNumChildren; ++i) {
        BuildSkeletonHierarchy(srcNode->mChildren[i], outNode.children[i]);
    }
}

void ModelLoader::LoadAssimpAnimations(const aiScene* scene, ModelData& outModel) {
    if (!scene || !scene->mRootNode) return;

    if (outModel.skeleton) {
        outModel.skeleton->globalInverseTransform = glm::mat4(1.0f);
        BuildSkeletonHierarchy(scene->mRootNode, outModel.skeleton->rootNode);
        ComputeBindPoseInverseTransforms(outModel.skeleton->rootNode, glm::mat4(1.0f), *outModel.skeleton);
    }

    if (!scene->HasAnimations()) return;

    outModel.animations.reserve(scene->mNumAnimations);
    for (unsigned int a = 0; a < scene->mNumAnimations; ++a) {
        const aiAnimation* aiAnim = scene->mAnimations[a];
        if (!aiAnim) continue;

        auto clip = std::make_shared<AnimationClip>();
        clip->name = aiAnim->mName.C_Str();
        if (clip->name.empty()) {
            clip->name = "Animation_" + std::to_string(a);
        }
        clip->duration = static_cast<float>(aiAnim->mDuration);
        clip->ticksPerSecond = (aiAnim->mTicksPerSecond != 0.0)
            ? static_cast<float>(aiAnim->mTicksPerSecond)
            : 25.0f;

        for (unsigned int c = 0; c < aiAnim->mNumChannels; ++c) {
            const aiNodeAnim* channel = aiAnim->mChannels[c];
            if (!channel) continue;

            std::string channelName = channel->mNodeName.C_Str();
            clip->channels[channelName] = LoadBoneChannel(channel);
        }

        outModel.animations.push_back(std::move(clip));
    }
}

std::vector<std::shared_ptr<AnimationClip>> ModelLoader::LoadAnimations(const std::string& filepath, ModelLoaderFlags flags) {
    ModelData model = Load(filepath, flags);
    return model.animations;
}
