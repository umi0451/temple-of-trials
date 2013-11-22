#include "generate.h"
#include "ai.h"
#include "game.h"
#include <cstdlib>

namespace World {

Cell floor()
{
	return Cell('.');
}

Cell wall()
{
	return Cell('#', false);
}

Item money(const Point & pos = Point())
{
	return Item::Builder().pos(pos).sprite('$').name("money");
}

Item scorpion_tail(const Point & pos = Point())
{
	return Item::Builder().pos(pos).sprite('!').name("scorpion tail");
}

Monster player(const Point & monster_pos)
{
	return Monster::Builder().pos(monster_pos).sprite('@').sight(10).hp(20).ai(AI::PLAYER).name("you");
}

Monster ant(int ai, const Point & monster_pos)
{
	return Monster::Builder().pos(monster_pos).sprite('A').sight(6).hp(3).ai(ai).name("ant");
}

Monster scorpion(int ai, const Point & monster_pos)
{
	return Monster::Builder().pos(monster_pos).sprite('S').sight(8).hp(5).ai(ai).name("scorpion").item(scorpion_tail());
}

Door door(const Point & pos)
{
	return Door::Builder().pos(pos).opened_sprite('-').closed_sprite('+').opened(false);
}

Container pot(const Point & pos)
{
	return Container::Builder().pos(pos).sprite('^').name("pot").item(money());
}

}

void generate(Game & game)
{
	log("Generating new game...");
	game.doors.clear();
	game.monsters.clear();
	game.map = Map(60, 23, World::floor());
	game.monsters.push_back(World::player(game.find_random_free_cell()));
	for(int i = 0; i < 10; ++i) {
		Point point = game.find_random_free_cell();
		if(point) {
			game.map.cell(point.x, point.y) = World::wall();
		}
	}
	for(int i = 0; i < 5; ++i) {
		Point point = game.find_random_free_cell();
		if(point) {
			game.doors.push_back(World::door(point));
		}
	}
	for(int i = 0; i < 5; ++i) {
		Point point = game.find_random_free_cell();
		if(point) {
			game.items.push_back(World::money(point));
		}
	}
	Point point = game.find_random_free_cell();
	if(point) {
		game.containers.push_back(World::pot(point));
	}
	for(int i = 0; i < 5; ++i) {
		Point point = game.find_random_free_cell();
		if(point) {
			int ai = AI::CALM_AND_STILL;
			switch(rand() % 3) {
				case 0: ai = AI::ANGRY_AND_WANDER; break;
				case 1: ai = AI::ANGRY_AND_STILL; break;
				case 2: ai = AI::CALM_AND_STILL; break;
				default: break;
			}
			game.monsters.push_back(World::ant(ai, point));
		}
	}
	point = game.find_random_free_cell();
	if(point) {
		game.monsters.push_back(World::scorpion(AI::ANGRY_AND_WANDER, point));
	}
	log("Done.");
}

