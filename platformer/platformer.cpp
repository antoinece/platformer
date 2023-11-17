
#ifdef _WIN32
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <SFML/Graphics.hpp>
#include <iostream>
#include <stdio.h>
#include "physique.h"
#include <SFML/Audio.hpp>

#include "tile_map.h"

#define TILEMAP_WIDTH 2000
#define TILEMAP_HEIGHT 45
#define TILE_SIZE_PX 18
#define TILE_SIZE_PX_BACKGROUND 192

bool player_alive = true;
bool player_win = false;
int lives_count = 3;
int death_count = 0;
int coins = 0;
int ten_coins = 0;
int hundred_coins = 0;
sf::Vector2f view_pos(0, 0);




static TileState tilemap_ground[TILEMAP_WIDTH * TILEMAP_HEIGHT] = { kEmpty };
static Items tilemap_item[TILEMAP_WIDTH * TILEMAP_HEIGHT] = { kEmptyItem };
static BackgroundState tilemap_background[TILEMAP_WIDTH * 12] = { kEmptyBackground };

bool in_bounds(sf::Vector2i coords)
{
    return coords.x >= 0
        && coords.x < TILEMAP_WIDTH
        && coords.y >= 0
        && coords.y < TILEMAP_HEIGHT;
}

void save()
{
    FILE* f = fopen("level.data", "wb");
    if (f)
    {
	    fwrite(tilemap_ground, sizeof(tilemap_ground), 1, f);
    	fclose(f);
    }
    FILE* f2 = fopen("background.data", "wb");
    if (f2)
    {
        fwrite(tilemap_background, sizeof(tilemap_background), 1, f2);
        fclose(f2);
    }
    FILE* f3 = fopen("item.data", "wb");
    if (f3)
    {
        fwrite(tilemap_item, sizeof(tilemap_item), 1, f3);
        fclose(f3);
    }
}

void load()
{
    FILE* f = fopen("level.data", "rb");
    if (f)
    {
	    fread(tilemap_ground, sizeof(tilemap_ground), 1, f);
    	fclose(f);
    }
    FILE* f2 = fopen("background.data", "rb");
    if (f2)
    {
        fread(tilemap_background, sizeof(tilemap_background), 1, f2);
        fclose(f2);
    }
    FILE* f3 = fopen("item.data", "rb");
    if (f3)
    {
        fread(tilemap_item, sizeof(tilemap_item), 1, f3);
        fclose(f3);
    }
}

sf::Vector2i pos_to_coords(sf::Vector2f world_position)
{
    sf::Vector2i coord;
    coord.x = std::floor(world_position.x / TILE_SIZE_PX);
    coord.y = std::floor(world_position.y / TILE_SIZE_PX);
    return coord;
}

bool tile_at(sf::Vector2i tile_coord)
{
    if (tile_coord.x < 0 || tile_coord.y < 0 || tile_coord.x >= TILEMAP_WIDTH || tile_coord.y >= TILEMAP_HEIGHT) {
        return true;
    }
    int index = tile_coord.y * TILEMAP_WIDTH + tile_coord.x;
    return tilemap_ground[index];
}

