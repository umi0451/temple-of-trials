#include "generate.h"
#include "ai.h"
#include "game.h"
#include "map.h"
#include "objects.h"
#include "console.h"
#include "util.h"
#include <algorithm>
#include <map>
#include <cstdlib>
#include <cstdio>

const std::string SAVEFILE = "temple.sav";

int main()
{
	srand(time(0));
	log("Log started: " + now());
	Game game;
	if(game.load(SAVEFILE)) {
		if(remove(SAVEFILE.c_str()) != 0) {
			log("Error: cannot delete savefile!");
			return 1;
		}
	} else {
		generate(game);
	}

	while(!game.done) {
		foreach(Monster & monster, game.monsters) {
			if(monster.is_dead()) {
				continue;
			}
			Controller controller = get_controller(monster.ai);
			if(!controller) {
				log(format("No controller found for AI #{0}!", monster.ai));
				continue;
			}
			Control control = controller(monster, game);
			try {
				switch(control.control) {
					case Control::MOVE: game.move(monster, control.direction); break;
					case Control::OPEN: game.open(monster, control.direction); break;
					case Control::CLOSE: game.close(monster, control.direction); break;
					case Control::SWING: game.swing(monster, control.direction); break;
					case Control::FIRE: game.fire(monster, control.direction); break;
					case Control::GRAB: game.grab(monster); break;
					case Control::DROP: game.drop(monster, control.slot); break;
					case Control::WIELD: game.wield(monster, control.slot); break;
					case Control::UNWIELD: game.unwield(monster); break;
					case Control::WEAR: game.wear(monster, control.slot); break;
					case Control::TAKE_OFF: game.take_off(monster); break;
					case Control::WAIT: break;
					default: log("Unknown control: {0}", control.control); break;
				}
			} catch(const Game::Message & msg) {
				game.message(msg.text);
			}
			if(game.done) {
				break;
			}
		}
		game.monsters.erase(std::remove_if(game.monsters.begin(), game.monsters.end(), std::mem_fun_ref(&Monster::is_dead)), game.monsters.end());
		++game.turns;
	}
	Console::instance().see_messages(game);
	if(!game.player_died) {
		game.save(SAVEFILE);
	}

	log("Exiting.");
	return 0;
}
