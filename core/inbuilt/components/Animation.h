#pragma once

#include <vector>
#include <memory>
#include <utility>
#include <glm/glm.hpp>

#include "core/rendering/mesh/Mesh.h"
#include "anim/AnimationData.h"

struct Animation {
public:
    std::shared_ptr<Skeleton> skeleton = nullptr;
    std::shared_ptr<AnimationClip> clip = nullptr;

    float currentTime = 0.0f;
    float speed = 1.0f;
    bool isPlaying = true;
    bool hasFinished = false;

    std::vector<glm::mat4> finalBoneMatrices;

    Animation()
        : finalBoneMatrices(Mesh::MAX_BONES, glm::mat4(1.0f)) {}

    Animation(
        std::shared_ptr<Skeleton> skeleton,
        std::shared_ptr<AnimationClip> clip,
        float speed = 1.0f,
        bool playOnStart = true
    )
        : skeleton(std::move(skeleton)),
          clip(std::move(clip)),
          currentTime(0.0f),
          speed(speed),
          isPlaying(playOnStart),
          hasFinished(false),
          finalBoneMatrices(Mesh::MAX_BONES, glm::mat4(1.0f)) {}

    template <typename TModelData, typename = decltype(std::declval<TModelData>().skeleton)>
    explicit Animation(
        const TModelData& model,
        size_t clipIndex = 0,
        float speed = 1.0f,
        bool playOnStart = true
    )
        : skeleton(model.skeleton),
          clip(clipIndex < model.animations.size() ? model.animations[clipIndex] : nullptr),
          currentTime(0.0f),
          speed(speed),
          isPlaying(playOnStart),
          hasFinished(false),
          finalBoneMatrices(Mesh::MAX_BONES, glm::mat4(1.0f)) {}

    void PlayOnce() {
        currentTime = 0.0f;
        hasFinished = false;
        isPlaying = true;
    }

    bool HasValidBones() const {
        return skeleton && skeleton->HasBones() && !finalBoneMatrices.empty();
    }
};
