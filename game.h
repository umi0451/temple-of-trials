#pragma once
#include "map.h"
#include "objects.h"
#include <map>
#include <list>

struct Game {
	enum { EXIT_MODE, NORMAL_MODE, OPEN_MODE, CLOSE_MODE };

	Map map;
	Monster player;
	std::vector<Monster> monsters;
	std::vector<Door> doors;
	int mode;
	std::list<std::string> messages;
	int turns;

	Game();
	void generate();
	bool load(const std::string & filename);
	bool save(const std::string & filename) const;
	int get_message_count() const;
	const std::string & get_top_message() const;

	bool turn_is_ended;
	static std::map<int, Point> directions;

	void message(std::string text);
	void process_normal_mode(int ch);
	void process_open_mode(int ch);
	void process_close_mode(int ch);
	Point find_random_free_cell() const;

	void move(Monster & someone, const Point & shift);
	void open(Monster & someone, const Point & shift);
	void close(Monster & someone, const Point & shift);
};

