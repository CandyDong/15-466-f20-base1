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
		glm::vec2(64.0f, 24.0f),
		glm::vec2(128.0f, 24.0f),
		glm::vec2(24.0f, 56.0f),
		glm::vec2(500.0f, 240.0f),
		glm::vec2(240.0f, 360.0f),
		glm::vec2(250.0f, 128.0f),
		glm::vec2(260.0f, 256.0f)
	};

	//----- drawing handled by PPU466 -----

	PPU466 ppu;

	uint8_t PLAYER_PALETTE = 7;
	uint8_t RED_PALETTE = 4;
	uint8_t BLUE_PALETTE = 5;

	uint8_t PLAYER_TILE_START = 33;
	uint8_t PLAYER_TILE_END; // exclusive
};
