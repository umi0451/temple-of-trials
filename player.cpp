#include "player.h"
#include "console.h"
#include <chthon/game.h>
#include <chthon/actions.h>
using namespace Chthon;

PlayerControl::PlayerControl(TempleUI & console)
	: interface(console)
{
}

Action * PlayerControl::act(Monster & player, Game & game)
{
	while(game.state == Game::PLAYING) {
		if(!player.plan.empty()) {
			interface.draw_game(game);
			delay(10);
			Action * action = player.plan.front();
			player.plan.pop_front();
			return action;
		}
		int ch = interface.draw_and_get_control(game);
		switch(ch) {
			case 'Q':
				game.state = Game::PLAYER_DIED;
				interface.message("You commited suicide.");
				break;
			case 'q':
				game.state = Game::SUSPENDED;
				break;
			case 'x':
				player.add_path(game.current_level().find_path(player.pos, interface.target_mode(game, player.pos)));
				break;
			case 'i':
				interface.draw_inventory(game, player);
				interface.get_control();
				break;
			case 'h': case 'j': case 'k': case 'l': case 'y': case 'u': case 'b': case 'n':
			{
				Point shift = interface.directions[ch];
				Point new_pos = player.pos + shift;
				if(find_at(game.current_level().monsters, new_pos).valid()) {
					return new Swing(shift);
				}
				Object & object = find_at(game.current_level().objects, new_pos);
				if(object.valid()) {
					if(object.type->openable && !object.opened()) {
						player.plan.push_front(new Move(shift));
						return new Open(shift);
					}
					if(object.type->containable) {
						return new Open(shift);
					}
					if(object.type->drinkable) {
						return new Drink(shift);
					}
				}
				return new Move(shift);
			}
			case '<': return new GoUp();
			case '>': return new GoDown();
			case 'g': return new Grab();
			case 'w': return new Wield(interface.get_inventory_slot(game, player));
			case 'W': return new Wear(interface.get_inventory_slot(game, player));
			case 't': return new Unwield();
			case 'T': return new TakeOff();
			case 'e': return new Eat(interface.get_inventory_slot(game, player));
			case 'd': return new Drop(interface.get_inventory_slot(game, player));
			case '.': return new Wait();
			case 'D': return new Drink(interface.draw_and_get_direction(game));
			case 'f': return new Fire(interface.draw_and_get_direction(game));
			case 'p': return new Put(interface.draw_and_get_direction(game));
			case 's': return new Swing(interface.draw_and_get_direction(game));
			case 'o': return new Open(interface.draw_and_get_direction(game));
			case 'c': return new Close(interface.draw_and_get_direction(game));
			default: interface.set_notification(format("Unknown control '{0}'", char(ch)));
		}
	}
	return nullptr;
}

