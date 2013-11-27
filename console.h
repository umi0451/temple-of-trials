#pragma once
#include <string>
#include <map>
class Game;
class Monster;
class Point;

struct Console {
	void draw_game(const Game & game);
	int draw_and_get_control(Game & game);
	int draw_target_mode(Game & game, const Point & target);
	int see_messages(Game & game);
	void draw_inventory(const Game & game, const Monster & monster);
	int get_inventory_slot(const Game & game, const Monster & monster);
	void notification(const std::string & text);

	void print_tile(int x, int y, int sprite);
	void print_fow(int x, int y, int sprite);
	void print_message(const std::string & text);
	void print_stat(int row, const std::string & text);
	void clear();
	int get_control();

	static Console & instance();
private:
	unsigned messages_seen;
	std::string notification_text;
	std::map<int, std::pair<char, int> > sprites;

	Console();
	~Console();
	void init_sprites();
};
