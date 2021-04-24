#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <list>
#include <algorithm>

const int WINDOW_WIDTH = 1200, WINDOW_HEIGHT = 800;
const float FZERO = 0.00001f;

float length(sf::Vector2f v)
{
    return sqrt(v.x * v.x + v.y * v.y);
}

void normalize(sf::Vector2f &v)
{
    v /= (float)length(v);
}

float distance(sf::Vector2f a, sf::Vector2f b)
{
    return sqrt(a.x * b.x - a.y * b.y);
}

class Wall : public sf::Drawable
{
    public:
        Wall(sf::Vector2f a, sf::Vector2f b) :
            m_position(a), m_size(b), m_texture(sf::RectangleShape())
        {
            m_texture.setPosition(m_position);
            m_texture.setSize(m_size);
            m_texture.setFillColor(sf::Color::Magenta);
        }
        Wall() = delete;

        void draw(sf::RenderTarget& target, sf::RenderStates states) const 
        {
            target.draw(m_texture);
        }

        sf::Vector2<sf::Vector2f> position()
        {
            return {m_position, m_size};
        }
    private:
        sf::Vector2f m_position;
        sf::Vector2f m_size;
        sf::RectangleShape m_texture;
};

class Hole : public sf::Drawable
{
    public:
    private:

};

class Ball : public sf::Drawable
{
    public:
        Ball(sf::Vector2f position, unsigned int id) :
            m_position(position), m_radius(10), m_sprite(sf::CircleShape(m_radius, 50)), m_ID(id) 
        {
            switch (m_ID)
            {
            case 1:
                m_sprite.setFillColor(sf::Color::White);
                break;
            
            default:
                m_sprite.setFillColor(sf::Color::Red);
                break;
            }
        };
        ~Ball() {};
        
        sf::Vector2f position() const
        {
            return m_position;
        }

        void push(sf::Vector2f force)
        {
            m_velocity += force;
        }

        sf::Vector2f move(std::list<Ball> &balls, std::list<Wall> &walls)
        {
            updateAcceleration();
            updateVelocity();
            if (length(m_velocity) != 0)
            {
                checkCollision(walls);
                checkCollision(balls);
            }

            m_position += m_velocity;
            m_sprite.setPosition(m_position.x - m_radius, m_position.y - m_radius);

            return m_position;
        }

