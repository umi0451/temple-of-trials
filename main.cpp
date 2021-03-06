#include "generate.h"
#include "player.h"
#include "console.h"
#include "savefile.h"
#include <chthon/game.h>
#include <chthon/files.h>
#include <chthon/log.h>
#include <chthon/format.h>
#include <cstdlib>
#include <string>
using namespace Chthon;

const std::string SAVEFILE = "temple.sav";

static bool load_game(Game & game)
{
	if(!file_exists(SAVEFILE)) {
		game.create_new_game();
		return true;
	}
	try {
		std::ifstream in(SAVEFILE.c_str(), std::ios::in);
		if(!in) {
			throw Reader::Exception(format("Cannot open file '{0}' for reading!", SAVEFILE));
		}
		Reader savefile(in);
		load(savefile, game);
		if(remove(SAVEFILE.c_str()) != 0) {
			throw Reader::Exception("Error: cannot delete savefile!");
		}
	} catch(const Reader::Exception & e) {
		log(e.message);
		return false;
	}
	return true;
}

static void save(const Game & game)
{
	try {
		std::ofstream out(SAVEFILE.c_str(), std::ios::out);
		if(!out) {
			throw Writer::Exception(format("Cannot open file '{0}' for writing!", SAVEFILE));
		}
		Writer savefile(out);
		save(savefile, game);
	} catch(const Writer::Exception & e) {
		log(e.message);
	}
}

int main()
{
	srand((unsigned)time(nullptr));
	std::ofstream log_file("temple.log", std::ios::app);
	direct_log(&log_file);

	TempleUI console;
	Chthon::Controller * player = new PlayerControl(console);
	LinearDungeon game(player);
	console.log_messages = true;
	if(!load_game(game)) {
		return 1;
	}
	game.run();
	console.see_messages(game);
	if(game.state == Game::SUSPENDED) {
		save(game);
	}

	log("Exiting.");
	return 0;
}
