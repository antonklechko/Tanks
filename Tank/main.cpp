#include <iostream>
#include <SFML/Graphics.hpp>
#include "map.h";
#include <SFML/OpenGL.hpp>
#include "view.h"
#include <vector>
#include "list"

using namespace sf;


class Entity
{
public:
    float x, y, vx, vy, h, w, speed = 1;
    bool life;
    int dir;
    Image image;
    Texture texture;
    Sprite sprite;
    String file;

    Entity(Image& image, int H, int W, int X, int Y)
    {
        dir = 0;
        x = X; y = Y;
        h = H; w = W;
        speed = 0;
        life = true;

        texture.loadFromImage(image);
        sprite.setTexture(texture);
        sprite.setOrigin(w / 2, h / 2);
    }

    virtual void update(float time) = 0;
};


class Tank :public Entity
{
public:
    bool isShot;
    enum { left, right, up, down, stay }state;
    Tank(Image& image, int H, int W, int X, int Y) :Entity(image, W, H, X, Y)
    {
        isShot = false;
        state = up;
        sprite.setTextureRect(IntRect(0, 0, 32, 32));
        sprite.setPosition(x + w / 2, y + h / 2);
    }
    void control()
    {
        if (Keyboard::isKeyPressed)
        {
            if (Keyboard::isKeyPressed(Keyboard::A) || Keyboard::isKeyPressed(Keyboard::Left))
            {
                state = left;
                speed = 0.1;
            }
            if (Keyboard::isKeyPressed(Keyboard::D) || Keyboard::isKeyPressed(Keyboard::Right))
            {
                state = right;
                speed = 0.1;
            }
            if (Keyboard::isKeyPressed(Keyboard::W) || Keyboard::isKeyPressed(Keyboard::Up))
            {
                state = up;
                speed = 0.1;
            }
            if (Keyboard::isKeyPressed(Keyboard::S) || Keyboard::isKeyPressed(Keyboard::Down))
            {
                state = down;
                speed = 0.1;
            }
            if (Keyboard::isKeyPressed(Keyboard::Space))
            {
                isShot = true;
            }
        }
    }

    void update(float time)
    {
        control();
        ColisionWithMap();

        switch (state)
        {
        case left:vx = -speed; vy = 0; sprite.setTextureRect(sf::IntRect(32, 32, 32, 32)); break;
        case right:vx = speed; vy = 0; sprite.setTextureRect(sf::IntRect(0, 32, 32, 32)); break;
        case up:vx = 0; vy = -speed; sprite.setTextureRect(sf::IntRect(0, 0, 32, 32)); break;
        case down:vx = 0; vy = speed; sprite.setTextureRect(sf::IntRect(32, 0, 32, 32)); break;
        case stay:vx = 0; vy = 0; break;
        }

        x += vx * time;
        y += vy * time;
        speed = 0;
        getCoordinateView(x, y);
        sprite.setPosition(x + w / 2, y + h / 2);
    }

    void ColisionWithMap()
    {
        for (int i = y / 32; i < (y + h) / 32; ++i)
            for (int j = x / 32; j < (x + w) / 32; ++j)
            {
                if (TileMap[i][j] == '0' || TileMap[i][j] == '1')
                {
                    if (vy < 0)
                        y = i * 32 + 32;
                    if (vy > 0)
                        y = i * 32 - h;
                    if (vx > 0)
                        x = j * 32 - w;
                    if (vx < 0)
                        x = j * 32 + 32;
                }
                if (TileMap[i][j] == '.')
                {
                    speed /= 1.3;
                }
            }
    }
};

class Bullet :public Entity
{
public:
    Bullet(Image& image, int H, int W, float X, float Y, int direction) :Entity(image, H, W, X, Y)
    {
        float speed = 0.3;
        dir = direction;

        sprite.setTextureRect(IntRect(0, 0, 6, 8));
        switch (dir)
        {
        case 0:x -= 16; y++; break;
        case 1:x += 14; y++; break;
        case 2:y -= 15; break;
        case 3:y += 15; break;
        }
    }

    void update(float time)
    {
        speed = 0.2;
        switch (dir)
        {
        case 0:vx = -speed; vy = 0; sprite.setTextureRect(sf::IntRect(6, 6, 8, 6)); break;
        case 1:vx = speed; vy = 0; sprite.setTextureRect(sf::IntRect(6, 0, 8, 6));  break;
        case 2:vx = 0; vy = -speed; break;
        case 3:vx = 0; vy = speed; sprite.setTextureRect(sf::IntRect(0, 8, 6, 8)); break;
        }

        x += vx * time;
        y += vy * time;

        sprite.setPosition(x, y);
        ColisionWithMap();
    }

    void ColisionWithMap()
    {
        for (int i = (y) / 32; i < (y + h - 9) / 32; ++i)
            for (int j = (x) / 32; j < (x + w - 5) / 32; ++j)
            {
                if (TileMap[i][j] == '0' || TileMap[i][j] == '1')
                {
                    life = false;
                }
            }
    }

};

int main()
{
    RenderWindow window(VideoMode(1024, 768), "Tanki");
    window.setVerticalSyncEnabled(true);
    view.reset(FloatRect(480, 480, 640, 480));

    std::list<Entity*> entities;
    std::list<Entity*>::iterator it;

    Image map_image;
    map_image.loadFromFile("background.png");

    Texture map_texture;
    map_texture.loadFromImage(map_image);

    Sprite map_sprite;
    map_sprite.setTexture(map_texture);

    Image tankImage;
    tankImage.loadFromFile("tank1.png");
    tankImage.createMaskFromColor(Color(255, 255, 255));

    Image bulletImage;
    bulletImage.loadFromFile("bullet.png");
    bulletImage.createMaskFromColor(Color(255, 255, 255));

    Tank tank(tankImage, 32, 32, 480, 480);

    Clock clock;

    float reloadTime = 0;

    while (window.isOpen())
    {
        float time = clock.getElapsedTime().asMicroseconds();
        clock.restart();
        time = time / 500;
        reloadTime += time;

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            tank.update(time);

            if (tank.isShot && reloadTime >= 750)
            {
                reloadTime = 0;
                entities.push_back(new Bullet(bulletImage, 8, 6, tank.x + 16, tank.y + 16, tank.state));
            }
            tank.isShot = false;
        }

        for (it = entities.begin(); it != entities.end();)
        {
            Entity* b = *it;
            b->update(time);
            if (b->life == false) {
                it = entities.erase(it);
                delete b;
            }
            else ++it;
        }

        window.clear();

        for (int i = 0; i < HEIGHT_MAP; ++i)
            for (int j = 0; j < WIDHT_MAP; ++j)
            {
                if (TileMap[i][j] == ' ')
                    map_sprite.setTextureRect(IntRect(0, 96, 32, 32));
                if (TileMap[i][j] == '0')
                    map_sprite.setTextureRect(IntRect(160, 0, 32, 32));
                if (TileMap[i][j] == '1')
                    map_sprite.setTextureRect(IntRect(192, 0, 32, 32));
                if (TileMap[i][j] == '.')
                    map_sprite.setTextureRect(IntRect(32, 96, 32, 32));

                map_sprite.setPosition(j * 32, i * 32);
                window.draw(map_sprite);
            }

        window.setView(view);
        window.draw(tank.sprite);
        for (it = entities.begin(); it != entities.end(); ++it) {
            window.draw((*it)->sprite);
        }
        window.display();
    }

    return 0;
}