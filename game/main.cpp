#include <cstdint>

#include <map>
#include <print>
#include <random>
#include <vector>
#include <string>
#include <string_view>

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include <entt/entt.hpp>

struct TransformC {
    sf::Vector2f position;
    float rotation{0.f};
    sf::Vector2f scale{1.f, 1.f};
};

struct VelocityC {
    sf::Vector2f velocity;
};

// Just a marker component.
struct PlayerC {};
struct CharacterC {};

struct MovementStatsC {
    float walkSpeed{200.f};
    float runSpeed{340.f};

    bool canRun{true};
};

enum class Direction : uint8_t {
    Up,
    Down,
    Left,
    Right
};

struct DirectionC {
    Direction direction{Direction::Down};
};

enum class Action : uint8_t {
    Idle,
    Walk,
    Run,
    Attack
};

struct ActionC {
    Action action{Action::Idle};
};

enum class Layer : uint8_t {
    Background,
    Ground,
    Foreground,
    UI
};

struct RenderableC {
    const sf::Texture* texture{nullptr};

    Layer layer{Layer::Foreground};
    sf::Vector2i spriteSize{48, 48};
};

enum class AnimationState : uint8_t {
    Looping,
    Once
};

struct AnimationClip {
    std::vector<size_t> frames;
    float frameDuration{0.12f};
    AnimationState state{AnimationState::Looping};
};

struct AnimationSet {
    std::map<
        std::pair<Action, Direction>,
        AnimationClip
    > animations;
};

constexpr size_t FRAME_END = std::numeric_limits<size_t>::max();
struct AnimationC {
    const AnimationClip* clip{nullptr};
    const AnimationSet* set{nullptr};

    size_t frameIndex{0};
    float elapsedTime{0.f};
};

class ResourceManager {
public:
    static auto get_texture(const std::string& name) -> const sf::Texture& {
        if (const auto it = textures.find(name); it != textures.end()) {
            return it->second;
        }

        // Load the texture if it's not already loaded.
        const auto fullPath = std::filesystem::path("textures") / name;

        sf::Texture texture;
        if (!texture.loadFromFile(fullPath)) {
            throw std::runtime_error("Failed to load texture: " + fullPath.string());
        }

        textures[name] = std::move(texture);

        return textures[name];
    }

    static auto get_animation_set(const std::string& name) -> const AnimationSet& {
        if (const auto it = animationSets.find(name); it != animationSets.end()) {
            return it->second;
        }

        throw std::runtime_error("Animation set not found: " + name);
    }

private:
    static std::map<std::string, sf::Texture> textures;
    static std::map<std::string, AnimationSet> animationSets;
};

auto construct_npc_animation_set() -> AnimationSet {
    AnimationSet set;

    constexpr std::array<Direction, 4> directions = {
        Direction::Down,
        Direction::Up,
        Direction::Left,
        Direction::Right
    };

    struct AnimDef {
        std::vector<size_t> frames;
        float frameDuration;
    };

    const std::unordered_map<Action, std::unordered_map<Direction, AnimDef>> defs = {
        {
            Action::Idle, {
                { Direction::Down,  {{0, 1, 2, 3},        0.14f} },
                { Direction::Up,    {{4, 5, 6, 7},        0.14f} },
                { Direction::Left,  {{8, 9, 10, 11},      0.14f} },
                { Direction::Right, {{12, 13, 14, 15},    0.14f} }
            }
        },
        {
            Action::Walk, {
                { Direction::Down,  {{16, 17, 18, 19, 20, 21}, 0.12f} },
                { Direction::Up,    {{22, 23, 24, 25, 26, 27}, 0.12f} },
                { Direction::Left,  {{28, 29, 30, 31, 32, 33}, 0.12f} },
                { Direction::Right, {{34, 35, 36, 37, 38, 39}, 0.12f} }
            }
        },
        {
            Action::Run, {
                { Direction::Down,  {{40, 41, 42, 43, 44, 45}, 0.10f} },
                { Direction::Up,    {{46, 47, 48, 49, 50, 51}, 0.10f} },
                { Direction::Left,  {{52, 53, 54, 55, 56, 57}, 0.10f} },
                { Direction::Right, {{58, 59, 60, 61, 62, 63}, 0.10f} }
            }
        },
        {
            Action::Attack, {
                { Direction::Down,  {{96, 97, 98, 99},      0.16f} },
                { Direction::Up,    {{100, 101, 102, 103},  0.16f} },
                { Direction::Left,  {{104, 105, 106, 107},  0.16f} },
                { Direction::Right, {{108, 109, 110, 111},  0.16f} }
            }
        }
    };

    for (const auto& [action, dirMap] : defs) {
        for (const auto& dir : directions) {
            const auto& anim = dirMap.at(dir);

            set.animations[{action, dir}] = AnimationClip{
                .frames = anim.frames,
                .frameDuration = anim.frameDuration,
                .state = action == Action::Attack ? AnimationState::Once : AnimationState::Looping
            };
        }
    }

    return set;
}

