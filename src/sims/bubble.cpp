#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <optional>
#include <vector>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <iostream>
#include "math_helpers.cpp"   // has dot(), length(), vec ops

using namespace std;

struct ActiveSound {
    sf::Sound sound;
    float age;

    ActiveSound(const sf::SoundBuffer& buffer, float volume)
        : sound(buffer), age(0.f)
    {
        sound.setVolume(volume);
        sound.play();
    }
};

struct PopRing {
    sf::CircleShape shape;
    float baseRadius = 0.f;
    float age = 0.f;
    float lifetime = 0.35f;
};

struct Bubble {
    sf::CircleShape main;
    sf::CircleShape shine;
    sf::Vector2f velocity;
    float radius;
    bool canMerge;
    float age;

    Bubble(float r, sf::Vector2f pos, sf::Vector2f vel, sf::Color col) {
        radius = r;
        canMerge = true;
        age = 0.f;

        main = sf::CircleShape(radius);
        main.setFillColor(col);
        main.setPosition(pos);

        float shineR = radius * 0.35f;
        shine = sf::CircleShape(shineR);
        shine.setFillColor(sf::Color(220, 240, 255, 180));

        velocity = vel;
    }

    void updateShine(const sf::Vector2u& windowSize) {
        sf::Vector2f pos = main.getPosition();
        sf::Vector2f center = pos + sf::Vector2f(radius, radius);

        float nx = center.x / static_cast<float>(windowSize.x);
        float ny = center.y / static_cast<float>(windowSize.y);
        float t = (nx + ny) * 0.5f;

        const float baseDeg = 25.f;
        const float topDeg  = 80.f;
        float angleDeg = baseDeg + t * (topDeg - baseDeg);
        float angleRad = angleDeg * 3.14159265f / 180.f;

        float shineR = radius * 0.35f;
        float dist = radius - shineR * 1.2f;

        sf::Vector2f dir(std::cos(angleRad), -std::sin(angleRad));

        sf::Vector2f shineCenter = center + dir * dist;
        shine.setPosition(shineCenter - sf::Vector2f(shineR, shineR));
    }
};


