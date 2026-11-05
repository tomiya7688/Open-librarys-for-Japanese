#include <iostream>

#include "xyz_engine.hpp"

int main() {
    olj::game::Engine engine;

    auto& player = engine.scene().create("player");
    player.add_tag("controllable");
    player.transform.position = {0.0f, 1.0f, 0.0f};
    player.velocity = {2.0f, 0.0f, 0.0f};
    player.add_component<olj::game::BoxCollider>(
        olj::game::Vec3{0.5f, 0.5f, 0.5f}
    );

    auto& wall = engine.scene().create("wall");
    wall.transform.position = {3.0f, 1.0f, 0.0f};
    wall.add_component<olj::game::BoxCollider>(
        olj::game::Vec3{0.5f, 1.0f, 0.5f}
    );

    for (int i = 0; i < 3; ++i) {
        engine.step(0.5f);

        std::cout
            << "player: ("
            << player.transform.position.x << ", "
            << player.transform.position.y << ", "
            << player.transform.position.z << ")\n";

        if (olj::game::collides_with(player, wall)) {
            std::cout << "wall hit!\n";
            player.velocity = {};
        }
    }
}
