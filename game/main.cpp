#include <cstdint>

#include <map>
#include <print>
#include <random>
#include <vector>

#include <SFML/Graphics.hpp>

#include <entt/entity/registry.hpp>

struct TransformC {
    sf::Vector2f position;
    float rotation{0.f};
    sf::Vector2f scale{1.f, 1.f};
};

enum class Direction : uint8_t {
    Up,
    Down,
    Left,
    Right
};

enum class AnimationState : uint8_t {
    Idle,
    Walking,
    Sprinting
};

struct RenderableC {
    sf::Sprite sprite;

    Direction direction{Direction::Down};
    AnimationState animationState{AnimationState::Idle};
};

enum class Action : uint8_t {
    None,
    Up,
    Down,
    Left,
    Right,
    Action1,
    Action2
};

std::map<sf::Keyboard::Key, Action> keyToActionMap = {
    {sf::Keyboard::Key::W, Action::Up},
    {sf::Keyboard::Key::Up, Action::Up},

    {sf::Keyboard::Key::S, Action::Down},
    {sf::Keyboard::Key::Down, Action::Down},

    {sf::Keyboard::Key::A, Action::Left},
    {sf::Keyboard::Key::Left, Action::Left},

    {sf::Keyboard::Key::D, Action::Right},
    {sf::Keyboard::Key::Right, Action::Right},

    {sf::Keyboard::Key::Space, Action::Action1},
    {sf::Keyboard::Key::Enter, Action::Action2}
};

// Multiple input states can be active at the same time.
struct InputState {
    std::map<Action, bool> actionStates = {
        {Action::None, false},
        {Action::Up, false},
        {Action::Down, false},
        {Action::Left, false},
        {Action::Right, false},
        {Action::Action1, false},
        {Action::Action2, false}
    };

    auto key_pressed(const sf::Keyboard::Key key) -> void {
        if (const auto it = keyToActionMap.find(key); it != keyToActionMap.end()) {
            actionStates[it->second] = true;
        }
    }

    auto key_released(const sf::Keyboard::Key key) -> void {
        if (const auto it = keyToActionMap.find(key); it != keyToActionMap.end()) {
            actionStates[it->second] = false;
        }
    }

    auto operator[](const Action action) const -> bool {
        if (const auto it = actionStates.find(action); it != actionStates.end()) {
            return it->second;
        }

        return false;
    }
};

// Only the last input state is active at a time.
struct InputState2 {
    Action currentAction{Action::None};
    std::map<sf::Keyboard::Key, bool> modifiers = {
        {sf::Keyboard::Key::LShift, false},
        {sf::Keyboard::Key::LControl, false}
    };

    auto key_pressed(const sf::Keyboard::Key key) -> void {
        if (modifiers.find(key) != modifiers.end()) {
            modifiers[key] = true;
        }

        if (const auto it = keyToActionMap.find(key); it != keyToActionMap.end()) {
            currentAction = it->second;
        }
    }

    auto key_released(const sf::Keyboard::Key key) -> void {
        if (modifiers.find(key) != modifiers.end()) {
            modifiers[key] = false;
        }

        if (const auto it = keyToActionMap.find(key); it != keyToActionMap.end()) {
            if (currentAction == it->second) {
                currentAction = Action::None;
            }
        }
    }

    auto operator[](const Action action) const -> bool {
        return currentAction == action;
    }
};