bool collision_check(sf::Vector2f position)
{
    sf::Vector2i coord;
    coord.x = position.x / TILE_SIZE_PX;
    coord.y = position.y / TILE_SIZE_PX;

    if (coord.x < 0 || coord.y < 0 || coord.x >= TILEMAP_WIDTH || coord.y >= TILEMAP_HEIGHT) {
        return true;
    }

    int index = coord.y * TILEMAP_WIDTH + coord.x;
    bool tile_solid = tilemap_ground[index];

    return tile_solid;
}
sf::Texture dirt_and_grass;
sf::Texture dirt;
sf::Texture heart;
sf::Texture empty_heart;
sf::Texture top;
sf::Texture mid_1;
sf::Texture mid_2;
sf::Texture bottom;
sf::Texture diamond;
sf::Texture coin;
sf::Texture n0;
sf::Texture n1;
sf::Texture n2;
sf::Texture n3;
sf::Texture n4;
sf::Texture n5;
sf::Texture n6;
sf::Texture n7;
sf::Texture n8;
sf::Texture n9;
sf::Texture game_over;
sf::Texture win;
sf::Texture flag_up;
sf::Texture flag_down;
sf::Texture player; 
sf::SoundBuffer jump_sound;
sf::SoundBuffer death;
sf::SoundBuffer coin_sound;
void load_texture()
{
    dirt.loadFromFile("sprites/S2.png");
    dirt_and_grass.loadFromFile("sprites/S3.png");
    heart.loadFromFile("sprites/fullHeart.png");
    empty_heart.loadFromFile("sprites/emptyHeart.png");
    top.loadFromFile("sprites/background1.png");
    mid_1.loadFromFile("sprites/background2.png");
    mid_2.loadFromFile("sprites/background3.png");
    bottom.loadFromFile("sprites/background4.png");
    diamond.loadFromFile("sprites/diamond.png");
    coin.loadFromFile("sprites/coin.png");
    n0.loadFromFile("sprites/N0.png");
    n1.loadFromFile("sprites/N1.png");
    n2.loadFromFile("sprites/N2.png");
    n3.loadFromFile("sprites/N3.png");
    n4.loadFromFile("sprites/N4.png");
    n5.loadFromFile("sprites/N5.png");
    n6.loadFromFile("sprites/N6.png");
    n7.loadFromFile("sprites/N7.png");
    n8.loadFromFile("sprites/N8.png");
    n9.loadFromFile("sprites/N9.png");
    game_over.loadFromFile("sprites/gameOver.png");
    win.loadFromFile("sprites/win.png");
    flag_up.loadFromFile("sprites/up.png");
    flag_down.loadFromFile("sprites/down.png");
    player.loadFromFile("sprites/player.png");
    jump_sound.loadFromFile("sound/jump.wav");
    death.loadFromFile("sound/death.wav");
    coin_sound.loadFromFile("sound/coin.wav");
}

void SetNumber(sf::Sprite& numbers,int coin_count)
{
    switch (coin_count)
    {
    case 1:
        numbers.setTexture(n1);
        break;
    case 2:
        numbers.setTexture(n2);
        break;
    case 3:
        numbers.setTexture(n3);
        break;
    case 4:
        numbers.setTexture(n4);
        break;
    case 5:
        numbers.setTexture(n5);
        break;
    case 6:
        numbers.setTexture(n6);
        break;
    case 7:
        numbers.setTexture(n7);
        break;
    case 8:
        numbers.setTexture(n8);
        break;
    case 9:
        numbers.setTexture(n9);
        break;
    default:
        numbers.setTexture(n0);
        break;
    }
}

void hud(sf::RenderWindow& window)
{
    sf::Sprite lives;
    sf::Sprite numbers;

    //lives
    lives.setOrigin(TILE_SIZE_PX / 2, TILE_SIZE_PX / 2);
    lives.setScale(3, 3);
    if (lives_count < 3) {
        lives.setTexture(empty_heart);
    }
    else
    {
        lives.setTexture(heart);
    }
    lives.setPosition(view_pos.x+450, view_pos.y +50);
    window.draw(lives);
    if (lives_count < 2) {
        lives.setTexture(empty_heart);
    }
    else
    {
        lives.setTexture(heart);
    }
    lives.setPosition(view_pos.x + 400, view_pos.y + 50);
    window.draw(lives);
    if (lives_count < 1) {
        lives.setTexture(empty_heart);
    }
    else
    {
        lives.setTexture(heart);
    }
    lives.setPosition(view_pos.x + 350, view_pos.y + 50);
    window.draw(lives);

    //coins
    numbers.setOrigin(TILE_SIZE_PX / 2, TILE_SIZE_PX / 2);
    numbers.setScale(2, 2);
    SetNumber(numbers, coins);
    numbers.setPosition(view_pos.x - 440, view_pos.y + 50);
    window.draw(numbers);
    SetNumber(numbers, ten_coins);
    numbers.setPosition(view_pos.x - 460, view_pos.y + 50);
    window.draw(numbers);
    SetNumber(numbers, hundred_coins);
    numbers.setPosition(view_pos.x - 480, view_pos.y + 50);
    window.draw(numbers);
}

void add_coin(int coin_to_add)
{
    coins += coin_to_add;
    if (coins>9)
    {
        coins -= 10;
        ten_coins++;
    }
    if (ten_coins > 9)
    {
        ten_coins -= 10;
        hundred_coins++;
    }
}

