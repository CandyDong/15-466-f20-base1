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
#include <string>
#include <filesystem>

namespace fs = std::__fs::filesystem;

void parse_str_to_color(std::string color_str, glm::u8vec4 &color);
bool endsWith(std::string const &fullString, std::string const &ending);
void load_background_tile(std::string filename, PPU466::Palette palette, 
                        std::array<PPU466::Tile, 16*16>& tile_table,
                        uint8_t start_index,
                        uint8_t &end_index);

void load_palette(std::string filename, uint8_t &num_palettes, 
                        std::array<PPU466::Palette, 8> &palette_table,
                        uint8_t start_index) {
    
    std::ifstream infile(filename);

    if(!infile)
    {
        std::cerr << "Cannot open the File : "<< filename <<std::endl;
        return;
    }
    // std::cout << filename << std::endl;
    std::string num;
    // parse the first line which contains the number of palette needed for the sprite
    std::getline(infile, num);
    num_palettes = std::stoi(num);
    // std::cout << "number of palettes needed for " + filename + " :" + std::to_string(num_palettes) << std::endl;

    std::string str;
    // Read the next line from File untill it reaches the end.
    uint8_t count = 0;
    while (std::getline(infile, str))
    {   
        if(str.size() > 0){
            parse_str_to_color(str, palette_table[start_index][count]);
        }
        count++;
    }
    //Close The File
    infile.close();
}

void load_player_sprite_tile(std::string filename, PPU466::Palette palette, 
                        std::array<PPU466::Tile, 16*16>& tile_table,
                        uint8_t start_index,
                        glm::uvec2 &size) {
    glm::uvec2 img_size = glm::uvec2(0); // size in pixels
    std::vector< glm::u8vec4 > data; // pixel data

    //actually load the background:
    load_png(filename, &img_size, &data, LowerLeftOrigin);
    // printf("background size: %d, %d\n", img_size.x, img_size.y);
    size = img_size/glm::uvec2(8,8)*glm::uvec2(2, 1);
    // printf("tile size: %d, %d\n", size.x, size.y);

    // std::ofstream outfile;
	// outfile.open("assets/colors_at_index_mirror.txt");
    for (int y = 0; y < size.y; y++) {
        for (int x = 0; x < size.x; x++) {
            PPU466::Tile tile;
            for (int i = 0; i < 8; i++) {
                uint8_t bit0 = 0;
                uint8_t bit1 = 0;
                for (int j = 0; j < 8; j++) {
                    uint8_t color_y = (y*8+i);
                    uint8_t color_x;
                    if (x == 0 || x == 3) {
                        color_x = 8 + j;
                    } else {
                        color_x = j;
                    }
                    glm::u8vec4 color = data[(color_y)*img_size.x + color_x];
                    int index = find_color_index_in_palette(color, palette);
                    uint8_t shift = j;
                    if (x < size.x / 2) {
                        shift = (7-j);
                    }
                    bit0 |= (index & 0b01) << shift;
                    bit1 |= ((index & 0b10) >> 1) << shift;
                    std::bitset<8> b0(bit0);
                    std::bitset<8> b1(bit1);
                    // outfile << std::to_string(color_y) + ", " + std::to_string(color_x) +
                    //              " color: " + glm::to_string(color) + 
                    //              " tile=" + std::to_string(start_index+y*size.x+x) +
                    //              " index=" + std::to_string(index) + " "
                    //              << b1 << " " << b0 << std::endl;
                }
                tile.bit0[i] = bit0;
                tile.bit1[i] = bit1;
            }
            tile_table[start_index+y*size.x+x] = tile;
            // print_tile("assets/test_tile.txt", tile, start_index+y*size.x+x);
        }
    }
    // outfile.close();
}

