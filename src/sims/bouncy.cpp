#include <SFML/Graphics.hpp>
#include <optional>
#include <vector>
#include <cmath>
using namespace std;

struct Ball {
    sf::CircleShape shape;
    sf::Vector2f velocity;
    float radius;
};

sf::Vector2f operator*(const sf::Vector2f& v, float s) {
    return { v.x * s, v.y * s };
}

sf::Vector2f operator*(float s, const sf::Vector2f& v) {
    return { v.x * s, v.y * s };
}

// helpers
float dot(const sf::Vector2f& a, const sf::Vector2f& b) {
    return a.x * b.x + a.y * b.y;
}

float length(const sf::Vector2f& v) {
    return std::sqrt(v.x*v.x + v.y*v.y);
}

int runBouncyBall() {
    const unsigned int WINDOW_WIDTH  = 800;
    const unsigned int WINDOW_HEIGHT = 600;

    sf::RenderWindow window(
        sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}),
        "Inelastic Bouncy Balls"
    );
    window.setFramerateLimit(60);

    const float restitution_ball = 0.8f; // inelastic (1.0 = elastic)
    const float restitution_wall = 0.8f;

    std::vector<Ball> balls;

    auto makeBall = [&](sf::Vector2f pos, sf::Vector2f vel, sf::Color col) {
        Ball b;
        int r = rand() % (8 - 2 + 1) + 2;
        b.radius = r * 10.f;
        b.shape = sf::CircleShape(b.radius);
        b.shape.setFillColor(col);
        b.shape.setPosition(pos);
        r = rand() % 10 + 1;
        int sign = (rand() % 2 == 0) ? 1 : -1;
        b.velocity = sign * r * vel;
        balls.push_back(b);
    };

    makeBall({100,100}, {200,150}, sf::Color::Red);
    makeBall({250,200}, {-180,170}, sf::Color::Green);
    makeBall({500,250}, {220,-160}, sf::Color::Blue);
    makeBall({200,400}, {-150,-220}, sf::Color::Yellow);
    makeBall({200,300}, {200,150}, sf::Color::Cyan);
    makeBall({250,200}, {-180,170}, sf::Color::Magenta);
    makeBall({500,250}, {220,-160}, sf::Color::White);
    makeBall({200,400}, {-150,-220}, sf::Color(std::rand() % 256, std::rand() % 256, std::rand() % 256,128));

    sf::Clock clock;

    while (window.isOpen()) {

        // -------------------- SFML 3 EVENT LOOP --------------------
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        float dt = clock.restart().asSeconds();

        // ----- Move & wall collisions -----
        for (auto& b : balls) {
            sf::Vector2f pos = b.shape.getPosition();
            pos += b.velocity * dt;

            // Wall collisions
            if (pos.x < 0) {
                pos.x = 0;
                b.velocity.x = -b.velocity.x * restitution_wall;
            } 
            else if (pos.x + 2*b.radius > WINDOW_WIDTH) {
                pos.x = WINDOW_WIDTH - 2*b.radius;
                b.velocity.x = -b.velocity.x * restitution_wall;
            }

            if (pos.y < 0) {
                pos.y = 0;
                b.velocity.y = -b.velocity.y * restitution_wall;
            } 
            else if (pos.y + 2*b.radius > WINDOW_HEIGHT) {
                pos.y = WINDOW_HEIGHT - 2*b.radius;
                b.velocity.y = -b.velocity.y * restitution_wall;
            }

            b.shape.setPosition(pos);
        }

        // ----- Ball-to-ball collisions -----
        for (int i = 0; i < balls.size(); i++) {
            for (int j = i + 1; j < balls.size(); j++) {
                Ball& A = balls[i];
                Ball& B = balls[j];

                sf::Vector2f posA = A.shape.getPosition() + sf::Vector2f(A.radius,A.radius);
                sf::Vector2f posB = B.shape.getPosition() + sf::Vector2f(B.radius,B.radius);

                sf::Vector2f delta = posB - posA;
                float dist = length(delta);
                float minDist = A.radius + B.radius;

                if (dist > 0 && dist < minDist) {

                    // Normal vector
                    sf::Vector2f n = delta / dist;

                    // Relative velocity
                    sf::Vector2f rel = B.velocity - A.velocity;
                    float velAlongNormal = dot(rel, n);

                    if (velAlongNormal < 0) {
                        float j = -(1 + restitution_ball) * velAlongNormal / 2.f;
                        sf::Vector2f impulse = j * n;
                        A.velocity -= impulse;
                        B.velocity += impulse;
                    }

                    // Positional correction to avoid overlap
                    float penetration = minDist - dist;
                    sf::Vector2f correction = 0.5f * penetration * n;
                    A.shape.move(-correction);
                    B.shape.move( correction);
                }
            }
        }

        // ----- Render -----
        window.clear(sf::Color::Black);
        for (auto& b : balls)
            window.draw(b.shape);
        window.display();
    }

    return 0;
}
