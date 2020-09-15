#include "PlayMode.hpp"
#include "Sprite.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>

#include <random>

PlayMode::PlayMode() {
	//TODO:
	// you *must* use an asset pipeline of some sort to generate tiles.
	// don't hardcode them like this!
	// or, at least, if you do hardcode them like this,
	//  make yourself a script that spits out the code that you paste in here
	//   and check that script into your repository.

	//Also, *don't* use these tiles in your game:
	uint8_t num_palettes;
	load_palette("assets/yellow_car_palette.txt", num_palettes, ppu.palette_table, PLAYER_PALETTE);
	load_palette("assets/red_car_palette.txt", num_palettes, ppu.palette_table, RED_PALETTE);
	load_palette("assets/blue_car_palette.txt", num_palettes, ppu.palette_table, BLUE_PALETTE);
	load_palette("assets/background_palette.txt", num_palettes, ppu.palette_table, BACKGROUND_PALETTE);
	

	{ //use tiles 0-16 as some weird dot pattern thing:
		std::array< uint8_t, 8*8 > distance;
		for (uint32_t y = 0; y < 8; ++y) {
			for (uint32_t x = 0; x < 8; ++x) {
				float d = glm::length(glm::vec2((x + 0.5f) - 4.0f, (y + 0.5f) - 4.0f));
				d /= glm::length(glm::vec2(4.0f, 4.0f));
				distance[x+8*y] = std::max(0,std::min(255,int32_t( 255.0f * d )));
			}
		}
		for (uint32_t index = 0; index < 16; ++index) {
			PPU466::Tile tile;
			uint8_t t = (255 * index) / 16;
			for (uint32_t y = 0; y < 8; ++y) {
				uint8_t bit0 = 0;
				uint8_t bit1 = 0;
				for (uint32_t x = 0; x < 8; ++x) {
					uint8_t d = distance[x+8*y];
					if (d > t) {
						bit0 |= (1 << x);
					} else {
						bit1 |= (1 << x);
					}
				}
				tile.bit0[y] = bit0;
				tile.bit1[y] = bit1;
			}
			ppu.tile_table[index] = tile;
		}
	}

	glm::uvec2 sprite_tile_size;
	// load player sprite tiles
	load_player_sprite_tile("assets/yellow_car.png", ppu.palette_table[PLAYER_PALETTE], 
                        ppu.tile_table, PLAYER_TILE_START, sprite_tile_size);
	PLAYER_TILE_END = PLAYER_TILE_START + sprite_tile_size.x*sprite_tile_size.y;
	std::cout << "player tiles: " + std::to_string(PLAYER_TILE_START) + "-" +
				std::to_string(PLAYER_TILE_END) << std::endl;
	
	BACKGROUND_TILE_START = PLAYER_TILE_END;
	// load player sprite tiles
	load_background_tiles("assets/background_tiles", ppu.palette_table[BACKGROUND_PALETTE], 
                        ppu.tile_table, BACKGROUND_TILE_START, BACKGROUND_TILE_END);
	std::cout << "background tiles: " + std::to_string(BACKGROUND_TILE_START) + "-" +
				std::to_string(BACKGROUND_TILE_END) << std::endl;

	//makes the outside of tiles 0-16 solid:
	ppu.palette_table[0] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	};

	//makes the center of tiles 0-16 solid:
	ppu.palette_table[1] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	};

}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.pressed = false;
			return true;
		}
	}

	return false;
}

bool equal(float a, float b)
{
    return (fabs(a-b) < 1e-2);
}

float get_absolute (float speed, float screen) {

	while (speed < 0.0f) {
		if (speed > 0.0f && speed < screen) {
			return speed;
		}
		speed += screen;
	}
	while (speed > 0.0f) {
		if (speed > 0.0f && speed < screen) {
			return speed;
		}
		speed -= screen;
	}

	return speed;
}

