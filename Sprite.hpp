#pragma once

#include "GL.hpp"
#include "PPU466.hpp"

#include <glm/glm.hpp>

#include <unordered_map>
#include <string>

void load_sprite_palette(std::string filename, uint8_t &num_palettes, std::array<glm::u8vec4, 4> &palette);

void load_sprite_tile(std::string filename, PPU466::Palette palette, 
                        std::array<PPU466::Tile, 4> tiles,
                        glm::uvec2 &size);

void print_tile(PPU466::Tile tile);
int find_color_index_in_palette(glm::u8vec4 color, PPU466::Palette palette);

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
