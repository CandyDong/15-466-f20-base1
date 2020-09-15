#include "PPU466.hpp"
#include "Mode.hpp"
#include "load_save_png.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----
	bool GAME_OVER = false;

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	//some weird background animation:
	float background_fade = 0.0f;

	//player position:
	glm::vec2 player_at = glm::vec2(0.0f);

	//opponents
	uint8_t num_opponents = 7;
	std::array<glm::vec2, 7> oppo_speeds = {
		glm::vec2(180.0f, 64.0f),
		glm::vec2(128.0f, 150.0f),
		glm::vec2(100.0f, 120.0f),
		glm::vec2(128.0f, 60.0f),
		glm::vec2(110.0f, 40.0f),
		glm::vec2(190.0f, 10.0f),
		glm::vec2(200.0f, 175.0f)
	};
	// std::array<glm::vec2, 7> oppo_speeds = {
	// 	glm::vec2(225.0f, 20.0f),
	// 	glm::vec2(225.0f, 20.0f),
	// 	glm::vec2(225.0f, 20.0f),
	// 	glm::vec2(225.0f, 20.0f),
	// 	glm::vec2(225.0f, 20.0f),
	// 	glm::vec2(225.0f, 20.0f),
	// 	glm::vec2(225.0f, 20.0f)
	// };

	//----- drawing handled by PPU466 -----

	PPU466 ppu;

	uint8_t PLAYER_PALETTE = 7;
	uint8_t RED_PALETTE = 6;
	uint8_t BLUE_PALETTE = 5;
	uint8_t BACKGROUND_PALETTE = 4;

	uint8_t PLAYER_TILE_START = 33;
	uint8_t PLAYER_TILE_END; // exclusive

	uint8_t BACKGROUND_TILE_START;
	uint8_t BACKGROUND_TILE_END;
	uint8_t BACKGROUND_TILE_GROUP_HEIGHT = 3*2;
	uint8_t BACKGROUND_TILE_GROUP_WIDTH = 8*2;

	std::array<uint8_t, 6> BACKGROUND_MAP =  {0, 8, 4, 20, 16, 12};
};