auto main(const int32_t, const char**, const char**) -> int32_t {
    sf::RenderWindow window(
        sf::VideoMode({1280, 960}),
        "2D Game"
    );
    window.setFramerateLimit(60);

    entt::registry registry;

    InputState2 inputState;

    sf::Texture playerTexture(std::filesystem::path("textures/test.png"));
    constexpr auto spriteSize = sf::Vector2i{48, 48};

    sf::Sprite playerSprite(playerTexture);
    playerSprite.setTextureRect({{0, 0}, spriteSize});
    playerSprite.setOrigin({spriteSize.x / 2.f, spriteSize.y / 2.f});
    playerSprite.setPosition({window.getSize().x / 2.f, window.getSize().y / 2.f});
    playerSprite.setScale({4.f, 4.f});

    Direction playerDirection{Direction::Down};
    AnimationState playerAnimationState{AnimationState::Idle};

    constexpr float animationFrameDuration = 0.12f;
    size_t animationFrameIndex = 0;
    
    sf::Clock clock;
    sf::Time lastAnimationFrameTime = sf::Time::Zero;
    while (window.isOpen()) {
        sf::Time dt = clock.restart();
        const auto FPS = 1.0 / dt.asSeconds();
        window.setTitle("2D Game - FPS: " + std::to_string(FPS));

        lastAnimationFrameTime += dt;
        if (lastAnimationFrameTime.asSeconds() >= animationFrameDuration) {
            lastAnimationFrameTime = sf::Time::Zero;

            animationFrameIndex += 1;
            // At most 32 frames for an animation, some animations have 4, some have 6 so we need to handle looping there as well.
            animationFrameIndex %= 32;
        }

        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            else if (const auto keyEvent = event->getIf<sf::Event::KeyPressed>()) {
                if (keyEvent->code == sf::Keyboard::Key::Escape)
                    window.close();
                else
                    inputState.key_pressed(keyEvent->code);
            }
            else if (const auto keyEvent = event->getIf<sf::Event::KeyReleased>()) {
                inputState.key_released(keyEvent->code);
            }
            else if (const auto _ = event->getIf<sf::Event::MouseMoved>()) {}
        }

        switch (inputState.currentAction) {
            case Action::Up:
                playerDirection = Direction::Up;
                playerAnimationState = AnimationState::Walking;
                break;
            case Action::Down:
                playerDirection = Direction::Down;
                playerAnimationState = AnimationState::Walking;
                break;
            case Action::Left:
                playerDirection = Direction::Left;
                playerAnimationState = AnimationState::Walking;
                break;
            case Action::Right:
                playerDirection = Direction::Right;
                playerAnimationState = AnimationState::Walking;
                break;
            case Action::None:
                playerAnimationState = AnimationState::Idle;
                break;
            default:
                break;
        }

        if (playerAnimationState == AnimationState::Walking && inputState.modifiers[sf::Keyboard::Key::LShift]) {
            playerAnimationState = AnimationState::Sprinting;
        }

        constexpr float playerSpeed_s = 200.f;
        constexpr float playerSprintMultiplier = 1.7f;
        const float delta =
            playerSpeed_s * dt.asSeconds();
        const float movement =
            playerAnimationState == AnimationState::Walking ? delta :
            playerAnimationState == AnimationState::Sprinting ? delta * playerSprintMultiplier :
            0.f;

        switch (playerDirection) {
            case Direction::Up:
                playerSprite.move({0.f, -movement});
                break;
            case Direction::Down:
                playerSprite.move({0.f, movement});
                break;
            case Direction::Left:
                playerSprite.move({-movement, 0.f});
                break;
            case Direction::Right:
                playerSprite.move({movement, 0.f});
                break;
        }

        constexpr size_t idle_frame_count = 4;
        constexpr size_t walking_frame_count = 6;
        const size_t currentAnimationFrameCount =
            playerAnimationState == AnimationState::Idle
                ? idle_frame_count
                : walking_frame_count;

        const std::map<Direction, size_t> directionOffset = {
            {Direction::Down, 0},
            {Direction::Up, 1},
            {Direction::Left, 2},
            {Direction::Right, 3}
        };

        const std::map<AnimationState, size_t> animationStateOffset = {
            {AnimationState::Idle, 0},
            {AnimationState::Walking, 4 * idle_frame_count},
            {AnimationState::Sprinting, 4 * idle_frame_count + 4 * walking_frame_count}
        };

        const int32_t animationIndex =
            (animationFrameIndex % currentAnimationFrameCount) +
            animationStateOffset.at(playerAnimationState) +
            (directionOffset.at(playerDirection) * currentAnimationFrameCount);

        constexpr int32_t framesPerRow = 8;
        const sf::Vector2i framePosition = {
            (animationIndex % framesPerRow) * spriteSize.x,
            (animationIndex / framesPerRow) * spriteSize.y
        };

        playerSprite.setTextureRect({
            framePosition,
            spriteSize
        });

        window.clear();
        window.draw(playerSprite);
        window.display();
    }

    return 0;
}