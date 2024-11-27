#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include<vector>
#include<iostream>
#define GRAVITATIONAL_FORCE 0.1f
#define SUN_MASS 1000
#define SCALE 30

class Planet {
public:
    b2Body* body;
    sf::CircleShape shape;
    std::vector<sf::Vector2f> trail; // Add trail member

    Planet(b2World& world, sf::Vector2f mousepos, b2Body* sun) {
        b2BodyDef PlanetDef;
        PlanetDef.type = b2_dynamicBody;
        PlanetDef.position.Set(mousepos.x / 30.0f, mousepos.y / 30.0f); // Position above the sun
        body = world.CreateBody(&PlanetDef);
        b2CircleShape circl;
        circl.m_radius = 1.0f; // Radius of the planet
        b2FixtureDef fixtureDf;
        fixtureDf.shape = &circl;
        fixtureDf.density = 0.9f; // Mass density
        fixtureDf.friction = 0.3f; // Friction
        body->CreateFixture(&fixtureDf);

        // Calculate initial velocity
        b2Vec2 direction = body->GetPosition() - sun->GetPosition();
        float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        float orbitalSpeed = std::sqrt(GRAVITATIONAL_FORCE * SUN_MASS / distance);
        b2Vec2 tangentVelocity(-direction.y, direction.x);
        tangentVelocity.Normalize();
        tangentVelocity *= orbitalSpeed;
        body->SetLinearVelocity(tangentVelocity);

        shape.setRadius(circl.m_radius * 30.0f); // Planet is a circle shape with radius 30
        shape.setFillColor(sf::Color::Green);
        shape.setOrigin(circl.m_radius * 30, circl.m_radius * 30);
        shape.setPosition(mousepos);
    };

    void update() {
        sf::Vector2f pos(body->GetPosition().x * 30, body->GetPosition().y * 30);
        shape.setPosition(pos);
        trail.push_back(pos);
        if (trail.size() > 100) {
            trail.erase(trail.begin());
        }
    };

    void ApplyGravity(b2Body* sun) {
        b2Vec2 dir = sun->GetPosition() - body->GetPosition();
        float forceMagnitude = GRAVITATIONAL_FORCE * SUN_MASS * body->GetMass() / (dir.Length() * dir.Length());
        dir.Normalize();
        b2Vec2 force = forceMagnitude * dir;
        body->ApplyForceToCenter(force, true);
    }
};

bool isCollision(b2Body* body1, float radius1, b2Body* body2, float radius2) { 
    b2Vec2 pos1 = body1->GetPosition(); b2Vec2 pos2 = body2->GetPosition();
    float distance = std::sqrt((pos1.x - pos2.x) * (pos1.x - pos2.x) + (pos1.y - pos2.y) * (pos1.y - pos2.y));
    return distance < (radius1 + radius2);
}

std::vector<Planet*> Planets;

int main() {
    // SFML Setup
    sf::RenderWindow window(sf::VideoMode(1000, 800), "Box2D + SFML Test");
    window.setFramerateLimit(60);

    // Box2D World Setup
    b2Vec2 gravity(0.0f, 0.0f); // Gravity vector
    b2World world(gravity);

    // Sun (Static)
    b2BodyDef SunDef;
    SunDef.position.Set(500.0f / 30.0f, 400.0f / 30.0f); // Position at the center
    b2Body* Sun = world.CreateBody(&SunDef);
    b2CircleShape circle;
    circle.m_radius = 2.0f; // Radius of the sun
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &circle;
    fixtureDef.density = 0.0f; // Mass density 
    fixtureDef.friction = 0.3f; // Friction 
    Sun->CreateFixture(&fixtureDef);

    // SFML Shapes for Rendering
    sf::CircleShape center(60.f); // Sun is a circle shape with radius 60
    center.setFillColor(sf::Color::Yellow);
    center.setOrigin(60.f, 60.f);
    center.setPosition(500.0f, 400.0f); // Center of the screen

    // Main Loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::MouseButtonPressed) {
                Planet* spawn = new Planet(world, (sf::Vector2f)sf::Mouse::getPosition(window), Sun);
                Planets.push_back(spawn);
            }
        }

        // Box2D Simulation Step
        float timeStep = 1.0f / 60.0f; // 60 FPS
        int32 velocityIterations = 8;
        int32 positionIterations = 3;
        world.Step(timeStep, velocityIterations, positionIterations);

        // Rendering
        window.clear();

        for (auto it = Planets.begin(); it != Planets.end();) {
            Planet* p = *it;
            p->update();
            p->ApplyGravity(Sun);

            if (isCollision(p->body, 1.0f, Sun, 2.0f)) {
                world.DestroyBody(p->body);
                delete p;
                it = Planets.erase(it);
            }
            else {
                window.draw(p->shape);
                for (const auto& trailPos : p->trail) {
                    sf::CircleShape trailDot(2.f);
                    trailDot.setFillColor(sf::Color::White);
                    trailDot.setPosition(trailPos);
                    window.draw(trailDot);
                }
                ++it;
            }
        }

        window.draw(center);
        window.display();
    }
}