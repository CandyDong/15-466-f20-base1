//for the GL_ERRORS() macro:
#include "gl_errors.hpp"
#include "PPU466.hpp"
#include "load_save_png.hpp"
#include "Sprite.hpp"


//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>
#include "glm/gtx/string_cast.hpp"
#include <iostream>
#include <fstream>
#include <bitset>
#include <sstream>

void parse_str_to_color(std::string color_str, glm::u8vec4 &color);

void load_sprite_palette(std::string filename, uint8_t &num_palettes, std::array<glm::u8vec4, 4> &palette) {
    
    std::ifstream infile(filename);

    if(!infile)
    {
        std::cerr << "Cannot open the File : "<< filename <<std::endl;
        return;
    }
    std::cout << filename << std::endl;
    std::string num;
    // parse the first line which contains the number of palette needed for the sprite
    std::getline(infile, num);
    num_palettes = std::stoi(num);
    std::cout << "number of palettes needed for " + filename + " :" + std::to_string(num_palettes) << std::endl;

    std::string str;
    // Read the next line from File untill it reaches the end.
    uint8_t count = 0;
    while (std::getline(infile, str))
    {   
        if(str.size() > 0){
            parse_str_to_color(str, palette[count]);
        }
        count++;
    }
    //Close The File
    infile.close();
}

void load_sprite_tile(std::string filename, PPU466::Palette palette, 
                        std::array<PPU466::Tile, 4> tiles,
                        glm::uvec2 &size) {
    glm::uvec2 img_size = glm::uvec2(0); // size in pixels
    std::vector< glm::u8vec4 > data; // pixel data

    //actually load the background:
    load_png(filename, &img_size, &data, LowerLeftOrigin);
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
                    int index = find_color_index_in_palette(color, palette);
                    std::cout << "tile: " + std::to_string(y) + ", " + std::to_string(x) + 
                                 " " + std::to_string(i) + ", " + std::to_string(j) +
                                 " index: " + std::to_string(index) + " color: " + glm::to_string(color) << std::endl;
                    bit0 |= (index & 0b01) << j;
                    bit1 |= ((index & 0b10) >> 1) << j;
                }
                tile.bit0[i] = bit0;
                tile.bit1[i] = bit1;
            }
            tiles[y*size.x+x] = tile;
            std::cout << "tile: " + std::to_string(y) + ", " + std::to_string(x) + " => " + 
                        std::to_string(y*size.x+x) << std::endl;
            print_tile(tile);
        }
    }
    printf("tiles.size() = %lu\n", tiles.size());
}

void print_tile(PPU466::Tile tile) {
    for (int i = 0; i < 8; i++) {
        std::bitset<8> b0(tile.bit0[i]);
        std::bitset<8> b1(tile.bit1[i]);
        std::cout << b1 << "       "  << b0 << std::endl;
    }
    std::cout << "\n\n" << std::endl;
}

void parse_str_to_color(std::string color_str, glm::u8vec4 &color) {
    std::array<uint8_t, 4> color_values;
    std::cout << "line: " + color_str << std::endl;
    std::istringstream is(color_str);
    int value;

    int i = 0;
    while(is >> value) {
        color_values[i] = value;
        i++;
    }
    assert(color_values.size() == 4);
    color = glm::u8vec4(color_values[0], color_values[1], color_values[2], color_values[3]);
    std::cout << "color values: " + glm::to_string(color) << std::endl;
}

int find_color_index_in_palette(glm::u8vec4 color, PPU466::Palette palette) {
    for (int i = 0; i < 4; i++) {
        if (color == palette[i]) {
            return i;
        }
    }
    return 0;
}