std::map<std::string, sf::Texture> ResourceManager::textures = {};
std::map<std::string, AnimationSet> ResourceManager::animationSets = {
    {"default_npc", construct_npc_animation_set()}
};

class InputHandler {
public:
    auto key_pressed(const sf::Keyboard::Key key, entt::registry& registry) -> void {
        if (keyStates.find(key) != keyStates.end()) {
            keyStates[key] = true;
        }

        handle_input(registry);
    }
    auto key_released(const sf::Keyboard::Key key, entt::registry& registry) -> void {
        if (keyStates.find(key) != keyStates.end()) {
            keyStates[key] = false;
        }

        handle_input(registry);
    }
private:
    std::map<sf::Keyboard::Key, bool> keyStates = {
        {sf::Keyboard::Key::W, false},
        {sf::Keyboard::Key::Up, false},

        {sf::Keyboard::Key::S, false},
        {sf::Keyboard::Key::Down, false},

        {sf::Keyboard::Key::A, false},
        {sf::Keyboard::Key::Left, false},

        {sf::Keyboard::Key::D, false},
        {sf::Keyboard::Key::Right, false},

        {sf::Keyboard::Key::Space, false},
        {sf::Keyboard::Key::Enter, false},

        {sf::Keyboard::Key::LShift, false},
        {sf::Keyboard::Key::LControl, false}
    };

    auto handle_input(entt::registry& registry) -> void {
        using sf::Keyboard::Key;
        auto view = registry.view<PlayerC>();

        auto any_of = [&](std::initializer_list<Key> keys) -> bool {
            for (const auto& key : keys) {
                if (keyStates[key]) {
                    return true;
                }
            }
            return false;
        };

        for (auto e : view) {
            auto& direction = registry.get<DirectionC>(e);

            if (any_of({Key::W, Key::Up})) {
                direction.direction = Direction::Up;
            } else if (any_of({Key::S, Key::Down})) {
                direction.direction = Direction::Down;
            } else if (any_of({Key::A, Key::Left})) {
                direction.direction = Direction::Left;
            } else if (any_of({Key::D, Key::Right})) {
                direction.direction = Direction::Right;
            }

            if (any_of({Key::W, Key::Up, Key::S, Key::Down, Key::A, Key::Left, Key::D, Key::Right})) {
                if (registry.get<MovementStatsC>(e).canRun && keyStates[Key::LShift]) {
                    registry.get<ActionC>(e).action = Action::Run;
                } else {
                    registry.get<ActionC>(e).action = Action::Walk;
                }
            } else {
                registry.get<ActionC>(e).action = Action::Idle;
            }

            if (any_of({Key::Space})) {
                registry.get<ActionC>(e).action = Action::Attack;
            }
        }
    }
};

auto handle_input(entt::registry& registry, sf::RenderWindow& window, InputHandler& inputHandler) -> void {
    while (const auto event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
        } else if (const auto keyPress = event->getIf<sf::Event::KeyPressed>()) {
            if (keyPress->code == sf::Keyboard::Key::Escape) {
                window.close();
            }
            inputHandler.key_pressed(keyPress->code, registry);
        } else if (const auto keyRelease = event->getIf<sf::Event::KeyReleased>()) {
            inputHandler.key_released(keyRelease->code, registry);
        }
    }
}

