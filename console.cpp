#include "console.h"
#include "game.h"
#include "sprites.h"
#include <ncurses.h>

enum {
	MAP_WIDTH = 60,
	MAP_HEIGHT = 1 + 23
};

Console::Console()
	: messages_seen(0)
{
	initscr();
	raw();
	keypad(stdscr, TRUE);
	noecho();
	curs_set(0);
	start_color();
	
	init_sprites();
}

Console::~Console()
{
	cbreak();
	echo();
	curs_set(1);
	endwin();
}

void Console::init_sprites()
{
	for(int fore = 0; fore < 8; ++fore) {
		if(fore == 0) {
			continue;
		}
		init_pair(fore, fore, 0);
	}
	sprites[Sprites::EMPTY] = ' ' | COLOR_PAIR(0);
	sprites[Sprites::FLOOR] = '.' | COLOR_PAIR(COLOR_YELLOW);
	sprites[Sprites::WALL] = '#' | COLOR_PAIR(COLOR_YELLOW);
	sprites[Sprites::TORCH] = '&' | COLOR_PAIR(COLOR_RED) | A_BOLD;
	sprites[Sprites::GOO] = '~' | COLOR_PAIR(COLOR_GREEN) | A_BOLD;
	sprites[Sprites::EXPLOSIVE] = '*' | COLOR_PAIR(COLOR_WHITE) | A_BOLD;
	sprites[Sprites::MONEY] = '$' | COLOR_PAIR(COLOR_YELLOW);
	sprites[Sprites::SCORPION_TAIL] = '!' | COLOR_PAIR(COLOR_RED);
	sprites[Sprites::SPEAR] = '(' | COLOR_PAIR(COLOR_BLUE) | A_BOLD;
	sprites[Sprites::JACKET] = '[' | COLOR_PAIR(COLOR_BLUE) | A_BOLD;
	sprites[Sprites::ANTIDOTE] = '%' | COLOR_PAIR(COLOR_MAGENTA);
	sprites[Sprites::APPLE] = '%' | COLOR_PAIR(COLOR_GREEN);
	sprites[Sprites::PLAYER] = '@' | COLOR_PAIR(COLOR_WHITE) | A_BOLD;
	sprites[Sprites::ANT] = 'A' | COLOR_PAIR(COLOR_YELLOW) | A_BOLD;
	sprites[Sprites::SCORPION] = 'S' | COLOR_PAIR(COLOR_RED) | A_BOLD;
	sprites[Sprites::DOOR_OPENED] = '-' | COLOR_PAIR(COLOR_WHITE) | A_BOLD;
	sprites[Sprites::DOOR_CLOSED] = '+' | COLOR_PAIR(COLOR_WHITE) | A_BOLD;
	sprites[Sprites::POT] = 'V' | COLOR_PAIR(COLOR_YELLOW);
	sprites[Sprites::WELL] = '{' | COLOR_PAIR(COLOR_YELLOW) | A_BOLD;
	sprites[Sprites::GATE] = '<' | COLOR_PAIR(COLOR_WHITE) | A_BOLD;
}

void Console::print_tile(int x, int y, int sprite)
{
	if(sprites.count(sprite) > 0) {
		mvaddch(y + 1, x, sprites[sprite]);
	}
}

void Console::print_message(const std::string & text)
{
	mvprintw(MAP_HEIGHT, 0, "%s", text.c_str());
}

void Console::print_stat(int row, const std::string & text)
{
	mvprintw(row, MAP_WIDTH, "%s", text.c_str());
}

void Console::clear()
{
	::erase();
}

int Console::get_control()
{
	return getch();
}

Console & Console::instance()
{
	static Console console;
	return console;
}

void Console::notification(const std::string & text)
{
	notification_text = text;
}

int get_sprite_at(const Game & game, const Point & pos)
{
	foreach(const Monster & monster, game.monsters) {
		if(monster.pos == pos) {
			return monster.sprite;
		}
	}
	foreach(const Item & item, game.items) {
		if(item.pos == pos) {
			return item.sprite;
		}
	}
	foreach(const Container & container, game.containers) {
		if(container.pos == pos) {
			return container.sprite;
		}
	}
	foreach(const Fountain & fountain, game.fountains) {
		if(fountain.pos == pos) {
			return fountain.sprite;
		}
	}
	foreach(const Stairs & stair, game.stairs) {
		if(stair.pos == pos) {
			return stair.sprite;
		}
	}
	foreach(const Door & door, game.doors) {
		if(door.pos == pos) {
			return door.sprite();
		}
	}
	return game.map.cell(pos).sprite;
}

