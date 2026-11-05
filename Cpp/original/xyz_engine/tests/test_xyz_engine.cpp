#include "xyz_engine.hpp"

#include <cassert>

struct Health {
    int value;
    explicit Health(int initial) : value(initial) {}
};

int main() {
    using namespace olj::game;

    {
        const Vec3 a{1.0f, 2.0f, 3.0f};
        const Vec3 b{4.0f, 5.0f, 6.0f};

        assert((a + b) == Vec3(5.0f, 7.0f, 9.0f));
        assert(a.dot(b) == 32.0f);
        assert(a.cross(b) == Vec3(-3.0f, 6.0f, -3.0f));
        assert(
            Vec3(3.0f, 0.0f, 4.0f)
                .normalized()
                .nearly_equals({0.6f, 0.0f, 0.8f})
        );
    }

    Engine engine;

    auto& player = engine.scene().create("player");
    const auto player_id = player.id();
    player.add_tag("player");
    player.velocity = {2.0f, 0.0f, 0.0f};
    player.acceleration = {0.0f, 2.0f, 0.0f};

    player.on_update = [](GameObject& self, float dt) {
        self.transform.position.z += 3.0f * dt;
    };

    auto& health = player.add_component<Health>(100);
    assert(health.value == 100);
    assert(player.has_component<Health>());
    assert(player.get_component<Health>()->value == 100);

    engine.step(0.5f);

    assert(
        player.transform.position.nearly_equals(
            {1.0f, 0.5f, 1.5f}
        )
    );
    assert(player.velocity.nearly_equals({2.0f, 1.0f, 0.0f}));
    assert(engine.frame() == 1);
    assert(engine.elapsed_seconds() == 0.5);

    assert(engine.scene().find(player_id) == &player);
    assert(engine.scene().find("player") == &player);
    assert(engine.scene().with_tag("player").size() == 1);

    auto& wall = engine.scene().create("wall");
    wall.transform.position = {2.0f, 0.5f, 1.5f};

    player.add_component<BoxCollider>(
        Vec3{0.5f, 0.5f, 0.5f}
    );
    wall.add_component<BoxCollider>(
        Vec3{0.5f, 0.5f, 0.5f}
    );

    assert(collides_with(player, wall));

    wall.transform.position.x = 10.0f;
    assert(!collides_with(player, wall));

    struct Marker {
        int value;
        explicit Marker(int v) : value(v) {}
    };

    auto& doomed = engine.scene().create("doomed");
    const auto doomed_id = doomed.id();
    doomed.add_component<Marker>(1);

    doomed.on_update = [&engine, doomed_id](GameObject&, float) {
        engine.scene().destroy(doomed_id);
    };

    engine.step(0.1f);
    assert(engine.scene().find(doomed_id) == nullptr);

    engine.set_time_scale(2.0f);
    const float before = player.transform.position.x;
    engine.step(0.25f);
    assert(player.transform.position.x > before);

    engine.set_paused(true);
    const auto paused_position = player.transform.position;
    engine.step(1.0f);
    assert(player.transform.position == paused_position);

    assert(player.remove_component<Health>());
    assert(!player.has_component<Health>());

    const auto wall_id = wall.id();
    assert(engine.scene().destroy(wall_id));
    assert(engine.scene().find(wall_id) == nullptr);
}
