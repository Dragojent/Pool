#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <list>

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

class Ball : public sf::Drawable
{
    public:
        Ball(sf::Vector2f position, unsigned int id) :
            m_position(position), m_renderer(sf::CircleShape(10, 50)), m_ID(id) 
        {
            switch (m_ID)
            {
            case 1:
                m_renderer.setFillColor(sf::Color::White);
                break;
            
            default:
                m_renderer.setFillColor(sf::Color::Red);
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

        sf::Vector2f move(std::list<Ball> &balls)
        {
            updateAcceleration();
            updateVelocity();
            if (length(m_velocity) != 0)
                checkCollision(balls);

            m_position += m_velocity;
            m_renderer.setPosition(m_position.x - m_renderer.getRadius(), m_position.y - m_renderer.getRadius());

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
            target.draw(m_renderer);
        }

        sf::CircleShape getRenderer() const
        {
            return m_renderer;
        }

        unsigned int id()
        {
            return m_ID;
        }

    private:
        void checkCollision(std::list<Ball> &balls)
        {
            for (auto &ball : balls)
            {
                if (m_ID == ball.id())
                    continue;

                sf::Vector2f dis{m_position.x - ball.position().x, m_position.y - ball.position().y};
                if (length(dis) < m_renderer.getRadius() * 2)
                    collide(ball);
            }
        }

        void collide(Ball &ball)
        {
            sf::Vector2f dis{m_position.x - ball.position().x, m_position.y - ball.position().y};
            normalize(dis);

            float tranferedForce = 0.6f;
            float impactForce = length(m_velocity) * tranferedForce;

            m_velocity *= 0.7f;

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

        sf::Vector2f m_velocity;
        sf::Vector2f m_acceleration;
        sf::Vector2f m_position;
        sf::CircleShape m_renderer;
        unsigned int m_ID;
};

void moveBalls(std::list<Ball> &balls)
{
    for (auto &ball : balls)
        ball.move(balls);
}

void renderBalls(std::list<Ball> &balls, sf::RenderWindow &win)
{
    for (auto &ball: balls)
        win.draw(ball);
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
        moveBalls(balls);
        win.clear();
        renderBalls(balls, win);
        win.display();
    }
    return 0;
}