void Console::draw_game(const Game & game)
{
	clear();
	for(unsigned x = 0; x < game.map.width; ++x) {
		for(unsigned y = 0; y < game.map.height; ++y) {
			if(game.map.cell_properties(x, y).visible) {
				print_tile(x, y, get_sprite_at(game, Point(x, y)));
			}
		}
	}

	if(game.messages.size() > messages_seen) {
		int width, height;
		getmaxyx(stdscr, height, width);
		(void)width;
		int message_pan = height - MAP_HEIGHT;
		if(message_pan <= 0) {
			messages_seen = game.messages.size();
		} else {
			int messages_left = game.messages.size() - messages_seen;
			int message_count = std::min(messages_left, message_pan);
			for(int i = 0; i < message_count; ++i) {
				const std::string & message = game.messages[messages_seen + i];
				if(message_count < messages_left && i == message_count - 1) {
					mvprintw(MAP_HEIGHT + i, 0, "%s", (message + " (...)").c_str());
				} else {
					mvprintw(MAP_HEIGHT + i, 0, "%s", message.c_str());
				}
			}
			messages_seen += message_count;
		}
	}

	mvprintw(0, 0, "%s", notification_text.c_str());
	notification_text.clear();

	const Monster & player = game.getPlayer();
	if(!player) {
		return;
	}
	int row = 0;
	print_stat(row++, format("Turns: {0}", game.turns));
	print_stat(row++, format("HP   : {0}/{1}", player.hp, player.max_hp));
	print_stat(row++, format("Items: {0}", player.inventory.size()));
	print_stat(row++, format("Wield: {0}", player.wielded_item() ? player.wielded_item().name : "none"));
	print_stat(row++, format("Wear : {0}", player.worn_item() ? player.worn_item().name : "none"));
	print_stat(row++, format("Dmg  : {0}", player.damage()));
	row++;
	if(player.poisoning > 0) {
		print_stat(row++, "Poisoned");
	}
}

int Console::draw_and_get_control(Game & game)
{
	int ch = see_messages(game);
	return ch;
}

int Console::see_messages(Game & game)
{
	draw_game(game);
	bool ask_control = !game.done || game.completed || game.player_died;
	int ch = (!ask_control && game.messages.size() == messages_seen) ? 0 : get_control();
	while(game.messages.size() > messages_seen) {
		if(ch == ' ') {
			draw_game(game);
		}
		ch = get_control();
	}
	return ch;
}

void Console::draw_inventory(const Game & game, const Monster & monster)
{
	clear();
	int width, height;
	getmaxyx(stdscr, height, width);
	(void)height;
	int pos = 0, index = 0;
	foreach(const Item & item, monster.inventory) {
		if(item) {
			int x = (pos < 13) ? 0 : width / 2;
			int y = 1 + ((pos < 13) ? pos : pos - 13);
			std::string text = format("{0} - {1}", char(index + 'a'), item.name);
			if(monster.wielded == index) {
				text += " (wielded)";
			}
			if(monster.worn == index) {
				text += " (worn)";
			}
			mvprintw(y, x, text.c_str());
			++pos;
		}
		++index;
		if(index > 26) {
			break;
		}
	}
}

int Console::get_inventory_slot(const Game & game, const Monster & monster)
{
	draw_inventory(game, monster);

	int width, height;
	getmaxyx(stdscr, height, width);
	(void)height;
	int slot = -1;
	std::string error;
	while(true) {
		mvprintw(0, 0, "%s", std::string(width, ' ').c_str());
		mvprintw(0, 0, "%s", error.c_str());

		int ch = getch();
		if(ch == 27) {
			nodelay(stdscr, TRUE);
			ch = getch();
			nodelay(stdscr, FALSE);
			if(ch == ERR || ch == 27) {
				slot = -1;
				break;
			}
			error = "This is not a slot";
			continue;
		}
		if(ch < 'a' || 'z' < ch) {
			error = "This is not a slot";
			continue;
		}
		slot = ch - 'a';
		if(slot >= int(monster.inventory.size()) || !monster.inventory[slot]) {
			error = "Slot is empty; nothing is here.";
			continue;
		}
		break;
	}
	return slot;
}