void PlayMode::update(float elapsed) {

	//slowly rotates through [0,1):
	// (will be used to set background color)
	background_fade += elapsed / 10.0f;
	background_fade -= std::floor(background_fade);

	if (GAME_OVER) {
		return;
	}

	constexpr float PlayerSpeed = 30.0f;
	player_at.x += PlayerSpeed * elapsed;
	if (left.pressed) player_at.x -= 2 * PlayerSpeed * elapsed;
	if (right.pressed) player_at.x += 2.0f * PlayerSpeed * elapsed;
	if (down.pressed) {
		player_at.y -= PlayerSpeed * elapsed;
		for (int i = 0; i < num_opponents; i++) {
			oppo_speeds[i].y += PlayerSpeed * elapsed;
		}
	}
	if (up.pressed) {
		player_at.y += PlayerSpeed * elapsed;
		for (int i = 0; i < num_opponents; i++) {
			oppo_speeds[i].y -= PlayerSpeed * elapsed;
		}
	}

	static std::mt19937 mt;
	for (int i = 0; i < num_opponents; i++) {
		oppo_speeds[i].x -=  PlayerSpeed * elapsed + 1.5f*(mt() / float(mt.max()));
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;

	//---- collision handling ----
	auto collide = [this](glm::vec2 const &oppo) {
		//compute area of overlap:
		glm::vec2 o = glm::vec2(get_absolute(oppo.x, PPU466::ScreenWidth), 
								get_absolute(oppo.y, PPU466::ScreenHeight));
		glm::vec2 p = glm::vec2(get_absolute(player_at.x, PPU466::ScreenWidth), 
								get_absolute(player_at.y, PPU466::ScreenHeight));
		// printf("player: (%f, %f), oppo (%f, %f)\n", p.x, p.y, o.x, o.y);
		glm::ivec2 min = glm::max(o, p);
		glm::ivec2 max = glm::min(o + glm::vec2(10, 16), p + glm::vec2(24, 16));

		//if no overlap, no collision:
		if (min.x > max.x || min.y > max.y) return;
		GAME_OVER = true;
		printf("GAME OVER\n");
	};

	for (int i = 0; i < num_opponents; i++) {
		// std::string msg = "player: (" + std::to_string(player_at.x) + ", " +  std::to_string(player_at.y) + 
		// 		") oppo " + std::to_string(i) + ": (" + 
		// 		std::to_string(oppo_speeds[i].x) + ", " +  
		// 		std::to_string(oppo_speeds[i].y) + ")\n";
		collide(oppo_speeds[i]);
		if (GAME_OVER) {
			return;
		}
	}
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---

	//background color will be some hsv-like fade:
	ppu.background_color = glm::u8vec4(
		std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI * (background_fade + 0.0f / 3.0f) ) ) ))),
		std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI * (background_fade + 1.0f / 3.0f) ) ) ))),
		std::min(255,std::max(0,int32_t(255 * 0.5f * (0.5f + std::sin( 2.0f * M_PI * (background_fade + 2.0f / 3.0f) ) ) ))),
		0xff
	);

	//tilemap gets recomputed every frame as some weird plasma thing:
	//NOTE: don't do this in your game! actually make a map or something :-)
	for (uint32_t y = 0; y < PPU466::BackgroundHeight; y += 2) {
		for (uint32_t x = 0; x < PPU466::BackgroundWidth; x += 2) {
			uint8_t row = y % BACKGROUND_TILE_GROUP_HEIGHT / 2;
			uint8_t zebra = y / BACKGROUND_TILE_GROUP_HEIGHT;
			uint8_t col = x % BACKGROUND_TILE_GROUP_WIDTH / 2;
			uint8_t tile_index;
			if (zebra % 2 == 0) {
				if (col == 0) {
					tile_index = BACKGROUND_TILE_START + BACKGROUND_MAP[2-row];
				} else {
					tile_index = BACKGROUND_TILE_START + BACKGROUND_MAP[5-row];
				}
			}
			else {
				tile_index = BACKGROUND_TILE_START + BACKGROUND_MAP[4];
			}
			
			
			// std::cout << "x: " + std::to_string(x) + 
			// 			" y: " + std::to_string(y) + 
			// 			" row: " + std::to_string(row) +  
			// 			" col: " + std::to_string(col) + 
			// 			" tile index: " + std::to_string(tile_index)
			// 			<< std::endl;
			
			for (int c = 0; c < 4; c++) {
				ppu.background[(x+c%2)+PPU466::BackgroundWidth*(y+c/2)] = (BACKGROUND_PALETTE << 8) | tile_index+c;
			}
		}
	}

	//background scroll:
	ppu.background_position.x = int32_t(-0.5f * player_at.x);
	ppu.background_position.y = int32_t(-player_at.y);

	//player sprite:
	uint8_t sprite_count = 0;
	for (int i = 0; i < PLAYER_TILE_END - PLAYER_TILE_START; i++) {
		ppu.sprites[i].x = int32_t(player_at.x) + (i%4)*8;
		ppu.sprites[i].y = int32_t(player_at.y) + (i>=4)*8;
		ppu.sprites[i].index = i + PLAYER_TILE_START;
		ppu.sprites[i].attributes = PLAYER_PALETTE;
		// print_tile("assets/test_tile.txt", ppu.tile_table[i + PLAYER_TILE_START], i + PLAYER_TILE_START);
		// std::cout << "index: " + std::to_string(i + PLAYER_TILE_START) + 
		// 			"x offset: " + std::to_string((i%4)*8) + 
		// 			"x offset: " + std::to_string((i/4)*8) << std::endl;
	}
	sprite_count +=8;

	static std::mt19937 mt; //mersenne twister pseudo-random number generator
	//some other misc sprites:
	for (int c = 0; c < num_opponents; c++) {
		for (uint32_t i = 0; i < PLAYER_TILE_END - PLAYER_TILE_START; i++) {
			ppu.sprites[sprite_count+i].x = oppo_speeds[c].x + (i%4)*8;
			ppu.sprites[sprite_count+i].y = oppo_speeds[c].y + (i>=4)*8;
			ppu.sprites[sprite_count+i].index = i + PLAYER_TILE_START;
			ppu.sprites[sprite_count+i].attributes = (c%2 == 0) ? RED_PALETTE : BLUE_PALETTE;
		}
		sprite_count += 8;
	}

	//--- actually draw ---
	ppu.draw(drawable_size);
}