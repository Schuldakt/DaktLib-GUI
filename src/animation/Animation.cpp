#include "dakt/gui/animation/Animation.hpp"

namespace dakt::gui {

Animator::AnimationID Animator::animateFloat(float from, float to, float* target, const AnimationConfig& config) {
    AnimationID id = nextId_++;
    FloatAnim anim;
    anim.tween = Tween<float>(from, to, config);
    anim.target = target;
    anim.tween.start();
    floatAnims_[id] = anim;
    return id;
}

Animator::AnimationID Animator::animateVec2(Vec2 from, Vec2 to, Vec2* target, const AnimationConfig& config) {
    AnimationID id = nextId_++;
    Vec2Anim anim;
    anim.tween = Tween<Vec2>(from, to, config);
    anim.target = target;
    anim.tween.start();
    vec2Anims_[id] = anim;
    return id;
}

Animator::AnimationID Animator::animateColor(Color from, Color to, Color* target, const AnimationConfig& config) {
    AnimationID id = nextId_++;
    ColorAnim anim;
    anim.tween = Tween<Color>(from, to, config);
    anim.target = target;
    anim.tween.start();
    colorAnims_[id] = anim;
    return id;
}

void Animator::pause(AnimationID id) {
    if (auto it = floatAnims_.find(id); it != floatAnims_.end())
        it->second.tween.pause();
    if (auto it = vec2Anims_.find(id); it != vec2Anims_.end())
        it->second.tween.pause();
    if (auto it = colorAnims_.find(id); it != colorAnims_.end())
        it->second.tween.pause();
}

void Animator::resume(AnimationID id) {
    if (auto it = floatAnims_.find(id); it != floatAnims_.end())
        it->second.tween.resume();
    if (auto it = vec2Anims_.find(id); it != vec2Anims_.end())
        it->second.tween.resume();
    if (auto it = colorAnims_.find(id); it != colorAnims_.end())
        it->second.tween.resume();
}

void Animator::stop(AnimationID id) {
    floatAnims_.erase(id);
    vec2Anims_.erase(id);
    colorAnims_.erase(id);
    callbacks_.erase(id);
}

void Animator::stopAll() {
    floatAnims_.clear();
    vec2Anims_.clear();
    colorAnims_.clear();
    callbacks_.clear();
}

void Animator::update(float dt) {
    // Update float animations
    for (auto it = floatAnims_.begin(); it != floatAnims_.end();) {
        it->second.tween.update(dt);
        if (it->second.target) {
            *it->second.target = it->second.tween.value();
        }
        if (it->second.tween.isFinished()) {
            AnimationID id = it->first;
            if (auto cb = callbacks_.find(id); cb != callbacks_.end()) {
                cb->second(id);
                callbacks_.erase(cb);
            }
            it = floatAnims_.erase(it);
        } else {
            ++it;
        }
    }

    // Update Vec2 animations
    for (auto it = vec2Anims_.begin(); it != vec2Anims_.end();) {
        it->second.tween.update(dt);
        if (it->second.target) {
            *it->second.target = it->second.tween.value();
        }
        if (it->second.tween.isFinished()) {
            AnimationID id = it->first;
            if (auto cb = callbacks_.find(id); cb != callbacks_.end()) {
                cb->second(id);
                callbacks_.erase(cb);
            }
            it = vec2Anims_.erase(it);
        } else {
            ++it;
        }
    }

    // Update Color animations
    for (auto it = colorAnims_.begin(); it != colorAnims_.end();) {
        it->second.tween.update(dt);
        if (it->second.target) {
            *it->second.target = it->second.tween.value();
        }
        if (it->second.tween.isFinished()) {
            AnimationID id = it->first;
            if (auto cb = callbacks_.find(id); cb != callbacks_.end()) {
                cb->second(id);
                callbacks_.erase(cb);
            }
            it = colorAnims_.erase(it);
        } else {
            ++it;
        }
    }
}

void Animator::onComplete(AnimationID id, CompletionCallback callback) { callbacks_[id] = std::move(callback); }

bool Animator::isRunning(AnimationID id) const {
    if (auto it = floatAnims_.find(id); it != floatAnims_.end())
        return it->second.tween.isRunning();
    if (auto it = vec2Anims_.find(id); it != vec2Anims_.end())
        return it->second.tween.isRunning();
    if (auto it = colorAnims_.find(id); it != colorAnims_.end())
        return it->second.tween.isRunning();
    return false;
}

size_t Animator::activeCount() const { return floatAnims_.size() + vec2Anims_.size() + colorAnims_.size(); }

} // namespace dakt::gui