        void punt(sf::Vector2i mousePos)
        {
            const float MAX_STRENGTH = 30.f;
            const float STRENGTH_INCREASE_SPEED = 0.002f;

            sf::Vector2f dir{};
            dir.x = -(mousePos.x - m_position.x);
            dir.y = -(mousePos.y - m_position.y);
            normalize(dir);

            float strength = 1.f;
            while (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
            {
                if (strength < MAX_STRENGTH)
                    strength += STRENGTH_INCREASE_SPEED;
            }

            m_velocity += dir * strength;
        }

        void draw(sf::RenderTarget& target, sf::RenderStates states) const 
        {
            target.draw(m_sprite);
        }

        sf::CircleShape sprite() const
        {
            return m_sprite;
        }

        unsigned int id() const
        {
            return m_ID;
        }

        float radius() const
        {
            return m_radius;
        }       

    private:
        void checkCollision(std::list<Ball> &balls)
        {
            for (auto &ball : balls)
            {
                if (m_ID == ball.id())
                    continue;

                sf::Vector2f distance{m_position.x - ball.position().x, m_position.y - ball.position().y};
                float distanceSquared = distance.x * distance.x + distance.y * distance.y;
                if (distanceSquared < m_radius * m_radius * 4)
                    collide(ball);
            }
        }

        void checkCollision(std::list<Wall> &walls)
        {
            for (auto &wall : walls)
            {
                float left = wall.position().x.x;
                float right = wall.position().x.x + wall.position().y.x;
                float top = wall.position().x.y;
                float bottom = wall.position().x.y + wall.position().y.y;

                float closestX = std::clamp(m_position.x, left, right);
                float closestY = std::clamp(m_position.y, top, bottom);

                float distanceX = m_position.x - closestX;
                float distanceY = m_position.y - closestY;

                float distanceSquared = (distanceX * distanceX) + (distanceY * distanceY);
                if (distanceSquared < m_radius * m_radius + m_radius * 3.f)
                {
                    if (distanceX == 0)
                        collide(wall, true);
                    else if (distanceY == 0)
                        collide(wall, false);
                }
            }
        }
        
        void collide(Wall &wall, bool horisontal)
        {
            if (horisontal)
                m_velocity.y = -m_velocity.y;
            else 
                m_velocity.x = -m_velocity.x;
        }

        void collide(Ball &ball)
        {
            sf::Vector2f dis{m_position.x - ball.position().x, m_position.y - ball.position().y};
            normalize(dis);

            float tranferedForce = 0.6f;
            float impactForce = length(m_velocity) * tranferedForce;

            m_velocity *= 0.6f;

            ball.push(-dis * impactForce);
        }

        void updateAcceleration()
        {
            const float FRICTION_FACTOR = 0.05f;

            m_acceleration.x = -m_velocity.x * FRICTION_FACTOR;
            m_acceleration.y = -m_velocity.y * FRICTION_FACTOR;
        }

        void updateVelocity()
        {
            m_velocity += m_acceleration;

            if ((m_velocity.x < FZERO && m_velocity.x > FZERO) || (m_velocity.y < FZERO && m_velocity.y > FZERO))
            {
                m_velocity.x = 0;
                m_velocity.y = 0;
            }
        }

        float m_radius;
        unsigned int m_ID;
        sf::Vector2f m_velocity;
        sf::Vector2f m_acceleration;
        sf::Vector2f m_position;
        sf::CircleShape m_sprite;
};

void moveBalls(std::list<Ball> &balls, std::list<Wall> &walls)
{
    for (auto &ball : balls)
        ball.move(balls, walls);
}

void renderBalls(std::list<Ball> &balls, sf::RenderWindow &win)
{
    for (auto &ball: balls)
        win.draw(ball);
}

void renderWalls(std::list<Wall> &walls, sf::RenderWindow &win)
{
    for (auto &wall : walls)
        win.draw(wall);
}

void renderAll(std::list<Wall> &walls, std::list<Ball> &balls, sf::RenderWindow &win)
{
    win.clear();
    renderWalls(walls, win);
    renderBalls(balls, win);
    win.display();
}

void reset(std::list<Ball> &balls)
{
    balls = {
        {{200, 200}, 1},
        {{500, 200}, 2},
        {{520, 210}, 3},
        {{520, 190}, 4},
        {{540, 220}, 5},
        {{540, 200}, 6},
        {{540, 180}, 7},
        {{560, 190}, 8},
        {{560, 210}, 9},
        {{580, 200}, 10},
    };
}

int main()
{
    sf::RenderWindow win(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Window");
    win.setFramerateLimit(60);
    sf::Mouse mouse{};
    win.setKeyRepeatEnabled(false);

    std::list<Ball> balls{};
    reset(balls);

    std::list<Wall> walls{
        {{50, 50},{500, 100}},
    }; 

    while (win.isOpen())
    {
        sf::Event event;
        while (win.pollEvent(event))
        {
            if (event.type == sf::Event::Resized)
            {
                sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                win.setView(sf::View(visibleArea));
            }

            if (event.type == sf::Event::Closed)
                    win.close();
            
            if (event.type == sf::Event::KeyPressed)
            {
                switch (event.key.code)
                {
                    case sf::Keyboard::Escape:
                        win.close();
                        break;

                    case sf::Keyboard::Space:
                        balls.front().punt(mouse.getPosition(win));
                        break;
                    
                    case sf::Keyboard::R:
                        reset(balls);
                        break;

                    default:
                        break;
                }
            }
        }
        moveBalls(balls, walls);
        renderAll(walls, balls, win);
    }
    return 0;
}