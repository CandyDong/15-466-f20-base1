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
	std::array<glm::u8vec4, 4> colors;
	load_sprite_palette("assets/yellow_car_palette.txt", num_palettes, colors);
	ppu.palette_table[7] = colors;

	// ppu.tile_table[34].bit1 = {
	// 	0b00000000,       
	// 	0b00000000,      
	// 	0b00000000,       
	// 	0b00000000,       
	// 	0b11111111,       
	// 	0b11111111,       
	// 	0b11111111,       
	// 	0b11111111      
	// };
	// ppu.tile_table[34].bit0 = {
	// 	0b10011000,
	// 	0b01100001,
	// 	0b10110100,
	// 	0b10011010,
	// 	0b11111111,
	// 	0b01111111,
	// 	0b00000000,
	// 	0b00000000
	// };
	

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

	//use sprite 32 as a "player":
	ppu.tile_table[32].bit0 = {
		0b01111110,
		0b11111111,
		0b11111111,
		0b11111111,
		0b11111111,
		0b11111111,
		0b11111111,
		0b01111110,
	};
	ppu.tile_table[32].bit1 = {
		0b00000000,
		0b00000000,
		0b00011000,
		0b00100100,
		0b00000000,
		0b00100100,
		0b00000000,
		0b00000000,
	};


	glm::uvec2 size;
	// load_sprite_tile("assets/yellow_car.png", ppu.palette_table[7], 
    //                     tiles, size);
	glm::uvec2 img_size = glm::uvec2(0); // size in pixels
    std::vector< glm::u8vec4 > data; // pixel data

    //actually load the background:
    load_png("assets/yellow_car.png", &img_size, &data, LowerLeftOrigin);
    printf("background size: %d, %d\n", img_size.x, img_size.y);
    size = img_size/glm::uvec2(8,8);
    printf("tile size: %d, %d\n", size.x, size.y);

    for (int y = 0; y < size.y; y++) {
        for (int x = 0; x < size.x; x++) {
            PPU466::Tile tile;
            uint8_t bit0 = 0;
            uint8_t bit1 = 0;
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    glm::u8vec4 color = data[(y*8+i)*img_size.x + x*8+j];
                    int index = find_color_index_in_palette(color, ppu.palette_table[7]);
                    bit0 |= (index & 0b01) << j;
                    bit1 |= ((index & 0b10) >> 1) << j;
                }
                tile.bit0[i] = bit0;
                tile.bit1[i] = bit1;
            }
            ppu.tile_table[33+y*size.x+x] = tile;
            std::cout << "tile: " + std::to_string(y) + ", " + std::to_string(x) + " => " + 
                        std::to_string(y*size.x+x) << std::endl;
            print_tile(tile);
        }
    }
	// ppu.tile_table[33].bit0 = tiles[0].bit0;
	// ppu.tile_table[33].bit1 = tiles[0].bit1;
	// ppu.tile_table[34].bit0 = tiles[1].bit0;
	// ppu.tile_table[34].bit1 = tiles[1].bit1;
	// ppu.tile_table[35].bit0 = tiles[2].bit0;
	// ppu.tile_table[35].bit1 = tiles[2].bit1;
	// ppu.tile_table[36].bit0 = tiles[3].bit0;
	// ppu.tile_table[36].bit1 = tiles[3].bit1;

	print_tile(ppu.tile_table[34]);

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

	// //used for the player:
	// ppu.palette_table[7] = {
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	// 	glm::u8vec4(0xff, 0xff, 0x00, 0xff),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	// };

	//used for the misc other sprites:
	ppu.palette_table[6] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x88, 0x88, 0xff, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0xff),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
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

void PlayMode::update(float elapsed) {

	//slowly rotates through [0,1):
	// (will be used to set background color)
	background_fade += elapsed / 10.0f;
	background_fade -= std::floor(background_fade);

	constexpr float PlayerSpeed = 30.0f;
	if (left.pressed) player_at.x -= PlayerSpeed * elapsed;
	if (right.pressed) player_at.x += PlayerSpeed * elapsed;
	if (down.pressed) player_at.y -= PlayerSpeed * elapsed;
	if (up.pressed) player_at.y += PlayerSpeed * elapsed;

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
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
	for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y) {
		for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
			//TODO: make weird plasma thing
			ppu.background[x+PPU466::BackgroundWidth*y] = ((x+y)%16);
		}
	}

	//background scroll:
	ppu.background_position.x = int32_t(-0.5f * player_at.x);
	ppu.background_position.y = int32_t(-0.5f * player_at.y);

	//player sprite:
	ppu.sprites[0].x = int32_t(player_at.x);
	ppu.sprites[0].y = int32_t(player_at.y);
	ppu.sprites[0].index = 33;
	ppu.sprites[0].attributes = 7;

	ppu.sprites[1].x = int32_t(player_at.x) + 8;
	ppu.sprites[1].y = int32_t(player_at.y);
	ppu.sprites[1].index = 34;
	ppu.sprites[1].attributes = 7;

	ppu.sprites[2].x = int32_t(player_at.x);
	ppu.sprites[2].y = int32_t(player_at.y) + 8;
	ppu.sprites[2].index = 35;
	ppu.sprites[2].attributes = 7;

	ppu.sprites[3].x = int32_t(player_at.x) + 8;
	ppu.sprites[3].y = int32_t(player_at.y) + 8;
	ppu.sprites[3].index = 36;
	ppu.sprites[3].attributes = 7;


	//some other misc sprites:
	for (uint32_t i = 4; i < 63; ++i) {
		float amt = (i + 2.0f * background_fade) / 62.0f;
		ppu.sprites[i].x = int32_t(0.5f * PPU466::ScreenWidth + std::cos( 2.0f * M_PI * amt * 5.0f + 0.01f * player_at.x) * 0.4f * PPU466::ScreenWidth);
		ppu.sprites[i].y = int32_t(0.5f * PPU466::ScreenHeight + std::sin( 2.0f * M_PI * amt * 3.0f + 0.01f * player_at.y) * 0.4f * PPU466::ScreenWidth);
		ppu.sprites[i].index = 32;
		ppu.sprites[i].attributes = 6;
		if (i % 2) ppu.sprites[i].attributes |= 0x80; //'behind' bit
	}

	//--- actually draw ---
	ppu.draw(drawable_size);
}