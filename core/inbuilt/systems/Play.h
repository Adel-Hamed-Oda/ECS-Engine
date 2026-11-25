#pragma once

#include <algorithm>
#include <glm/glm.hpp>

#include "core/ecs/System.h"
#include "core/global/EngineTime.h"
#include "core/inbuilt/components/Animation.h"

// Inbuilt ECS system that plays an Animation component once from start to finish
class Play : public System<Animation> {
public:
    Play()
        : System<Animation>([](Entity e, Animation& anim) {
            UpdateAnimation(e, anim);
        }) {}

    static void EvaluatePose(Animation& anim) {
        if (!anim.clip || !anim.skeleton || !anim.skeleton->HasBones()) {
            return;
        }

        if (anim.finalBoneMatrices.size() < static_cast<size_t>(Mesh::MAX_BONES)) {
            anim.finalBoneMatrices.resize(Mesh::MAX_BONES, glm::mat4(1.0f));
        }

        CalculateBoneTransform(anim, anim.skeleton->rootNode, glm::mat4(1.0f));
    }

    static void UpdateAnimation(Entity /*entity*/, Animation& anim) {
        if (!anim.isPlaying || anim.hasFinished || !anim.clip || !anim.skeleton || !anim.skeleton->HasBones()) {
            return;
        }

        if (anim.finalBoneMatrices.size() < static_cast<size_t>(Mesh::MAX_BONES)) {
            anim.finalBoneMatrices.resize(Mesh::MAX_BONES, glm::mat4(1.0f));
        }

        const float ticksPerSecond = (anim.clip->ticksPerSecond > 0.0f)
            ? anim.clip->ticksPerSecond
            : 25.0f;

        // Clamp first-frame loading deltaTime spikes so the clip doesn't skip to the end immediately
        const float dt = std::min(static_cast<float>(Time::deltaTime), 0.1f);
        anim.currentTime += dt * ticksPerSecond * anim.speed;

        // Play the animation once and stop at the final pose
        if (anim.currentTime >= anim.clip->duration) {
            anim.currentTime = anim.clip->duration;
            anim.isPlaying = false;
            anim.hasFinished = true;
        }

        CalculateBoneTransform(anim, anim.skeleton->rootNode, glm::mat4(1.0f));
    }

private:
    static void CalculateBoneTransform(
        Animation& anim,
        const SkeletonNode& node,
        const glm::mat4& parentTransform
    ) {
        glm::mat4 nodeTransform = node.transformation;

        if (const BoneChannel* channel = anim.clip->FindChannel(node.name)) {
            nodeTransform = channel->GetLocalTransform(anim.currentTime);
        } else {
            // Assimp FBX splits nodes with PreRotation/PostRotation into
            // "<Base>_$AssimpFbx$_Translation", "<Base>_$AssimpFbx$_PreRotation", etc.
            // while GenerateSimpleNodeAnim places the combined TRS + Pre/PostRotation
            // animation channel on "<Base>". Neutralize the static chain nodes when
            // "<Base>" has a direct animation channel so transforms aren't applied twice.
            const size_t fbxTagPos = node.name.find("_$AssimpFbx$_");
            if (fbxTagPos != std::string::npos) {
                const std::string baseName = node.name.substr(0, fbxTagPos);
                if (anim.clip->FindChannel(baseName) != nullptr) {
                    nodeTransform = glm::mat4(1.0f);
                }
            }
        }

        glm::mat4 globalTransform = parentTransform * nodeTransform;

        auto it = anim.skeleton->boneInfoMap.find(node.name);
        if (it != anim.skeleton->boneInfoMap.end()) {
            int boneIndex = it->second.id;
            if (boneIndex >= 0 && boneIndex < static_cast<int>(anim.finalBoneMatrices.size())) {
                anim.finalBoneMatrices[boneIndex] =
                    anim.skeleton->globalInverseTransform *
                    it->second.invBindTransform *
                    globalTransform *
                    it->second.offsetMatrix;
            }
        }

        for (const SkeletonNode& child : node.children) {
            CalculateBoneTransform(anim, child, globalTransform);
        }
    }
};
