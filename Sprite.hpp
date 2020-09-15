#pragma once

#include "GL.hpp"
#include "PPU466.hpp"

#include <glm/glm.hpp>

#include <unordered_map>
#include <string>

void load_palette(std::string filename, uint8_t &num_palettes, 
                            std::array<PPU466::Palette, 8> &palette_table,
                            uint8_t start_index);

void load_background_palette(std::string filename, uint8_t &num_palettes, 
                            std::array<PPU466::Palette, 8> &palette_table,
                            uint8_t start_index);

void load_player_sprite_tile(std::string filename, PPU466::Palette palette, 
                        std::array<PPU466::Tile, 16*16>& tile_table,
                        uint8_t start_index,
                        glm::uvec2 &size);

void load_background_tiles(std::string dirname, PPU466::Palette palette, 
                        std::array<PPU466::Tile, 16*16>& tile_table,
                        uint8_t start_index,
                        uint8_t &end_index);

int find_color_index_in_palette(glm::u8vec4 color, PPU466::Palette palette);

void print_tile(std::string filename, PPU466::Tile tile, uint8_t index);

// struct SpriteTile {
// 	//load from filebase.png and filebase.atlas:
// 	SpriteTile(std::string const &filebase);
// 	~SpriteTile();

// 	//look up sprite in list of loaded sprites:
// 	// throws an error if name is missing
// 	Sprite const &lookup(std::string const &name) const;

// 	//this is the atlas texture; used when drawing sprites:
// 	GLuint tex = 0;
// 	glm::uvec2 tex_size = glm::uvec2(0);

// 	//---- internal data ---

// 	//table of loaded sprites, sorted by name:
// 	std::unordered_map< std::string, Sprite > sprites;

// };
