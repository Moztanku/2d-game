#include <cstdint>
#include <random>

#include <SFML/Graphics.hpp>

#include <entt/entity/registry.hpp>

struct position {
    float x;
    float y;
};

struct velocity {
    float x;
    float y;
};

struct color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

auto update(entt::registry& registry, const sf::Window& window) -> void {
    const auto [w, h] = window.getSize();

    registry.view<position, velocity>().each([w, h](auto& pos, auto& vel) {
        pos.x += vel.x;
        pos.y += vel.y;

        if (pos.x < 0.f || pos.x > w) {
            vel.x = -vel.x;
            pos.x += vel.x;
        }

        if (pos.y < 0.f || pos.y > h) {
            vel.y = -vel.y;
            pos.y += vel.y;
        }
    });
}

auto draw(const entt::registry& registry, sf::RenderWindow& window) -> void {
    constexpr float RADIUS = 5.f;
    constexpr float ALPHA = 1.0f;

    sf::CircleShape shape(RADIUS);
    shape.setOrigin({RADIUS, RADIUS});

    registry.view<position, color>().each([&window, &shape](const auto& pos, const auto& col) {
        shape.setPosition({pos.x, pos.y});
        shape.setFillColor(sf::Color(col.r, col.g, col.b, static_cast<uint8_t>(ALPHA * 255)));
        window.draw(shape);
    });
}

auto main(const int32_t, const char**, const char**) -> int32_t {
    sf::RenderWindow window(
        sf::VideoMode({1280, 960}),
        "2D Game"
    );
    window.setFramerateLimit(60);

    entt::registry registry;

    const size_t NUM_BALLS = 8'000;
    std::mt19937 rng{std::random_device{}()};

    for (size_t i = 0; i < NUM_BALLS; i++) {
        const auto e = registry.create();

        const auto x = std::uniform_real_distribution<float>{0.f, static_cast<float>(window.getSize().x)}(rng);
        const auto y = std::uniform_real_distribution<float>{0.f, static_cast<float>(window.getSize().y)}(rng);
        registry.emplace<position>(e, x, y);

        const auto vx = std::uniform_real_distribution<float>{-10.f, 10.f}(rng);
        const auto vy = std::uniform_real_distribution<float>{-10.f, 10.f}(rng);
        registry.emplace<velocity>(e, vx, vy);

        const auto r = std::uniform_int_distribution<uint8_t>{0, 255}(rng);
        const auto g = std::uniform_int_distribution<uint8_t>{0, 255}(rng);
        const auto b = std::uniform_int_distribution<uint8_t>{0, 255}(rng);
        registry.emplace<color>(e, r, g, b);
    }
    
    sf::Clock clock;
    while (window.isOpen()) {
        sf::Time dt = clock.restart();
        const auto FPS = 1.0 / dt.asSeconds();
        window.setTitle("2D Game - FPS: " + std::to_string(FPS));

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto mousePress = event->getIf<sf::Event::MouseButtonPressed>()) {
                const auto [x, y] = mousePress->position;

                registry.view<position>().each([x, y](auto& pos) {
                    pos = {static_cast<float>(x), static_cast<float>(y)};
                });
            }
        }

        update(registry, window);

        window.clear();
        draw(registry, window);
        window.display();
    }

    return 0;
}