auto create_player(entt::registry& registry) -> entt::entity {
    const auto entity = registry.create();

    registry.emplace<TransformC>(entity, TransformC{
        .position = {640.f, 480.f},
        .rotation = 0.f,
        .scale = {4.f, 4.f}
    });
    registry.emplace<VelocityC>(entity, VelocityC{
        .velocity = {0.f, 0.f}
    });

    registry.emplace<PlayerC>(entity);
    registry.emplace<CharacterC>(entity);
    registry.emplace<MovementStatsC>(entity, MovementStatsC{
        .walkSpeed = 200.f,
        .runSpeed = 340.f,
        .canRun = true
    });
    registry.emplace<DirectionC>(entity, DirectionC{
        .direction = Direction::Down
    });
    registry.emplace<ActionC>(entity, ActionC{
        .action = Action::Idle
    });

    registry.emplace<RenderableC>(entity, RenderableC{
        .texture = &ResourceManager::get_texture("test.png"),
        .layer = Layer::Foreground
    });

    const auto& animSet = ResourceManager::get_animation_set("default_npc");
    registry.emplace<AnimationC>(entity, AnimationC{
        .clip = &animSet.animations.at({Action::Idle, Direction::Down}),
        .set = &animSet
    });

    return entity;
}

auto update_animations(entt::registry& registry, const sf::Time& dt) -> void {
    auto view = registry.view<AnimationC>();

    // 1. Move to the next frame
    for (auto e : view) {
        auto& animation = view.get<AnimationC>(e);
        const auto& clip = animation.clip;

        animation.elapsedTime += dt.asSeconds();
        while (animation.elapsedTime >= clip->frameDuration) {
            animation.elapsedTime -= animation.clip->frameDuration;

            animation.frameIndex += 1;
            if (animation.frameIndex >= clip->frames.size()) {
                if (clip->state == AnimationState::Looping) {
                    animation.frameIndex = 0;
                } else if (clip->state == AnimationState::Once) {
                    animation.frameIndex = FRAME_END;
                }
            }
        }
    }

    // 2. Update the clip if the action or direction changed, or animation ended.
    for (auto e : view) {
        auto& animation = view.get<AnimationC>(e);
        const auto& clip = animation.clip;

        if (clip->state == AnimationState::Once && animation.frameIndex != FRAME_END) {
            continue; // Animations marked as "Once" play until their end.
        }

        const auto& action = registry.get<ActionC>(e).action;
        const auto& direction = registry.get<DirectionC>(e).direction;

        const auto* desiredClip = &animation.set->animations.at({action, direction});

        if (animation.clip == desiredClip && animation.frameIndex != FRAME_END) {
            continue; // No change in the clip, so we can skip.
        }

        animation.clip = desiredClip;
        animation.frameIndex = 0;
        animation.elapsedTime = 0.f;
    }
}

auto render(entt::registry& registry, sf::RenderWindow& window) -> void {
    auto view = registry.view<TransformC, RenderableC>();

    for (auto e : view) {
        const auto& [transform, renderable] = view.get<TransformC, RenderableC>(e);
        sf::Sprite sprite(*renderable.texture);

        sprite.setOrigin({
            renderable.spriteSize.x / 2.f,
            renderable.spriteSize.y / 2.f
        });

        if (registry.all_of<AnimationC>(e)) {
            const auto& animation = registry.get<AnimationC>(e);
            const auto& clip = animation.clip;

            if (animation.frameIndex == FRAME_END) {
                throw std::runtime_error("Animation ended but not updated to a new clip.");
            }

            const int32_t animationIndex = clip->frames[animation.frameIndex];
            const int32_t framesPerRow = renderable.texture->getSize().x / renderable.spriteSize.x;

            const sf::Vector2i framePosition = {
                (animationIndex % framesPerRow) * renderable.spriteSize.x,
                (animationIndex / framesPerRow) * renderable.spriteSize.y
            };

            sprite.setTextureRect({
                framePosition,
                renderable.spriteSize
            });
        }

        sprite.setPosition(transform.position);
        sprite.setRotation(sf::degrees(transform.rotation));
        sprite.setScale(transform.scale);
        
        window.draw(sprite);
    }
}

auto main(const int32_t, const char**, const char**) -> int32_t {
    sf::RenderWindow window(
        sf::VideoMode({1280, 960}),
        "2D Game"
    );
    window.setFramerateLimit(60);

    entt::registry registry;

    auto _ = create_player(registry);

    InputHandler inputHandler;
    
    sf::Clock clock;
    while (window.isOpen()) {
        sf::Time dt = clock.restart();
        const auto FPS = 1.0 / dt.asSeconds();
        window.setTitle("2D Game - FPS: " + std::to_string(FPS));

        handle_input(registry, window, inputHandler);

        window.clear();

        update_animations(registry, dt);
        render(registry, window);

        window.display();
    }

    return 0;
}