int runBouncyBubble() {
    // ---------- Shader ----------
    sf::Shader bubbleShader;
    if (!bubbleShader.loadFromFile("assets(shaders_failing_right_now_dont_try)/bubble.frag", sf::Shader::Type::Fragment)) {
        std::cerr << "Failed to load fragment shader\n";
    }

    const unsigned int WINDOW_WIDTH  = 800;
    const unsigned int WINDOW_HEIGHT = 600;

    sf::RenderWindow window(
        sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}),
        "Inelastic Bouncy Bubbles"
    );
    window.setFramerateLimit(80);

    const float restitution_ball = 0.8f;
    const float restitution_wall = 0.8f;

    std::vector<Bubble>  bubbles;
    std::vector<PopRing> popRings;

    // ---------- Audio ----------
    sf::SoundBuffer popBuffer;
    if (!popBuffer.loadFromFile("assets/pop.wav")) {
        std::cerr << "Failed to load pop sound\n";
    }

    std::vector<ActiveSound> activeSounds;

    auto makeBubble = [&](sf::Vector2f pos, sf::Vector2f vel) {
        int ri = rand() % 20 + 5;
        float r = static_cast<float>(ri);

        int style = rand() % 3;
        int R, G, B, A;

        switch (style) {
            case 0: {
                int base = rand() % 50 + 180;
                R = base;
                G = base + 10;
                B = rand() % 20 + (255 - 20 + 1);
                A = rand() % 40 + 140;
                break;
            }
            case 1: {
                R = rand() % 60 + 80;
                G = rand() % 80 + 170;
                B = 255;
                A = rand() % 40 + 140;
                break;
            }
            case 2: {
                R = rand() % 60 + 40;
                G = rand() % 80 + 80;
                B = rand() % 55 + 200;
                A = rand() % 40 + 140;
                break;
            }
        }

        sf::Color col(
            static_cast<std::uint8_t>(R),
            static_cast<std::uint8_t>(G),
            static_cast<std::uint8_t>(B),
            static_cast<std::uint8_t>(A)
        );

        bubbles.emplace_back(r, pos, vel, col);
    };

    auto makePopRing = [&](const Bubble& b) {
        PopRing ring;

        ring.baseRadius = b.radius * 1.10f;
        ring.shape = sf::CircleShape(ring.baseRadius);
        ring.shape.setOrigin(sf::Vector2f(ring.baseRadius, ring.baseRadius));

        sf::Vector2f center = b.main.getPosition() + sf::Vector2f(b.radius, b.radius);
        ring.shape.setPosition(center);

        sf::Color c = b.main.getFillColor();
        c.a = 200;
        ring.shape.setFillColor(sf::Color(0, 0, 0, 0));
        ring.shape.setOutlineColor(c);
        ring.shape.setOutlineThickness(3.f);

        ring.age = 0.f;
        ring.lifetime = 0.35f;
        popRings.push_back(ring);

        if (popBuffer.getSampleCount() > 0) {
            activeSounds.emplace_back(popBuffer, 60.f);
        }
    };

    int n = rand() % 20 + 100;
    int radius = 20;

    for (int i = 0; i < n; i++) {
        int rand_x  = rand() % (WINDOW_WIDTH  - 2 * radius) + radius;
        int rand_y  = rand() % (WINDOW_HEIGHT - 2 * radius) + radius;
        int rand_vx = (rand() % 200 + 100) * (rand() % 2 ? 1 : -1);
        int rand_vy = (rand() % 200 + 100) * (rand() % 2 ? 1 : -1);

        makeBubble(
            sf::Vector2f(static_cast<float>(rand_x),  static_cast<float>(rand_y)),
            sf::Vector2f(static_cast<float>(rand_vx), static_cast<float>(rand_vy))
        );
    }

    for (auto& b : bubbles) {
        b.updateShine(window.getSize());
    }

    sf::Clock clock;
    float elapsedTime = 0.f;  // <- for shader animation

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        float dt = clock.restart().asSeconds();
        elapsedTime += dt;

        for (auto& b : bubbles) {
            b.age += dt;
        }

        // ---- Movement + walls ----
        for (auto& b : bubbles) {
            sf::Vector2f pos = b.main.getPosition();
            pos += b.velocity * dt;

            if (pos.x < 0) {
                pos.x = 0;
                b.velocity.x = -b.velocity.x * restitution_wall;
            }
            else if (pos.x + 2 * b.radius > WINDOW_WIDTH) {
                pos.x = WINDOW_WIDTH - 2 * b.radius;
                b.velocity.x = -b.velocity.x * restitution_wall;
            }

            if (pos.y < 0) {
                pos.y = 0;
                b.velocity.y = -b.velocity.y * restitution_wall;
            }
            else if (pos.y + 2 * b.radius > WINDOW_HEIGHT) {
                pos.y = WINDOW_HEIGHT - 2 * b.radius;
                b.velocity.y = -b.velocity.y * restitution_wall;
            }

            b.main.setPosition(pos);
        }

        // ---- Global slow-pop ----
        {
            std::vector<Bubble> stillMoving;
            stillMoving.reserve(bubbles.size());

            for (auto& b : bubbles) {
                float speed = length(b.velocity);

                if (b.age > 0.5f && speed < 15.f && (rand() % 5 == 0)) {
                    makePopRing(b);
                } else {
                    stillMoving.push_back(b);
                }
            }

            bubbles.swap(stillMoving);
        }

        // ---- Collision handling ----
        std::vector<bool> alive(bubbles.size(), true);
        std::vector<Bubble> mergedToAdd;

        for (std::size_t i = 0; i < bubbles.size(); i++) {
            if (!alive[i]) continue;

            for (std::size_t j = i + 1; j < bubbles.size(); j++) {
                if (!alive[j]) continue;

                Bubble& A = bubbles[i];
                Bubble& B = bubbles[j];

                sf::Vector2f centerA = A.main.getPosition() + sf::Vector2f(A.radius, A.radius);
                sf::Vector2f centerB = B.main.getPosition() + sf::Vector2f(B.radius, B.radius);

                sf::Vector2f delta = centerB - centerA;
                float dist = length(delta);
                float minDist = A.radius + B.radius;

                if (dist > 0 && dist < minDist) {
                    float speedA = length(A.velocity);
                    float speedB = length(B.velocity);

                    if (A.age > 0.7f && B.age > 0.7f && speedA < 20.f && speedB < 20.f && (rand() % 6 == 0)) {
                        makePopRing(A);
                        makePopRing(B);

                        alive[i] = false;
                        alive[j] = false;
                        continue;
                    }

                    bool pairCanMerge = (A.canMerge && B.canMerge);
                    bool doMerge = pairCanMerge && (rand() % 120 == 0);

                    if (doMerge) {
                        float newRadius = A.radius + B.radius;

                        float wA = A.radius;
                        float wB = B.radius;
                        float wSum = wA + wB;

                        sf::Vector2f newCenter = (centerA * wA + centerB * wB) * (1.f / wSum);
                        sf::Vector2f newPos    = newCenter - sf::Vector2f(newRadius, newRadius);
                        sf::Vector2f newVel    = (A.velocity * wA + B.velocity * wB) * (1.f / wSum);

                        sf::Color cA = A.main.getFillColor();
                        sf::Color cB = B.main.getFillColor();

                        auto blendChannel = [&](std::uint8_t a, std::uint8_t b) -> std::uint8_t {
                            float ca = static_cast<float>(a);
                            float cb = static_cast<float>(b);
                            float v  = (ca * wA + cb * wB) / wSum;
                            if (v < 0.f)   v = 0.f;
                            if (v > 255.f) v = 255.f;
                            return static_cast<std::uint8_t>(v);
                        };

                        sf::Color newCol(
                            blendChannel(cA.r, cB.r),
                            blendChannel(cA.g, cB.g),
                            blendChannel(cA.b, cB.b),
                            blendChannel(cA.a, cB.a)
                        );

                        mergedToAdd.emplace_back(newRadius, newPos, newVel, newCol);
                        mergedToAdd.back().canMerge = false;

                        alive[i] = false;
                        alive[j] = false;
                        continue;
                    }

                    // normal collision
                    sf::Vector2f n = delta / dist;
                    sf::Vector2f rel = B.velocity - A.velocity;
                    float velAlongNormal = dot(rel, n);

                    if (velAlongNormal < 0) {
                        float jImpulse = -(1 + restitution_ball) * velAlongNormal / 2.f;
                        sf::Vector2f impulse = jImpulse * n;
                        A.velocity -= impulse;
                        B.velocity += impulse;
                    }

                    float penetration = minDist - dist;
                    sf::Vector2f correction = 0.5f * penetration * n;
                    A.main.move(-correction);
                    B.main.move( correction);
                }
            }
        }

        {
            std::vector<Bubble> newList;
            newList.reserve(bubbles.size() + mergedToAdd.size());

            for (std::size_t i = 0; i < bubbles.size(); ++i) {
                if (alive[i]) newList.push_back(bubbles[i]);
            }
            for (auto& nb : mergedToAdd) {
                newList.push_back(nb);
            }
            bubbles.swap(newList);
        }

        // ---- Pop ring animation ----
        for (auto& ring : popRings) {
            ring.age += dt;
            float t = ring.age / ring.lifetime;
            if (t > 1.f) t = 1.f;

            float currentRadius = ring.baseRadius * (1.f + 0.4f * t);
            ring.shape.setRadius(currentRadius);
            ring.shape.setOrigin(sf::Vector2f(currentRadius, currentRadius));

            sf::Color oc = ring.shape.getOutlineColor();
            oc.a = static_cast<std::uint8_t>((1.f - t) * 200.f);
            ring.shape.setOutlineColor(oc);
        }

        popRings.erase(
            std::remove_if(popRings.begin(), popRings.end(),
                           [](const PopRing& r) { return r.age >= r.lifetime; }),
            popRings.end()
        );

        // ---- Sound lifetime ----
        for (auto& as : activeSounds) {
            as.age += dt;
        }
        const float maxSoundAge = 1.0f;
        activeSounds.erase(
            std::remove_if(activeSounds.begin(), activeSounds.end(),
                           [maxSoundAge](const ActiveSound& as) {
                               return as.age >= maxSoundAge;
                           }),
            activeSounds.end()
        );

        // ---- Update shines ----
        for (auto& b : bubbles) {
            b.updateShine(window.getSize());
        }

        window.clear(sf::Color(180, 220, 255));

        for (auto& b : bubbles) {
            // bubble center in screen coords
            sf::Vector2f center = b.main.getPosition() + sf::Vector2f(b.radius, b.radius);
            sf::Color col = b.main.getFillColor();

            bubbleShader.setUniform("u_radius", b.radius);
            bubbleShader.setUniform("u_center", center);
            bubbleShader.setUniform("u_color", sf::Glsl::Vec4(col)); // SFML will convert
            bubbleShader.setUniform("u_time", elapsedTime);

            window.draw(b.main, &bubbleShader);   // draw main body with shader
            window.draw(b.shine);                 // keep the little highlight on top
        }

        // pop rings on top
        for (auto& r : popRings) {
            window.draw(r.shape);
        }

        window.display();
    }
    return 0;
}
