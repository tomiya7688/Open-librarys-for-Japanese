#pragma once

#include <algorithm>
#include <any>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace olj::game {

struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    constexpr Vec3() = default;
    constexpr Vec3(float x_value, float y_value, float z_value = 0.0f)
        : x(x_value), y(y_value), z(z_value) {}

    [[nodiscard]] constexpr Vec3 operator+() const noexcept { return *this; }
    [[nodiscard]] constexpr Vec3 operator-() const noexcept { return {-x, -y, -z}; }

    [[nodiscard]] constexpr Vec3 operator+(const Vec3& other) const noexcept {
        return {x + other.x, y + other.y, z + other.z};
    }

    [[nodiscard]] constexpr Vec3 operator-(const Vec3& other) const noexcept {
        return {x - other.x, y - other.y, z - other.z};
    }

    [[nodiscard]] constexpr Vec3 operator*(float scalar) const noexcept {
        return {x * scalar, y * scalar, z * scalar};
    }

    [[nodiscard]] Vec3 operator/(float scalar) const {
        if (scalar == 0.0f) {
            throw std::invalid_argument("Vec3 division by zero");
        }
        return {x / scalar, y / scalar, z / scalar};
    }

    constexpr Vec3& operator+=(const Vec3& other) noexcept {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    constexpr Vec3& operator-=(const Vec3& other) noexcept {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    constexpr Vec3& operator*=(float scalar) noexcept {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    Vec3& operator/=(float scalar) {
        *this = *this / scalar;
        return *this;
    }

    [[nodiscard]] constexpr bool operator==(const Vec3& other) const noexcept {
        return x == other.x && y == other.y && z == other.z;
    }

    [[nodiscard]] constexpr bool operator!=(const Vec3& other) const noexcept {
        return !(*this == other);
    }

    [[nodiscard]] constexpr float length_squared() const noexcept {
        return x * x + y * y + z * z;
    }

    [[nodiscard]] float length() const noexcept {
        return std::sqrt(length_squared());
    }

    [[nodiscard]] Vec3 normalized() const noexcept {
        const float len = length();
        if (len == 0.0f) {
            return {};
        }
        return {x / len, y / len, z / len};
    }

    [[nodiscard]] constexpr float dot(const Vec3& other) const noexcept {
        return x * other.x + y * other.y + z * other.z;
    }

    [[nodiscard]] constexpr Vec3 cross(const Vec3& other) const noexcept {
        return {
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x,
        };
    }

    [[nodiscard]] bool nearly_equals(
        const Vec3& other,
        float epsilon = 1.0e-5f
    ) const noexcept {
        return std::fabs(x - other.x) <= epsilon &&
               std::fabs(y - other.y) <= epsilon &&
               std::fabs(z - other.z) <= epsilon;
    }
};

[[nodiscard]] constexpr inline Vec3 operator*(float scalar, const Vec3& value) noexcept {
    return value * scalar;
}

struct Transform {
    Vec3 position{};
    Vec3 rotation{};  // degrees: x=pitch, y=yaw, z=roll
    Vec3 scale{1.0f, 1.0f, 1.0f};

    constexpr void translate(const Vec3& delta) noexcept { position += delta; }
    constexpr void rotate(const Vec3& delta_degrees) noexcept {
        rotation += delta_degrees;
    }
};

struct AABB {
    Vec3 min{};
    Vec3 max{};

    [[nodiscard]] constexpr bool contains(const Vec3& point) const noexcept {
        return point.x >= min.x && point.x <= max.x &&
               point.y >= min.y && point.y <= max.y &&
               point.z >= min.z && point.z <= max.z;
    }
};

[[nodiscard]] constexpr inline bool intersects(
    const AABB& a,
    const AABB& b
) noexcept {
    return a.min.x <= b.max.x && a.max.x >= b.min.x &&
           a.min.y <= b.max.y && a.max.y >= b.min.y &&
           a.min.z <= b.max.z && a.max.z >= b.min.z;
}

struct BoxCollider {
    Vec3 half_extents{0.5f, 0.5f, 0.5f};
    Vec3 offset{};

    constexpr BoxCollider() = default;
    constexpr explicit BoxCollider(
        Vec3 half_extents_value,
        Vec3 offset_value = {}
    )
        : half_extents(half_extents_value), offset(offset_value) {}
};

class GameObject {
public:
    using Id = std::uint64_t;
    using UpdateCallback = std::function<void(GameObject&, float)>;

    Transform transform{};
    Vec3 velocity{};
    Vec3 acceleration{};
    bool active = true;
    UpdateCallback on_update{};

    [[nodiscard]] Id id() const noexcept { return id_; }
    [[nodiscard]] const std::string& name() const noexcept { return name_; }

    void set_name(std::string name) { name_ = std::move(name); }

    void add_tag(std::string tag) { tags_.insert(std::move(tag)); }
    void remove_tag(const std::string& tag) { tags_.erase(tag); }

    [[nodiscard]] bool has_tag(const std::string& tag) const {
        return tags_.find(tag) != tags_.end();
    }

    template <typename T, typename... Args>
    T& add_component(Args&&... args) {
        static_assert(
            !std::is_reference_v<T>,
            "Component type must not be a reference"
        );

        const std::type_index key(typeid(T));
        auto value = std::any(T(std::forward<Args>(args)...));
        auto it = components_.find(key);

        if (it == components_.end()) {
            it = components_.emplace(key, std::move(value)).first;
        } else {
            it->second = std::move(value);
        }

        return std::any_cast<T&>(it->second);
    }

    template <typename T>
    [[nodiscard]] bool has_component() const {
        return components_.find(std::type_index(typeid(T))) != components_.end();
    }

    template <typename T>
    [[nodiscard]] T* get_component() noexcept {
        const auto it = components_.find(std::type_index(typeid(T)));
        if (it == components_.end()) {
            return nullptr;
        }
        return std::any_cast<T>(&it->second);
    }

    template <typename T>
    [[nodiscard]] const T* get_component() const noexcept {
        const auto it = components_.find(std::type_index(typeid(T)));
        if (it == components_.end()) {
            return nullptr;
        }
        return std::any_cast<T>(&it->second);
    }

    template <typename T>
    bool remove_component() {
        return components_.erase(std::type_index(typeid(T))) != 0;
    }

private:
    friend class Scene;

    explicit GameObject(Id id, std::string name)
        : id_(id), name_(std::move(name)) {}

    Id id_ = 0;
    std::string name_;
    std::unordered_set<std::string> tags_;
    std::unordered_map<std::type_index, std::any> components_;
};

[[nodiscard]] inline AABB world_aabb(
    const GameObject& object,
    const BoxCollider& collider
) noexcept {
    const Vec3 abs_scale{
        std::fabs(object.transform.scale.x),
        std::fabs(object.transform.scale.y),
        std::fabs(object.transform.scale.z),
    };

    const Vec3 half{
        collider.half_extents.x * abs_scale.x,
        collider.half_extents.y * abs_scale.y,
        collider.half_extents.z * abs_scale.z,
    };

    const Vec3 center = object.transform.position + collider.offset;
    return {center - half, center + half};
}

[[nodiscard]] inline bool collides_with(
    const GameObject& a,
    const GameObject& b
) noexcept {
    const auto* a_collider = a.get_component<BoxCollider>();
    const auto* b_collider = b.get_component<BoxCollider>();

    if (a_collider == nullptr || b_collider == nullptr) {
        return false;
    }

    return intersects(
        world_aabb(a, *a_collider),
        world_aabb(b, *b_collider)
    );
}

class Scene {
public:
    using Id = GameObject::Id;

    GameObject& create(std::string name = {}) {
        auto object = std::unique_ptr<GameObject>(
            new GameObject(next_id_++, std::move(name))
        );
        auto& reference = *object;
        objects_.push_back(std::move(object));
        return reference;
    }

    [[nodiscard]] GameObject* find(Id id) noexcept {
        for (auto& object : objects_) {
            if (object->id() == id) {
                return object.get();
            }
        }
        return nullptr;
    }

    [[nodiscard]] const GameObject* find(Id id) const noexcept {
        for (const auto& object : objects_) {
            if (object->id() == id) {
                return object.get();
            }
        }
        return nullptr;
    }

    [[nodiscard]] GameObject* find(const std::string& name) noexcept {
        for (auto& object : objects_) {
            if (object->name() == name) {
                return object.get();
            }
        }
        return nullptr;
    }

    [[nodiscard]] const GameObject* find(
        const std::string& name
    ) const noexcept {
        for (const auto& object : objects_) {
            if (object->name() == name) {
                return object.get();
            }
        }
        return nullptr;
    }

    [[nodiscard]] std::vector<GameObject*> with_tag(
        const std::string& tag
    ) {
        std::vector<GameObject*> result;
        for (auto& object : objects_) {
            if (object->has_tag(tag)) {
                result.push_back(object.get());
            }
        }
        return result;
    }

    [[nodiscard]] std::vector<const GameObject*> with_tag(
        const std::string& tag
    ) const {
        std::vector<const GameObject*> result;
        for (const auto& object : objects_) {
            if (object->has_tag(tag)) {
                result.push_back(object.get());
            }
        }
        return result;
    }

    bool destroy(Id id) {
        if (updating_) {
            if (find(id) == nullptr) {
                return false;
            }
            pending_destroy_.insert(id);
            return true;
        }
        return erase_now(id);
    }

    bool destroy(GameObject& object) { return destroy(object.id()); }

    void clear() {
        if (updating_) {
            for (const auto& object : objects_) {
                pending_destroy_.insert(object->id());
            }
            return;
        }

        objects_.clear();
        pending_destroy_.clear();
    }

    void update(float dt) {
        if (dt < 0.0f) {
            throw std::invalid_argument("Scene::update dt must be >= 0");
        }

        updating_ = true;

        for (auto& object : objects_) {
            if (!object->active ||
                pending_destroy_.count(object->id()) != 0) {
                continue;
            }

            if (object->on_update) {
                object->on_update(*object, dt);
            }

            object->velocity += object->acceleration * dt;
            object->transform.position += object->velocity * dt;
        }

        updating_ = false;
        flush_destroyed();
    }

    template <typename Fn>
    void for_each(Fn&& fn) {
        for (auto& object : objects_) {
            if (pending_destroy_.count(object->id()) == 0) {
                fn(*object);
            }
        }
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return objects_.size() - pending_destroy_.size();
    }

    [[nodiscard]] bool empty() const noexcept {
        return size() == 0;
    }

private:
    bool erase_now(Id id) {
        const auto it = std::find_if(
            objects_.begin(),
            objects_.end(),
            [id](const std::unique_ptr<GameObject>& object) {
                return object->id() == id;
            }
        );

        if (it == objects_.end()) {
            return false;
        }

        objects_.erase(it);
        pending_destroy_.erase(id);
        return true;
    }

    void flush_destroyed() {
        if (pending_destroy_.empty()) {
            return;
        }

        objects_.erase(
            std::remove_if(
                objects_.begin(),
                objects_.end(),
                [this](const std::unique_ptr<GameObject>& object) {
                    return pending_destroy_.count(object->id()) != 0;
                }
            ),
            objects_.end()
        );

        pending_destroy_.clear();
    }

    std::vector<std::unique_ptr<GameObject>> objects_;
    std::unordered_set<Id> pending_destroy_;
    Id next_id_ = 1;
    bool updating_ = false;
};

class Engine {
public:
    using StepCallback = std::function<void(Engine&, float)>;

    [[nodiscard]] Scene& scene() noexcept { return scene_; }
    [[nodiscard]] const Scene& scene() const noexcept { return scene_; }

    void step(float dt) {
        if (dt < 0.0f) {
            throw std::invalid_argument("Engine::step dt must be >= 0");
        }

        const float scaled_dt = paused_ ? 0.0f : dt * time_scale_;

        if (before_step) {
            before_step(*this, scaled_dt);
        }

        if (!paused_) {
            scene_.update(scaled_dt);
            elapsed_seconds_ += static_cast<double>(scaled_dt);
        }

        if (after_step) {
            after_step(*this, scaled_dt);
        }

        ++frame_;
    }

    void set_time_scale(float scale) {
        if (scale < 0.0f) {
            throw std::invalid_argument("time scale must be >= 0");
        }
        time_scale_ = scale;
    }

    [[nodiscard]] float time_scale() const noexcept { return time_scale_; }

    void set_paused(bool paused) noexcept { paused_ = paused; }
    [[nodiscard]] bool paused() const noexcept { return paused_; }

    [[nodiscard]] std::uint64_t frame() const noexcept { return frame_; }

    [[nodiscard]] double elapsed_seconds() const noexcept {
        return elapsed_seconds_;
    }

    StepCallback before_step{};
    StepCallback after_step{};

private:
    Scene scene_{};
    float time_scale_ = 1.0f;
    bool paused_ = false;
    std::uint64_t frame_ = 0;
    double elapsed_seconds_ = 0.0;
};

}  // namespace olj::game
