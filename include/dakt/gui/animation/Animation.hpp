#ifndef DAKT_GUI_ANIMATION_HPP
#define DAKT_GUI_ANIMATION_HPP

#include "../core/Types.hpp"
#include <cmath>
#include <functional>
#include <unordered_map>
#include <vector>

namespace dakt::gui {

// ============================================================================
// Easing Functions
// ============================================================================

namespace easing {

inline float linear(float t) { return t; }

inline float easeInQuad(float t) { return t * t; }
inline float easeOutQuad(float t) { return t * (2.0f - t); }
inline float easeInOutQuad(float t) { return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t; }

inline float easeInCubic(float t) { return t * t * t; }
inline float easeOutCubic(float t) {
    float f = t - 1.0f;
    return f * f * f + 1.0f;
}
inline float easeInOutCubic(float t) { return t < 0.5f ? 4.0f * t * t * t : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f; }

inline float easeInExpo(float t) { return t == 0.0f ? 0.0f : std::pow(2.0f, 10.0f * (t - 1.0f)); }
inline float easeOutExpo(float t) { return t == 1.0f ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * t); }

inline float easeInBack(float t) {
    const float c = 1.70158f;
    return t * t * ((c + 1.0f) * t - c);
}
inline float easeOutBack(float t) {
    const float c = 1.70158f;
    float f = t - 1.0f;
    return f * f * ((c + 1.0f) * f + c) + 1.0f;
}

inline float easeOutBounce(float t) {
    if (t < 1.0f / 2.75f) {
        return 7.5625f * t * t;
    } else if (t < 2.0f / 2.75f) {
        t -= 1.5f / 2.75f;
        return 7.5625f * t * t + 0.75f;
    } else if (t < 2.5f / 2.75f) {
        t -= 2.25f / 2.75f;
        return 7.5625f * t * t + 0.9375f;
    } else {
        t -= 2.625f / 2.75f;
        return 7.5625f * t * t + 0.984375f;
    }
}

inline float easeInElastic(float t) {
    if (t == 0.0f || t == 1.0f)
        return t;
    return -std::pow(2.0f, 10.0f * (t - 1.0f)) * std::sin((t - 1.1f) * 5.0f * 3.14159265f);
}

inline float easeOutElastic(float t) {
    if (t == 0.0f || t == 1.0f)
        return t;
    return std::pow(2.0f, -10.0f * t) * std::sin((t - 0.1f) * 5.0f * 3.14159265f) + 1.0f;
}

} // namespace easing

// ============================================================================
// Animation Types
// ============================================================================

using EasingFunc = float (*)(float);

enum class AnimationState { Idle, Running, Paused, Finished };

enum class AnimationDirection { Forward, Reverse, Alternate, AlternateReverse };

struct AnimationConfig {
    float duration = 0.3f;
    float delay = 0.0f;
    EasingFunc easing = easing::easeOutQuad;
    AnimationDirection direction = AnimationDirection::Forward;
    int repeatCount = 0; // 0 = no repeat, -1 = infinite
    bool autoReverse = false;
};

// ============================================================================
// Tween - Animates a single value
// ============================================================================

template <typename T> class Tween {
  public:
    Tween() = default;
    Tween(T from, T to, const AnimationConfig& config = {}) : from_(from), to_(to), current_(from), config_(config) {}

    void start() {
        state_ = AnimationState::Running;
        elapsed_ = 0.0f;
        currentRepeat_ = 0;
        forward_ = (config_.direction == AnimationDirection::Forward || config_.direction == AnimationDirection::Alternate);
    }

    void pause() {
        if (state_ == AnimationState::Running)
            state_ = AnimationState::Paused;
    }

    void resume() {
        if (state_ == AnimationState::Paused)
            state_ = AnimationState::Running;
    }

    void stop() {
        state_ = AnimationState::Idle;
        elapsed_ = 0.0f;
    }

    void update(float dt) {
        if (state_ != AnimationState::Running)
            return;

        elapsed_ += dt;

        // Handle delay
        if (elapsed_ < config_.delay)
            return;

        float activeTime = elapsed_ - config_.delay;
        float t = std::min(activeTime / config_.duration, 1.0f);

        // Apply direction
        if (!forward_)
            t = 1.0f - t;

        // Apply easing
        float eased = config_.easing ? config_.easing(t) : t;

        // Interpolate
        current_ = lerp(from_, to_, eased);

        // Check completion
        if (activeTime >= config_.duration) {
            handleCompletion();
        }
    }

    T value() const { return current_; }
    AnimationState state() const { return state_; }
    bool isFinished() const { return state_ == AnimationState::Finished; }
    bool isRunning() const { return state_ == AnimationState::Running; }

    void setFrom(T from) { from_ = from; }
    void setTo(T to) { to_ = to; }

  private:
    void handleCompletion() {
        if (config_.repeatCount == 0) {
            state_ = AnimationState::Finished;
            current_ = forward_ ? to_ : from_;
            return;
        }

        currentRepeat_++;
        if (config_.repeatCount > 0 && currentRepeat_ >= config_.repeatCount) {
            state_ = AnimationState::Finished;
            current_ = forward_ ? to_ : from_;
            return;
        }

        // Reset for next iteration
        elapsed_ = config_.delay;

        if (config_.direction == AnimationDirection::Alternate || config_.direction == AnimationDirection::AlternateReverse) {
            forward_ = !forward_;
        }
    }

    static T lerp(T a, T b, float t) { return a + (b - a) * t; }

    T from_{};
    T to_{};
    T current_{};
    AnimationConfig config_;
    AnimationState state_ = AnimationState::Idle;
    float elapsed_ = 0.0f;
    int currentRepeat_ = 0;
    bool forward_ = true;
};