void load_background_tiles(std::string dirname, PPU466::Palette palette, 
                        std::array<PPU466::Tile, 16*16>& tile_table,
                        uint8_t start_index,
                        uint8_t &end_index) {
    uint8_t count = 0;
    for (const auto & entry : fs::directory_iterator(dirname)) {
        std::cout << entry.path() << std::endl;
        if (endsWith(entry.path(), ".png")) {
            uint8_t tmp_end;
            load_background_tile(entry.path(), palette, tile_table, start_index + count, tmp_end);
            count += tmp_end - (start_index+count);
            // std::cout << std::to_string(count) << std::endl;
        }
    }
    end_index = start_index + count;
}

bool endsWith(std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

void load_background_tile(std::string filename, PPU466::Palette palette, 
                        std::array<PPU466::Tile, 16*16>& tile_table,
                        uint8_t start_index,
                        uint8_t &end_index) {
    glm::uvec2 img_size = glm::uvec2(0); // size in pixels
    std::vector< glm::u8vec4 > data; // pixel data

    //actually load the background:
    load_png(filename, &img_size, &data, LowerLeftOrigin);
    glm::uvec2 size = img_size/glm::uvec2(8,8);
    end_index = start_index + size.x*size.y;

    // std::ofstream outfile;
    // outfile.open("assets/colors_at_index_background_" + std::to_string(start_index) + ".txt");
    for (int y = 0; y < size.y; y++) {
        for (int x = 0; x < size.x; x++) {
            PPU466::Tile tile;
            for (int i = 0; i < 8; i++) {
                uint8_t bit0 = 0;
                uint8_t bit1 = 0;
                for (int j = 0; j < 8; j++) {
                    glm::u8vec4 color = data[(y*8+i)*img_size.x + 8*x+j];
                    int index = find_color_index_in_palette(color, palette);
                    bit0 |= (index & 0b01) << j;
                    bit1 |= ((index & 0b10) >> 1) << j;
                    std::bitset<8> b0(tile.bit0[i]);
                    std::bitset<8> b1(tile.bit1[i]);
                    // outfile << std::to_string(8*x+j) + ", " + std::to_string(y*8+i) +
                    //              " color: " + glm::to_string(color) + 
                    //              " tile=" + std::to_string(start_index+y*size.x+x) +
                    //              " index=" + std::to_string(index) + 
                    //              " x=" + std::to_string(x) + 
                    //              " y=" + std::to_string(y) + " "
                    //              << b1 << " " << b0 << std::endl;
                    
                }
                tile.bit0[i] = bit0;
                tile.bit1[i] = bit1;
            }
            tile_table[start_index+y*size.x+x] = tile;
            // print_tile("assets/test_tile.txt", tile, start_index+y*size.x+x);
        }
    }
    // outfile.close();
}

void print_tile(std::string filename, PPU466::Tile tile,
                uint8_t index) {
    std::ofstream outfile;
	outfile.open(filename, std::ofstream::out | std::ofstream::app);
    outfile << "tile: "  +  std::to_string(index) << std::endl;
    for (int i = 0; i < 8; i++) {
        std::bitset<8> b0(tile.bit0[i]);
        std::bitset<8> b1(tile.bit1[i]);
        outfile << b1 << "       "  << b0 << std::endl;
    }
    outfile << "\n\n" << std::endl;
    outfile.close();
}

void parse_str_to_color(std::string color_str, glm::u8vec4 &color) {
    std::array<uint8_t, 4> color_values;
    // std::cout << "line: " + color_str << std::endl;
    std::istringstream is(color_str);
    int value;

    int i = 0;
    while(is >> value) {
        color_values[i] = value;
        i++;
    }
    assert(color_values.size() == 4);
    color = glm::u8vec4(color_values[0], color_values[1], color_values[2], color_values[3]);
    // std::cout << "color values: " + glm::to_string(color) << std::endl;
}

int find_color_index_in_palette(glm::u8vec4 color, PPU466::Palette palette) {
    for (int i = 0; i < 4; i++) {
        if (color == palette[i]) {
            return i;
        }
    }
    std::cout << "color " + glm::to_string(color) + " does not exist in palette" << std::endl;
    return -1;
}