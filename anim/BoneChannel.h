#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

struct KeyPosition {
    glm::vec3 position = glm::vec3(0.0f);
    float timeStamp = 0.0f;
};

struct KeyRotation {
    glm::quat orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    float timeStamp = 0.0f;
};

struct KeyScale {
    glm::vec3 scale = glm::vec3(1.0f);
    float timeStamp = 0.0f;
};

struct BoneChannel {
    std::string name;
    std::vector<KeyPosition> positions;
    std::vector<KeyRotation> rotations;
    std::vector<KeyScale> scales;

    glm::mat4 GetLocalTransform(float animationTime) const {
        glm::mat4 translation = InterpolatePosition(animationTime);
        glm::mat4 rotation = InterpolateRotation(animationTime);
        glm::mat4 scale = InterpolateScale(animationTime);
        return translation * rotation * scale;
    }

private:
    static float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) {
        float framesDiff = nextTimeStamp - lastTimeStamp;
        if (framesDiff <= 0.00001f) {
            return 0.0f;
        }
        float midWayLength = animationTime - lastTimeStamp;
        return std::clamp(midWayLength / framesDiff, 0.0f, 1.0f);
    }

    int GetPositionIndex(float animationTime) const {
        for (size_t i = 0; i + 1 < positions.size(); ++i) {
            if (animationTime < positions[i + 1].timeStamp) {
                return static_cast<int>(i);
            }
        }
        return static_cast<int>(positions.size()) - 2;
    }

    int GetRotationIndex(float animationTime) const {
        for (size_t i = 0; i + 1 < rotations.size(); ++i) {
            if (animationTime < rotations[i + 1].timeStamp) {
                return static_cast<int>(i);
            }
        }
        return static_cast<int>(rotations.size()) - 2;
    }

    int GetScaleIndex(float animationTime) const {
        for (size_t i = 0; i + 1 < scales.size(); ++i) {
            if (animationTime < scales[i + 1].timeStamp) {
                return static_cast<int>(i);
            }
        }
        return static_cast<int>(scales.size()) - 2;
    }

    glm::mat4 InterpolatePosition(float animationTime) const {
        if (positions.empty()) {
            return glm::mat4(1.0f);
        }
        if (positions.size() == 1) {
            return glm::translate(glm::mat4(1.0f), positions[0].position);
        }

        int p0Index = std::max(0, GetPositionIndex(animationTime));
        int p1Index = p0Index + 1;
        float scaleFactor = GetScaleFactor(
            positions[p0Index].timeStamp,
            positions[p1Index].timeStamp,
            animationTime
        );
        glm::vec3 finalPosition = glm::mix(
            positions[p0Index].position,
            positions[p1Index].position,
            scaleFactor
        );
        return glm::translate(glm::mat4(1.0f), finalPosition);
    }

    glm::mat4 InterpolateRotation(float animationTime) const {
        if (rotations.empty()) {
            return glm::mat4(1.0f);
        }
        if (rotations.size() == 1) {
            glm::quat rotation = glm::normalize(rotations[0].orientation);
            return glm::mat4_cast(rotation);
        }

        int p0Index = std::max(0, GetRotationIndex(animationTime));
        int p1Index = p0Index + 1;
        float scaleFactor = GetScaleFactor(
            rotations[p0Index].timeStamp,
            rotations[p1Index].timeStamp,
            animationTime
        );
        glm::quat finalRotation = glm::slerp(
            rotations[p0Index].orientation,
            rotations[p1Index].orientation,
            scaleFactor
        );
        finalRotation = glm::normalize(finalRotation);
        return glm::mat4_cast(finalRotation);
    }

    glm::mat4 InterpolateScale(float animationTime) const {
        if (scales.empty()) {
            return glm::mat4(1.0f);
        }
        if (scales.size() == 1) {
            return glm::scale(glm::mat4(1.0f), scales[0].scale);
        }

        int p0Index = std::max(0, GetScaleIndex(animationTime));
        int p1Index = p0Index + 1;
        float scaleFactor = GetScaleFactor(
            scales[p0Index].timeStamp,
            scales[p1Index].timeStamp,
            animationTime
        );
        glm::vec3 finalScale = glm::mix(
            scales[p0Index].scale,
            scales[p1Index].scale,
            scaleFactor
        );
        return glm::scale(glm::mat4(1.0f), finalScale);
    }
};