// Specialization for Color
template <> inline Color Tween<Color>::lerp(Color a, Color b, float t) { return Color::lerp(a, b, t); }

// Specialization for Vec2
template <> inline Vec2 Tween<Vec2>::lerp(Vec2 a, Vec2 b, float t) { return Vec2::lerp(a, b, t); }

// ============================================================================
// Animator - Manages multiple animations
// ============================================================================

class Animator {
  public:
    using AnimationID = uint64_t;
    using CompletionCallback = std::function<void(AnimationID)>;

    Animator() = default;
    ~Animator() = default;

    // Float animations
    AnimationID animateFloat(float from, float to, float* target, const AnimationConfig& config = {});

    // Vec2 animations
    AnimationID animateVec2(Vec2 from, Vec2 to, Vec2* target, const AnimationConfig& config = {});

    // Color animations
    AnimationID animateColor(Color from, Color to, Color* target, const AnimationConfig& config = {});

    // Control
    void pause(AnimationID id);
    void resume(AnimationID id);
    void stop(AnimationID id);
    void stopAll();

    // Update all animations
    void update(float dt);

    // Callbacks
    void onComplete(AnimationID id, CompletionCallback callback);

    // Query
    bool isRunning(AnimationID id) const;
    size_t activeCount() const;

  private:
    struct FloatAnim {
        Tween<float> tween;
        float* target;
    };
    struct Vec2Anim {
        Tween<Vec2> tween;
        Vec2* target;
    };
    struct ColorAnim {
        Tween<Color> tween;
        Color* target;
    };

    std::unordered_map<AnimationID, FloatAnim> floatAnims_;
    std::unordered_map<AnimationID, Vec2Anim> vec2Anims_;
    std::unordered_map<AnimationID, ColorAnim> colorAnims_;
    std::unordered_map<AnimationID, CompletionCallback> callbacks_;
    AnimationID nextId_ = 1;
};

// ============================================================================
// Spring Animation
// ============================================================================

template <typename T> class Spring {
  public:
    Spring(float stiffness = 100.0f, float damping = 10.0f) : stiffness_(stiffness), damping_(damping) {}

    void setTarget(T target) { target_ = target; }

    void update(float dt) {
        // Spring physics: F = -kx - cv
        T displacement = current_ - target_;
        T springForce = displacement * (-stiffness_);
        T dampingForce = velocity_ * (-damping_);
        T acceleration = springForce + dampingForce;

        velocity_ = velocity_ + acceleration * dt;
        current_ = current_ + velocity_ * dt;
    }

    T value() const { return current_; }
    void setValue(T value) {
        current_ = value;
        velocity_ = T{};
    }

    bool isSettled(float threshold = 0.001f) const {
        // Check if spring has settled (velocity and displacement are small)
        return lengthOf(current_ - target_) < threshold && lengthOf(velocity_) < threshold;
    }

  private:
    static float lengthOf(float v) { return std::abs(v); }
    static float lengthOf(Vec2 v) { return v.length(); }

    T current_{};
    T target_{};
    T velocity_{};
    float stiffness_;
    float damping_;
};

} // namespace dakt::gui

#endif
