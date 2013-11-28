#pragma once
#include "map.h"
#include "pathfinding.h"
#include "objects.h"
#include <map>
#include <list>
class Game;

struct Control {
	enum {
		NONE,
		OPEN, CLOSE, MOVE, SWING, WAIT,
		GRAB, DROP, WIELD, UNWIELD, WEAR, TAKE_OFF, FIRE,
		DRINK, EAT,
		GO_UP, GO_DOWN,
		COUNT
	};
	int control;
	Point direction;
	int slot;
	Control(int control = NONE);
	Control(int control, const Point & direction);
	Control(int control, int slot);
	bool done() const { return control != NONE; }
};

class MapGenerator {
public:
	virtual ~MapGenerator() {}
	virtual void generate(Game & game, int level) = 0;
};

struct Game {
	struct Message {
		std::string text;
		Message(const std::string & message_text) : text(message_text) {}
		Message() {}
	};

	class MapPassabilityDetector : public PassabilityDetector {
	public:
		MapPassabilityDetector(const Game & _game);
		virtual ~MapPassabilityDetector() {}
		virtual bool is_passable(int x, int y) const;
	private:
		const Game & game;
	};

	Map map;
	int current_level;
	MapGenerator * generator;
	std::vector<Monster> monsters;
	std::vector<Door> doors;
	std::vector<Item> items;
	std::vector<Container> containers;
	std::vector<Fountain> fountains;
	std::vector<Stairs> stairs;
	bool done, player_died, completed, turn_ended;
	std::vector<std::string> messages;
	int turns;

	Game(MapGenerator * map_generator);
	bool load(const std::string & filename);
	bool save(const std::string & filename) const;
	int get_message_count() const;
	const std::string & get_top_message() const;

	void generate(int level);

	void message(std::string text);
	Point find_random_free_cell() const;

	const Monster & getPlayer() const;

	bool transparent(int x, int y) const;
	int get_sprite_at(int x, int y) const;
	int get_sprite_at(const Point & pos) const;
	std::string name_at(const Point & pos) const;

	void invalidate_fov(Monster & monster);
	void process_environment(Monster & someone);
	void die(Monster & someone);
	void hurt(Monster & someone, int damage, bool pierce_armour = false);
	void hit(Monster & someone, Monster & other, int damage);

	void move(Monster & someone, const Point & shift);
	void open(Monster & someone, const Point & shift);
	void close(Monster & someone, const Point & shift);
	void swing(Monster & someone, const Point & shift);
	void fire(Monster & someone, const Point & shift);
	void drink(Monster & someone, const Point & shift);
	void grab(Monster & someone);
	void drop(Monster & someone, int slot);
	void wield(Monster & someone, int slot);
	void unwield(Monster & someone);
	void wear(Monster & someone, int slot);
	void take_off(Monster & someone);
	void eat(Monster & someone, int slot);
	void go_up(Monster & someone);
	void go_down(Monster & someone);
};