int main()
{
    
    sf::View view;
    view.setSize(1000, 800);
    sf::Sound sound;
    sf::Sound sound2;
    sf::Vector2f view_size(view.getSize());

    load();
    load_texture();
    // create the window
    sf::RenderWindow window(sf::VideoMode(1000, 800), "My window");
    window.setFramerateLimit(60);

    // Create tile sprite
    sf::Sprite tile_sprite;
    sf::Sprite background_sprite;
    sf::Sprite item_sprite;
    sf::Sprite player_sprite;
    player_sprite.setTexture(player);
    player_sprite.setOrigin(12, 22);
    
    // Create cursor shape
    sf::RectangleShape cursor_shape(sf::Vector2f(TILE_SIZE_PX, TILE_SIZE_PX));
    cursor_shape.setFillColor(sf::Color(0, 0, 0, 0));
    cursor_shape.setOutlineColor(sf::Color(250, 250, 250));
    cursor_shape.setOutlineThickness(-3);

    // Create debug limit shapes
    sf::RectangleShape debug_limit_shape_vertical(sf::Vector2f(2, 10000));
    debug_limit_shape_vertical.setOrigin(1, 0);
    sf::RectangleShape debug_limit_shape_horizontal(sf::Vector2f(10000, 2));
    debug_limit_shape_horizontal.setOrigin(0, 1);

    // Create player shape
    sf::CircleShape player_origin_shape(10);
    player_origin_shape.setOrigin(10, 10);

    
    int edit_mode = 0;
    int sprite = 0;
    // run the program as long as the window is open
    while (window.isOpen())
    {
        if (player_pos.x<500)
        {
            view.setCenter(500, view_size.y / 2);
            view_pos.x = 500;
        }else
        {
	        view.setCenter(player_pos.x, view_size.y / 2);
            view_pos.x = player_pos.x;
        }
        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while (window.pollEvent(event))
        {
            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::F5) {
                    save();
                }
                if (event.key.code == sf::Keyboard::F9) {
                    load();
                }
                if (event.key.code == sf::Keyboard::F1) {
                    edit_mode = 1;
                }
                if (event.key.code == sf::Keyboard::F2) {
                    edit_mode = 2;
                }
                if (event.key.code == sf::Keyboard::F3) {
                    edit_mode = 3;
                }
                if (event.key.code == sf::Keyboard::F4) {
                    edit_mode = 0;
                }
                if (event.key.code == sf::Keyboard::Num1) {
                    sprite = 1;
                }
                if (event.key.code == sf::Keyboard::Num2) {
                    sprite = 2;
                }
                if (event.key.code == sf::Keyboard::Num3) {
                    sprite = 3;
                }
            }
        }


        float limit_x_low = -1000000000.f;
        float limit_x_high = 1000000000.f;
        float limit_y_low = -1000000000.f;
        float limit_y_high = 1000000000.f;

        sf::Vector2i player_coords = pos_to_coords(player_pos);
        const int margin = 1;
        if (tile_at(player_coords + sf::Vector2i(1, 0))) {
            limit_x_high = (player_coords.x + 1) * TILE_SIZE_PX - margin;
        }
        if (tile_at(player_coords + sf::Vector2i(-1, 0))) {
            limit_x_low = (player_coords.x) * TILE_SIZE_PX + margin;
        }
        if (tile_at(player_coords + sf::Vector2i(0, 1))) {
            limit_y_high = (player_coords.y + 1) * TILE_SIZE_PX - margin;
        }
        if (tile_at(player_coords + sf::Vector2i(0, -1))) {
            limit_y_low = (player_coords.y) * TILE_SIZE_PX + margin;
        }

        // moving the player!
        sf::Vector2f delta(0, 0);
        const float player_speed = 5;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            delta += sf::Vector2f(-player_speed, 0);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)|| sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            delta += sf::Vector2f(player_speed, 0);
        }

        // Cancel vertical velocity if grounded
        if (grounded) {
            player_vel.y = 0;
        }

        // jumping
        bool jump_key_is_down = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
        if (jump_key_is_down && grounded) {
            sound.setBuffer(jump_sound);
            sound.setVolume(1);
            sound.play();
            player_vel.y = -10;
        }

        // Falling speed limit
        if (player_vel.y > 7) {
            player_vel.y = 7;
        }

        // Gravity
        player_vel.y += jump_key_is_down&&player_vel.y<0 ? 0.3f : 0.6f; 
        if(lives_count != 0||player_win)
        {
	        delta += sf::Vector2f(0, player_vel.y);
        	player_pos += delta;
        }

        if (player_pos.x >= limit_x_high - (player_size.x / 2)) {
            player_pos.x = limit_x_high - (player_size.x / 2);
        }
        if (player_pos.x <= limit_x_low + (player_size.x / 2)) {
            player_pos.x = limit_x_low + (player_size.x / 2);
        }
        grounded = false;
        if (player_pos.y >= limit_y_high) {
            player_pos.y = limit_y_high;
            grounded = true;
        }
        if (player_pos.y <= limit_y_low + player_size.y) {
            player_pos.y = limit_y_low + player_size.y;
            player_vel.y = 0;
        }
        if (player_pos.y >= 800)
        {
            sound2.setBuffer(death);
            sound2.setVolume(10);
            sound2.play();
            player_alive = false;
            player_pos = player_spawn;
        }
        if (player_pos.x>4550&&player_pos.x<4700)
        {
            player_win = true;
        }

        // Determine tile coordinates that the mouse is hovering
        sf::Vector2i mouse_pos = sf::Mouse::getPosition(window);
        sf::Vector2i mouse_tile_coords((player_pos.x + mouse_pos.x-500) / TILE_SIZE_PX,mouse_pos.y / TILE_SIZE_PX);
        sf::Vector2i mouse_tile_coords2((player_pos.x + mouse_pos.x - 500) / TILE_SIZE_PX_BACKGROUND, mouse_pos.y / TILE_SIZE_PX_BACKGROUND);
        if(player_pos.x<500)
        {
            mouse_tile_coords.x = mouse_pos.x / TILE_SIZE_PX;
            mouse_tile_coords.y = mouse_pos.y / TILE_SIZE_PX;
            mouse_tile_coords2.x = mouse_pos.x / TILE_SIZE_PX_BACKGROUND;
            mouse_tile_coords2.y = mouse_pos.y / TILE_SIZE_PX_BACKGROUND;
        }
    	
        // clear the window with black color
        window.clear(sf::Color::Black);

        

        background_sprite.setScale(TILE_SIZE_PX_BACKGROUND / 24, TILE_SIZE_PX_BACKGROUND / 24);
        for (int y = 0; y < 12; y++) {
            for (int x = 0; x < TILEMAP_WIDTH; x++) {
                if (tilemap_background[x + y * TILEMAP_WIDTH] == kTop) 
                {
                    background_sprite.setTexture(top);
                    background_sprite.setPosition(TILE_SIZE_PX_BACKGROUND * x, TILE_SIZE_PX_BACKGROUND * y);
                    window.draw(background_sprite);
                }
                else if (tilemap_background[x + y * TILEMAP_WIDTH] == kMid1)
                {
                    background_sprite.setTexture(mid_1);
                    background_sprite.setPosition(TILE_SIZE_PX_BACKGROUND * x, TILE_SIZE_PX_BACKGROUND * y);
                    window.draw(background_sprite);
                }
                else if (tilemap_background[x + y * TILEMAP_WIDTH] == kMid2)
                {
                    background_sprite.setTexture(mid_2);
                    background_sprite.setPosition(TILE_SIZE_PX_BACKGROUND * x, TILE_SIZE_PX_BACKGROUND * y);
                    window.draw(background_sprite);
                }
                else if (tilemap_background[x + y * TILEMAP_WIDTH] == kBottom)
                {
                    background_sprite.setTexture(bottom);
                    background_sprite.setPosition(TILE_SIZE_PX_BACKGROUND * x, TILE_SIZE_PX_BACKGROUND * y);
                    window.draw(background_sprite);
                }
            }
        }
        // draw the tilemap_ground
        tile_sprite.setScale(1, 1);
        for (int y = 0; y < TILEMAP_HEIGHT; y++) {
            for (int x = 0; x < TILEMAP_WIDTH; x++) {
                if (tilemap_ground[x + y * TILEMAP_WIDTH] == kDirt)
                {
                    tile_sprite.setTexture(dirt);
                    tile_sprite.setPosition(TILE_SIZE_PX * x, TILE_SIZE_PX * y);
                    window.draw(tile_sprite);
                }
                else if (tilemap_ground[x + y * TILEMAP_WIDTH] == kDirtAndGrass)
                {
                    tile_sprite.setTexture(dirt_and_grass);
                    tile_sprite.setPosition(TILE_SIZE_PX* x, TILE_SIZE_PX* y);
                    window.draw(tile_sprite);
                }
            }
        }

       
    	int index = (int)player_pos.x/TILE_SIZE_PX + (int)player_pos.y/TILE_SIZE_PX *TILEMAP_WIDTH;
    	if (tilemap_item[index]==kCoin) {
            sound2.setBuffer(coin_sound);
            sound2.setVolume(5);
            sound2.play();
            add_coin(1);
    		tilemap_item[index] = kEmptyItem;
    	}
        if (tilemap_item[index] == kDiamond) {
            sound2.setBuffer(coin_sound);
            sound2.setVolume(5);
            sound2.play();
            add_coin(5);
            tilemap_item[index] = kEmptyItem;
        }
        if (tilemap_item[index] == kFlagDown) {
            player_spawn.x = player_pos.x;
            player_spawn.y = player_pos.y-10;
            tilemap_item[index] = kFlagUp;
        }

        item_sprite.setScale(1, 1);
        for (int y = 0; y < TILEMAP_HEIGHT; y++) {
            for (int x = 0; x < TILEMAP_WIDTH; x++) {
                if (tilemap_item[x + y * TILEMAP_WIDTH] == kDiamond)
                {
                    item_sprite.setTexture(diamond);
                    item_sprite.setPosition(TILE_SIZE_PX * x, TILE_SIZE_PX * y);
                    window.draw(item_sprite);
                }
                else if (tilemap_item[x + y * TILEMAP_WIDTH] == kCoin)
                {
                    item_sprite.setTexture(coin);
                    item_sprite.setPosition(TILE_SIZE_PX * x, TILE_SIZE_PX * y);
                    window.draw(item_sprite);
                }
                else if (tilemap_item[x + y * TILEMAP_WIDTH] == kFlagUp)
                {
                    item_sprite.setTexture(flag_up);
                    item_sprite.setPosition(TILE_SIZE_PX * x, TILE_SIZE_PX * y);
                    window.draw(item_sprite);
                }
                else if (tilemap_item[x + y * TILEMAP_WIDTH] == kFlagDown)
                {
                    item_sprite.setTexture(flag_down);
                    item_sprite.setPosition(TILE_SIZE_PX * x, TILE_SIZE_PX * y);
                    window.draw(item_sprite);
                }
            }
        }


        // Editor interaction
        bool mouse_left = sf::Mouse::isButtonPressed(sf::Mouse::Left);
        bool mouse_right = sf::Mouse::isButtonPressed(sf::Mouse::Right);
        if (edit_mode==1)
        {
            cursor_shape.setSize(sf::Vector2f(TILE_SIZE_PX, TILE_SIZE_PX));
            cursor_shape.setPosition(TILE_SIZE_PX * mouse_tile_coords.x, TILE_SIZE_PX * mouse_tile_coords.y);
        	window.draw(cursor_shape);
            if (mouse_right) {
                if (in_bounds(mouse_tile_coords)) {
                    tilemap_ground[mouse_tile_coords.y * TILEMAP_WIDTH + mouse_tile_coords.x] = kEmpty;
                }
            }
            if (mouse_left) {
                if (in_bounds(mouse_tile_coords)) {
	                switch (sprite)
	                {
	                case 1:
	                	tilemap_ground[mouse_tile_coords.y * TILEMAP_WIDTH + mouse_tile_coords.x] = kEmpty;
	                	break;
	                case 2:
	                	tilemap_ground[mouse_tile_coords.y * TILEMAP_WIDTH + mouse_tile_coords.x] = kDirt;
	                	break;
	                case 3:
	                	tilemap_ground[mouse_tile_coords.y * TILEMAP_WIDTH + mouse_tile_coords.x] = kDirtAndGrass;
	                	break;
	                case 4:
	                	
	                	break;
	                }
                }
            }
		}
        if (edit_mode == 3)
        {
            cursor_shape.setSize(sf::Vector2f(TILE_SIZE_PX, TILE_SIZE_PX));
            cursor_shape.setPosition(TILE_SIZE_PX * mouse_tile_coords.x, TILE_SIZE_PX * mouse_tile_coords.y);
            window.draw(cursor_shape);
            if (mouse_right) {
                if (in_bounds(mouse_tile_coords)) {
                    tilemap_item[mouse_tile_coords.y * TILEMAP_WIDTH + mouse_tile_coords.x] = kEmptyItem;
                }
            }
            if (mouse_left) {
                if (in_bounds(mouse_tile_coords)) {
                    switch (sprite)
                    {
                    case 1:
                        tilemap_item[mouse_tile_coords.y * TILEMAP_WIDTH + mouse_tile_coords.x] = kCoin;
                        break;
                    case 2:
                        tilemap_item[mouse_tile_coords.y * TILEMAP_WIDTH + mouse_tile_coords.x] = kDiamond;
                        break;
                    case 3:
                        tilemap_item[mouse_tile_coords.y * TILEMAP_WIDTH + mouse_tile_coords.x] = kFlagDown;
                        break;
                    }
                }
            }
        }
        if (edit_mode==2)
        {
            cursor_shape.setPosition(TILE_SIZE_PX_BACKGROUND* mouse_tile_coords2.x, TILE_SIZE_PX_BACKGROUND* mouse_tile_coords2.y);
            cursor_shape.setSize(sf::Vector2f(TILE_SIZE_PX_BACKGROUND,TILE_SIZE_PX_BACKGROUND));
            window.draw(cursor_shape);
            if (mouse_right) {
                if (in_bounds(mouse_tile_coords)) {
                    tilemap_background[mouse_tile_coords2.y * TILEMAP_WIDTH + mouse_tile_coords2.x] = kEmptyBackground;
                }
            }
            if (mouse_left) {
                if (in_bounds(mouse_tile_coords)) {
                    switch (sprite)
                    {
                    case 1:
                        tilemap_background[mouse_tile_coords2.y * TILEMAP_WIDTH + mouse_tile_coords2.x] = kTop;
                        break;
                    case 2:
                        if (mouse_tile_coords2.x % 2 == 0)
                        {
                            tilemap_background[mouse_tile_coords2.y * TILEMAP_WIDTH + mouse_tile_coords2.x] = kMid1;
                            break;
                        }
                        if (mouse_tile_coords2.x % 2 == 1)
                        {
                            tilemap_background[mouse_tile_coords2.y * TILEMAP_WIDTH + mouse_tile_coords2.x] = kMid2;
                            break;
                        }
                        std::cout << "problem background\n";
                        break;
                    case 3:
                        tilemap_background[mouse_tile_coords2.y * TILEMAP_WIDTH + mouse_tile_coords2.x] = kBottom;
                        break;
                    }
                }
            }
        }
        // draw player

        player_sprite.setPosition(player_pos.x, player_pos.y);
        window.draw(player_sprite);
        player_origin_shape.setFillColor(grounded ? sf::Color(255, 0, 0) : sf::Color(0, 255, 255));
        player_origin_shape.setPosition(player_pos.x, player_pos.y);
        //window.draw(player_origin_shape);
    	window.setView(view);
        if (!player_alive)
        {
            lives_count--;
            std::cout << " taking damage\n" << lives_count << " lives remaining";
            player_alive = true;
        }
        if (lives_count == 0) {
            sf::Sprite game_o;
            game_o.setTexture(game_over);
            game_o.setPosition(player_pos);
            game_o.setOrigin(528 / 2, 50);
            game_o.setScale(2.5, 2.5);
            window.draw(game_o);
            player_spawn.x = 5000;
            player_spawn.y = -150;
            player_pos = player_spawn;
        }
        if(player_win)
        {
            sf::Sprite game_win;
            game_win.setTexture(win);
            game_win.setPosition(player_pos);
            game_win.setOrigin(400, -100);
            game_win.setScale(1.3, 1.3);
            window.draw(game_win);
            player_spawn.x = 6000;
            player_spawn.y = -150;
            player_pos = player_spawn;
        }
        hud(window);
        window.display();
    }
    return 0;
}
