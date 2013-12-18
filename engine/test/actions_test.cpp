#include "../actions.h"
#include "../game.h"
#include "../test.h"

std::string to_string(const Control & control)
{
	return format("Control({0}, dir={1}, slot={2})", control.control, control.direction, control.slot);
}

SUITE(actions) {

struct GameWithDummy {
	Game game;
	GameWithDummy() {
		game.level.map = Map(2, 2);
		game.level.map.celltypes.front().passable = true;
		Item armor = Item::Builder().sprite(1).wearable().defence(3).name("item");
		game.level.monsters.push_back(Monster::Builder().pos(Point(1, 1)).hp(100).name("dummy").item(armor));
	}
	Monster & dummy() { return game.level.monsters[0]; }
};

TEST_FIXTURE(GameWithDummy, should_move_on_smart_move_if_passable)
{
	smart_move(game, dummy(), Point(0, -1));
	EQUAL(dummy().pos, Point(1, 0));
	ASSERT(game.messages.empty());
}

TEST_FIXTURE(GameWithDummy, should_open_door_on_smart_move_if_exists)
{
	game.level.objects.push_back(Object::Builder().pos(Point(1, 0)).openable().opened(false).name("door"));
	smart_move(game, dummy(), Point(0, -1));
	ASSERT(game.level.objects.front().opened);
	EQUAL(dummy().pos, Point(1, 1));
	EQUAL(game.messages, MakeVector<std::string>("Dummy opened the door.").result);
}

TEST_FIXTURE(GameWithDummy, should_plan_to_move_in_just_opened_door_on_smart)
{
	game.level.objects.push_back(Object::Builder().pos(Point(1, 0)).openable().opened(false).name("door"));
	smart_move(game, dummy(), Point(0, -1));
	EQUAL_CONTAINERS(dummy().plan, MakeVector<Control>(Control(Control::MOVE, Point(0, -1))).result);
}

TEST_FIXTURE(GameWithDummy, should_open_container_on_smart_move_if_exists)
{
	Item apple = Item::Builder().sprite(1).name("apple");
	game.level.objects.push_back(Object::Builder().pos(Point(1, 0)).name("pot").containable().item(apple));
	smart_move(game, dummy(), Point(0, -1));
	ASSERT(game.level.objects.front().items.empty());
	EQUAL(dummy().pos, Point(1, 1));
	EQUAL(game.level.items, MakeVector<Item>(Item::Builder().sprite(1).name("apple").pos(Point(1, 1))).result);
	EQUAL(game.messages, MakeVector<std::string>("Dummy took up a apple from pot.").result);
}

TEST_FIXTURE(GameWithDummy, should_drink_from_fountain_on_smart_move_if_exists)
{
	game.level.objects.push_back(Object::Builder().pos(Point(1, 0)).name("well").drinkable());
	smart_move(game, dummy(), Point(0, -1));
	EQUAL(dummy().pos, Point(1, 1));
	EQUAL(game.messages, MakeVector<std::string>("Dummy drink from well.").result);
}

TEST_FIXTURE(GameWithDummy, should_swing_at_monster_on_smart_move_if_exists)
{
	game.level.monsters.push_back(Monster::Builder().pos(Point(1, 0)).name("stub"));
	smart_move(game, dummy(), Point(0, -1));
	EQUAL(dummy().pos, Point(1, 1));
	EQUAL(game.messages, MakeVector<std::string>("Dummy hit stub for 0 hp.").result);
}


TEST_FIXTURE(GameWithDummy, should_not_drink_monsters)
{
	game.level.monsters.push_back(Monster::Builder().pos(Point(1, 0)).name("stub"));
	drink(game, dummy(), Point(0, -1));
	EQUAL(game.messages, MakeVector<std::string>("It is stub. dummy is not a vampire to drink that.").result);
}

TEST_FIXTURE(GameWithDummy, should_not_drink_containers)
{
	game.level.objects.push_back(Object::Builder().pos(Point(1, 0)).name("pot").containable());
	drink(game, dummy(), Point(0, -1));
	EQUAL(game.messages, MakeVector<std::string>("Unfortunately, pot is totally empty.").result);
}

TEST_FIXTURE(GameWithDummy, should_not_drink_at_empty_cell)
{
	drink(game, dummy(), Point(0, -1));
	EQUAL(game.messages, MakeVector<std::string>("There is nothing to drink.").result);
}

TEST_FIXTURE(GameWithDummy, should_drink_fountains)
{
	game.level.objects.push_back(Object::Builder().pos(Point(1, 0)).name("well").drinkable());
	drink(game, dummy(), Point(0, -1));
	EQUAL(game.messages, MakeVector<std::string>("Dummy drink from well.").result);
}

TEST_FIXTURE(GameWithDummy, should_heal_from_fountains)
{
	game.level.objects.push_back(Object::Builder().pos(Point(1, 0)).name("well").drinkable());
	dummy().hp -= 5;
	drink(game, dummy(), Point(0, -1));
	EQUAL(dummy().hp, 96);
	EQUAL(game.messages, MakeVector<std::string>("Dummy drink from well. It helps a bit.").result);
}


TEST_FIXTURE(GameWithDummy, should_not_open_already_opened_doors)
{
	game.level.objects.push_back(Object::Builder().pos(Point(1, 0)).name("door").openable().opened(true));
	open(game, dummy(), Point(0, -1));
	ASSERT(game.level.objects[0].opened);
	EQUAL(game.messages, MakeVector<std::string>("Door is already opened.").result);
}

TEST_FIXTURE(GameWithDummy, should_open_closed_doors)
{
	game.level.objects.push_back(Object::Builder().pos(Point(1, 0)).name("door").openable().opened(false));
	open(game, dummy(), Point(0, -1));
	ASSERT(game.level.objects[0].opened);
	EQUAL(game.messages, MakeVector<std::string>("Dummy opened the door.").result);
}

TEST_FIXTURE(GameWithDummy, should_open_locked_doors_without_a_key)
{
	game.level.objects.push_back(Object::Builder().pos(Point(1, 0)).name("door").openable().opened(false).locked(true).lock_type(1));
	open(game, dummy(), Point(0, -1));
	ASSERT(game.level.objects[0].locked);
	ASSERT(!game.level.objects[0].opened);
	EQUAL(game.messages, MakeVector<std::string>("Door is locked.").result);
}

TEST_FIXTURE(GameWithDummy, should_open_locked_doors_with_a_key)
{
	game.level.objects.push_back(Object::Builder().pos(Point(1, 0)).name("door").openable().opened(false).locked(true).lock_type(1));
	dummy().inventory.push_back(Item::Builder().sprite(1).key_type(1));
	open(game, dummy(), Point(0, -1));
	ASSERT(!game.level.objects[0].locked);
	ASSERT(game.level.objects[0].opened);
	EQUAL(game.messages, MakeVector<std::string>("Dummy unlocked the door.")("Dummy opened the door.").result);
}

TEST_FIXTURE(GameWithDummy, should_not_open_empty_cell)
{
	open(game, dummy(), Point(0, -1));
	EQUAL(game.messages, MakeVector<std::string>("There is nothing to open there.").result);
}

TEST_FIXTURE(GameWithDummy, should_open_containers_and_drop_items)
{
	Item item = Item::Builder().sprite(1).name("item");
	game.level.objects.push_back(Object::Builder().pos(Point(1, 0)).name("pot").containable().item(item));
	open(game, dummy(), Point(0, -1));
	EQUAL(game.level.items[0].sprite, item.sprite);
	ASSERT(game.level.objects[0].items.empty());
	EQUAL(game.messages, MakeVector<std::string>("Dummy took up a item from pot.").result);
}

TEST_FIXTURE(GameWithDummy, should_not_open_empty_containers)
{
	game.level.objects.push_back(Object::Builder().pos(Point(1, 0)).name("pot").containable());
	open(game, dummy(), Point(0, -1));
	ASSERT(game.level.items.empty());
	ASSERT(game.level.objects[0].items.empty());
	EQUAL(game.messages, MakeVector<std::string>("Pot is empty.").result);
}


TEST_FIXTURE(GameWithDummy, should_close_opened_doors)
{
	game.level.objects.push_back(Object::Builder().pos(Point(1, 0)).name("door").openable().opened(true));
	close(game, dummy(), Point(0, -1));
	ASSERT(!game.level.objects[0].opened);
	EQUAL(game.messages, MakeVector<std::string>("Dummy closed the door.").result);
}

TEST_FIXTURE(GameWithDummy, should_not_close_already_closed_doors)
{
	game.level.objects.push_back(Object::Builder().pos(Point(1, 0)).name("door").openable().opened(false));
	close(game, dummy(), Point(0, -1));
	ASSERT(!game.level.objects[0].opened);
	EQUAL(game.messages, MakeVector<std::string>("Door is already closed.").result);
}

TEST_FIXTURE(GameWithDummy, should_not_close_empty_cell)
{
	close(game, dummy(), Point(0, -1));
	EQUAL(game.messages, MakeVector<std::string>("There is nothing to close there.").result);
}


TEST_FIXTURE(GameWithDummy, should_hit_impassable_cells_on_swing)
{
	int wall = game.level.map.add_cell_type(CellType::Builder().name("wall").passable(false));
	game.level.map.set_cell_type(Point(1, 0), wall);
	swing(game, dummy(), Point(0, -1));
	EQUAL(game.messages, MakeVector<std::string>("Dummy hit wall.").result);
}

TEST_FIXTURE(GameWithDummy, should_open_closed_doors_on_swing)
{
	game.level.objects.push_back(Object::Builder().pos(Point(1, 0)).name("door").openable().opened(false));
	swing(game, dummy(), Point(0, -1));
	ASSERT(game.level.objects[0].opened);
	EQUAL(game.messages, MakeVector<std::string>("Dummy swing at door.")("Dummy opened the door.").result);
}

TEST_FIXTURE(GameWithDummy, should_hit_monsters_on_swing)
{
	game.level.monsters.push_back(Monster::Builder().pos(Point(1, 0)).name("stub"));
	swing(game, dummy(), Point(0, -1));
	EQUAL(game.messages, MakeVector<std::string>("Dummy hit stub for 0 hp.").result);
}

TEST_FIXTURE(GameWithDummy, should_swing_at_nothing_at_empty_cell)
{
	swing(game, dummy(), Point(0, -1));
	EQUAL(game.messages, MakeVector<std::string>("Dummy swing at nothing.").result);
}


struct GameWithDummyWieldingAndWearing {
	Game game;
	GameWithDummyWieldingAndWearing() {
		game.level.map = Map(2, 3);
		int floor = game.level.map.add_cell_type(CellType::Builder().passable(true).transparent(true).name("floor"));
		game.level.map.fill(floor);
		Item armor = Item::Builder().sprite(1).wearable().defence(3).name("armor");
		Item spear = Item::Builder().sprite(2).damage(3).name("spear");
		game.level.monsters.push_back(Monster::Builder().pos(Point(1, 2)).hp(100).name("dummy").item(spear).item(armor).wield(0).wear(1));
	}
	Monster & dummy() { return game.level.monsters[0]; }
};

TEST_FIXTURE(GameWithDummyWieldingAndWearing, should_not_throw_if_wields_nothing)
{
	dummy().wielded = -1;
	fire(game, dummy(), Point(0, -1));
	EQUAL(game.messages, MakeVector<std::string>("Dummy have nothing to throw.").result);
}

TEST_FIXTURE(GameWithDummyWieldingAndWearing, should_remove_item_from_monster_when_thrown)
{
	fire(game, dummy(), Point(0, -1));
	ASSERT(!dummy().inventory[0]);
}

TEST_FIXTURE(GameWithDummyWieldingAndWearing, should_unwield_item_from_monster_when_thrown)
{
	fire(game, dummy(), Point(0, -1));
	EQUAL(dummy().wielded, -1);
}

TEST_FIXTURE(GameWithDummyWieldingAndWearing, should_hit_opaque_cell_and_drop_item_before_it)
{
	int wall = game.level.map.add_cell_type(CellType::Builder().name("wall").transparent(false));
	game.level.map.set_cell_type(Point(1, 0), wall);
	fire(game, dummy(), Point(0, -1));
	EQUAL(game.messages, MakeVector<std::string>("Dummy throw spear.")("Spear hit wall.").result);
	ASSERT(!game.level.items.empty());
	EQUAL(game.level.items[0].pos, Point(1, 1));
}

TEST_FIXTURE(GameWithDummyWieldingAndWearing, should_hit_closed_door_and_drop_item_before_it)
{
	game.level.objects.push_back(Object::Builder().pos(Point(1, 0)).name("door").openable().opened(false));
	fire(game, dummy(), Point(0, -1));
	EQUAL(game.messages, MakeVector<std::string>("Dummy throw spear.")("Spear hit door.").result);
	ASSERT(!game.level.items.empty());
	EQUAL(game.level.items[0].pos, Point(1, 1));
}

TEST_FIXTURE(GameWithDummyWieldingAndWearing, should_hit_container_and_drop_item_in_it)
{
	game.level.objects.push_back(Object::Builder().pos(Point(1, 0)).name("pot").containable());
	fire(game, dummy(), Point(0, -1));
	EQUAL(game.messages, MakeVector<std::string>("Dummy throw spear.")("Spear falls into pot.").result);
	ASSERT(game.level.items.empty());
	ASSERT(!game.level.objects[0].items.empty());
	EQUAL(game.level.objects[0].items[0].name, "spear");
}

TEST_FIXTURE(GameWithDummyWieldingAndWearing, should_hit_fountain_and_erase_item_forever)
{
	game.level.objects.push_back(Object::Builder().pos(Point(1, 0)).name("well").drinkable());
	fire(game, dummy(), Point(0, -1));
	EQUAL(game.messages, MakeVector<std::string>("Dummy throw spear.")("Spear falls into well. Forever lost.").result);
	ASSERT(game.level.items.empty());
}

TEST_FIXTURE(GameWithDummyWieldingAndWearing, should_hit_monster_and_drop_item_under_it)
{
	game.level.monsters.push_back(Monster::Builder().pos(Point(1, 0)).name("stub").hp(100));
	fire(game, dummy(), Point(0, -1));
	EQUAL(game.messages, MakeVector<std::string>("Dummy throw spear.")("Spear hits stub.")("Dummy hit stub for 3 hp.").result);
	ASSERT(!game.level.items.empty());
	EQUAL(game.level.items[0].pos, Point(1, 0));
}


TEST_FIXTURE(GameWithDummyWieldingAndWearing, should_not_drop_if_nothing_to_drop)
{
	dummy().inventory.clear();
	drop(game, dummy(), 0);
	EQUAL(game.messages, MakeVector<std::string>("Dummy have nothing to drop.").result);
}

TEST_FIXTURE(GameWithDummyWieldingAndWearing, should_drop_items_only_in_range)
{
	drop(game, dummy(), 4);
	EQUAL(game.messages, MakeVector<std::string>("No such object.").result);
}

TEST_FIXTURE(GameWithDummyWieldingAndWearing, should_unwield_item_before_dropping)
{
	drop(game, dummy(), 0);
	EQUAL(dummy().wielded, -1);
	EQUAL(game.messages, MakeVector<std::string>("Dummy unwields spear.")("Dummy dropped spear on the floor.").result);
}

TEST_FIXTURE(GameWithDummyWieldingAndWearing, should_take_off_item_before_dropping)
{
	drop(game, dummy(), 1);
	EQUAL(dummy().worn, -1);
	EQUAL(game.messages, MakeVector<std::string>("Dummy takes off armor.")("Dummy dropped armor on the floor.").result);
}

TEST_FIXTURE(GameWithDummyWieldingAndWearing, should_remove_item_from_inventory_when_dropped)
{
	drop(game, dummy(), 0);
	ASSERT(!dummy().inventory[0]);
}

TEST_FIXTURE(GameWithDummyWieldingAndWearing, should_place_item_on_the_floor_when_dropped)
{
	drop(game, dummy(), 0);
	EQUAL(game.level.items.size(), 1);
	EQUAL(game.level.items[0].name, "spear");
}


TEST_FIXTURE(GameWithDummyWieldingAndWearing, should_not_grab_if_floor_is_empty)
{
	grab(game, dummy());
	EQUAL(game.messages, MakeVector<std::string>("Nothing here to pick up.").result);
}

TEST_FIXTURE(GameWithDummyWieldingAndWearing, should_put_grabbed_item_to_the_first_empty_slot)
{
	game.level.items.push_back(Item::Builder().pos(Point(1, 2)).name("item").sprite(1));
	grab(game, dummy());
	EQUAL(dummy().inventory[2].name, "item");
	EQUAL(game.messages, MakeVector<std::string>("Dummy picked up item from the floor.").result);
}

TEST_FIXTURE(GameWithDummyWieldingAndWearing, should_remove_grabbed_item_from_map)
{
	game.level.items.push_back(Item::Builder().pos(Point(1, 2)).name("item").sprite(1));
	grab(game, dummy());
	ASSERT(game.level.items.empty());
}

TEST_FIXTURE(GameWithDummyWieldingAndWearing, should_notify_if_quest_item)
{
	game.level.items.push_back(Item::Builder().pos(Point(1, 2)).name("item").sprite(1).quest());
	grab(game, dummy());
	EQUAL(game.messages, MakeVector<std::string>("Dummy picked up item from the floor.")("Now bring it back to the surface!").result);
}

TEST_FIXTURE(GameWithDummyWieldingAndWearing, should_not_grab_item_if_inventory_is_full)
{
	for(int i = 2; i < 26; ++i) {
		dummy().inventory.push_back(Item::Builder().name("stub").sprite(2));
	}
	game.level.items.push_back(Item::Builder().pos(Point(1, 2)).name("item").sprite(1));
	grab(game, dummy());
	EQUAL(game.messages, MakeVector<std::string>("Dummy carry too much items.").result);
}


struct GameWithDummyWithItems {
	Game game;
	GameWithDummyWithItems() {
		game.level.map = Map(2, 3);
		int floor = game.level.map.add_cell_type(CellType::Builder().passable(true).transparent(true).name("floor"));
		game.level.map.fill(floor);
		Item armor = Item::Builder().sprite(1).wearable().defence(3).name("armor");
		Item spear = Item::Builder().sprite(2).damage(3).name("spear");
		game.level.monsters.push_back(Monster::Builder().pos(Point(1, 2)).hp(100).name("dummy").item(spear).item(armor).item(Item()));
	}
	Monster & dummy() { return game.level.monsters[0]; }
};

TEST_FIXTURE(GameWithDummyWithItems, should_wield_any_item)
{
	wield(game, dummy(), 0);
	EQUAL(game.messages, MakeVector<std::string>("Dummy wields spear.").result);
}

TEST_FIXTURE(GameWithDummyWithItems, should_not_wield_invalid_slot)
{
	wield(game, dummy(), 3);
	EQUAL(game.messages, MakeVector<std::string>("No such object.").result);
}

TEST_FIXTURE(GameWithDummyWithItems, should_not_wield_empty_slot)
{
	wield(game, dummy(), 2);
	EQUAL(game.messages, MakeVector<std::string>("No such object.").result);
}

TEST_FIXTURE(GameWithDummyWieldingAndWearing, should_unwield_previous_item_before_wielding_new)
{
	dummy().inventory.push_back(Item::Builder().sprite(1).name("sword"));
	wield(game, dummy(), 2);
	EQUAL(game.messages, MakeVector<std::string>("Dummy unwields spear.")("Dummy wields sword.").result);
}

TEST_FIXTURE(GameWithDummyWieldingAndWearing, should_take_off_item_before_wielding_it)
{
	wield(game, dummy(), 1);
	EQUAL(game.messages, MakeVector<std::string>("Dummy unwields spear.")("Dummy takes off armor.")("Dummy wields armor.").result);
}


TEST_FIXTURE(GameWithDummyWieldingAndWearing, should_unwield_item_if_wielded)
{
	unwield(game, dummy());
	EQUAL(game.messages, MakeVector<std::string>("Dummy unwields spear.").result);
}

TEST_FIXTURE(GameWithDummyWithItems, should_not_unwield_item_if_not_wielded)
{
	unwield(game, dummy());
	EQUAL(game.messages, MakeVector<std::string>("Dummy is wielding nothing.").result);
}


TEST_FIXTURE(GameWithDummyWithItems, should_wear_any_item)
{
	wear(game, dummy(), 1);
	EQUAL(game.messages, MakeVector<std::string>("Dummy wear armor.").result);
}

TEST_FIXTURE(GameWithDummyWithItems, should_not_wear_invalid_slot)
{
	wear(game, dummy(), 3);
	EQUAL(game.messages, MakeVector<std::string>("No such object.").result);
}

TEST_FIXTURE(GameWithDummyWithItems, should_not_wear_empty_slot)
{
	wear(game, dummy(), 2);
	EQUAL(game.messages, MakeVector<std::string>("No such object.").result);
}

TEST_FIXTURE(GameWithDummyWithItems, should_not_wear_unwearable_item)
{
	dummy().inventory[2].sprite = 1;
	dummy().inventory[2].name = "pot";
	wear(game, dummy(), 2);
	EQUAL(game.messages, MakeVector<std::string>("Pot cannot be worn.").result);
}

TEST_FIXTURE(GameWithDummyWieldingAndWearing, should_take_off_previous_item_before_wearing_new)
{
	dummy().inventory.push_back(Item::Builder().sprite(1).name("jacket").wearable());
	wear(game, dummy(), 2);
	EQUAL(game.messages, MakeVector<std::string>("Dummy takes off armor.")("Dummy wear jacket.").result);
}

TEST_FIXTURE(GameWithDummyWieldingAndWearing, should_unwield_item_before_wearing_it)
{
	dummy().inventory[0].wearable = true;
	wear(game, dummy(), 0);
	EQUAL(game.messages, MakeVector<std::string>("Dummy takes off armor.")("Dummy unwields spear.")("Dummy wear spear.").result);
}


TEST_FIXTURE(GameWithDummyWieldingAndWearing, should_take_off_item_if_worn)
{
	take_off(game, dummy());
	EQUAL(game.messages, MakeVector<std::string>("Dummy takes off armor.").result);
}

TEST_FIXTURE(GameWithDummyWithItems, should_not_take_off_item_if_not_worn)
{
	take_off(game, dummy());
	EQUAL(game.messages, MakeVector<std::string>("Dummy is wearing nothing.").result);
}


struct GameWithDummyAndFood {
	Game game;
	enum { ARMOR, SPEAR, JUNK, FOOD, MEDKIT, MEGASPHERE, ANTIDOTE, EMPTY, NONE };
	GameWithDummyAndFood() {
		game.level.map = Map(2, 2);
		Item armor = Item::Builder().sprite(1).wearable().defence(3).name("armor").edible();
		Item spear = Item::Builder().sprite(2).damage(3).name("spear").edible();
		Item junk = Item::Builder().sprite(3).name("junk");
		Item food = Item::Builder().sprite(4).name("food").edible();
		Item medkit = Item::Builder().sprite(4).name("medkit").edible().healing(5);
		Item megasphere = Item::Builder().sprite(4).name("megasphere").edible().healing(100);
		Item antidote = Item::Builder().sprite(4).name("antidote").edible().antidote(5);
		Item empty;
		game.level.monsters.push_back(Monster::Builder().pos(Point(1, 1)).hp(100).name("dummy").wield(1).wear(0));
		dummy().inventory << armor << spear << junk << food << medkit << megasphere << antidote << empty;
		dummy().hp = 90;
	}
	Monster & dummy() { return game.level.monsters[0]; }
};

TEST_FIXTURE(GameWithDummyAndFood, should_not_eat_invalid_slot)
{
	eat(game, dummy(), NONE);
	EQUAL(game.messages, MakeVector<std::string>("No such object.").result);
}

TEST_FIXTURE(GameWithDummyAndFood, should_not_eat_empty_slot)
{
	eat(game, dummy(), EMPTY);
	EQUAL(game.messages, MakeVector<std::string>("No such object.").result);
}

TEST_FIXTURE(GameWithDummyAndFood, should_not_eat_not_edible_item)
{
	eat(game, dummy(), JUNK);
	EQUAL(game.messages, MakeVector<std::string>("Junk isn't edible.").result);
}

TEST_FIXTURE(GameWithDummyAndFood, should_take_off_item_before_eating)
{
	eat(game, dummy(), ARMOR);
	EQUAL(game.messages, MakeVector<std::string>("Dummy takes off armor.")("Dummy eats armor.").result);
}

TEST_FIXTURE(GameWithDummyAndFood, should_unwield_item_before_eating)
{
	eat(game, dummy(), SPEAR);
	EQUAL(game.messages, MakeVector<std::string>("Dummy unwields spear.")("Dummy eats spear.").result);
}

TEST_FIXTURE(GameWithDummyAndFood, should_eat_items)
{
	eat(game, dummy(), FOOD);
	EQUAL(game.messages, MakeVector<std::string>("Dummy eats food.").result);
}

TEST_FIXTURE(GameWithDummyAndFood, should_heal_when_eating_healing_item)
{
	eat(game, dummy(), MEDKIT);
	EQUAL(dummy().hp, 95);
	EQUAL(game.messages, MakeVector<std::string>("Dummy eats medkit.")("Medkit heals dummy.").result);
}

TEST_FIXTURE(GameWithDummyAndFood, should_heal_up_to_max_hp_when_eating_healing_item)
{
	eat(game, dummy(), MEGASPHERE);
	EQUAL(dummy().hp, 100);
	EQUAL(game.messages, MakeVector<std::string>("Dummy eats megasphere.")("Megasphere heals dummy.").result);
}

TEST_FIXTURE(GameWithDummyAndFood, should_cure_poisoning_when_eating_antidote)
{
	dummy().poisoning = 10;
	eat(game, dummy(), ANTIDOTE);
	EQUAL(dummy().poisoning, 5);
	EQUAL(game.messages, MakeVector<std::string>("Dummy eats antidote.")("Antidote cures poisoning a little.").result);
}

TEST_FIXTURE(GameWithDummyAndFood, should_cure_poisoning_to_the_end_when_eating_antidote)
{
	dummy().poisoning = 5;
	eat(game, dummy(), ANTIDOTE);
	EQUAL(dummy().poisoning, 0);
	EQUAL(game.messages, MakeVector<std::string>("Dummy eats antidote.")("Antidote cures poisoning.").result);
}


class TestLevelGenerator : public LevelGenerator {
public:
	TestLevelGenerator(const Point & player_pos1, const Point & player_pos2)
		: generated(false), pos1(player_pos1), pos2(player_pos2) { }
	virtual void generate(Level & level, int level_index)
	{
		generated = true;
		level = Level(4, 4);
		if(level_index == 1) {
			level.monsters.push_back(Monster::Builder().sprite(1).faction(Monster::PLAYER).pos(pos1));
		} else {
			level.monsters.push_back(Monster::Builder().sprite(2).faction(Monster::PLAYER).pos(pos2));
		}
	}
	bool was_generated() const { return generated; }
private:
	bool generated;
	Point pos1, pos2;
};

struct GameWithDummyAndStairs {
	TestLevelGenerator generator;
	Game game;
	GameWithDummyAndStairs()
		: generator(Point(1, 1), Point(2, 2)), game(&generator)
	{
		game.level.map = Map(2, 2);
		game.level.monsters.push_back(Monster::Builder().pos(Point(1, 1)).name("dummy"));
		game.level.objects.push_back(Object::Builder().pos(Point(1, 1)).name("stairs").transporting());
	}
	Monster & dummy() { return game.level.monsters[0]; }
	Object & stairs() { return game.level.objects[0]; }
};

TEST_FIXTURE(GameWithDummyAndStairs, should_go_up_on_upstairs)
{
	stairs().up_destination = 1;
	go_up(game, dummy());
	EQUAL(game.messages, MakeVector<std::string>("Dummy goes up.").result);
}

TEST_FIXTURE(GameWithDummyAndStairs, should_send_to_quest_on_upstairs_to_the_surface)
{
	stairs().up_destination = -1;
	go_up(game, dummy());
	EQUAL(game.messages, MakeVector<std::string>("Dummy must complete mission in order to go back to the surface.").result);
}

TEST_FIXTURE(GameWithDummyAndStairs, should_win_game_on_upstairs_when_have_quest_item)
{
	dummy().inventory.push_back(Item::Builder().sprite(1).quest().name("Yendor"));
	stairs().up_destination = -1;
	go_up(game, dummy());
	EQUAL(game.state, Game::COMPLETED);
	EQUAL(game.messages, MakeVector<std::string>("Dummy have brought Yendor to the surface. Yay! Game if finished.").result);
}

TEST_FIXTURE(GameWithDummyAndStairs, should_generate_corresponding_level_when_going_up)
{
	stairs().up_destination = 1;
	go_up(game, dummy());
	ASSERT(generator.was_generated());
}


TEST_FIXTURE(GameWithDummyAndStairs, should_go_down_on_downstairs)
{
	stairs().down_destination = 1;
	go_down(game, dummy());
	EQUAL(game.messages, MakeVector<std::string>("Dummy goes down.").result);
}

TEST_FIXTURE(GameWithDummyAndStairs, should_send_to_quest_on_downstairs_to_the_surface)
{
	stairs().down_destination = -1;
	go_down(game, dummy());
	EQUAL(game.messages, MakeVector<std::string>("Dummy must complete mission in order to go back to the surface.").result);
}

TEST_FIXTURE(GameWithDummyAndStairs, should_win_game_on_downstairs_when_have_quest_item)
{
	dummy().inventory.push_back(Item::Builder().sprite(1).quest().name("Yendor"));
	stairs().down_destination = -1;
	go_down(game, dummy());
	EQUAL(game.state, Game::COMPLETED);
	EQUAL(game.messages, MakeVector<std::string>("Dummy have brought Yendor to the surface. Yay! Game if finished.").result);
}

TEST_FIXTURE(GameWithDummyAndStairs, should_generate_corresponding_level_when_going_down)
{
	stairs().down_destination = 1;
	go_down(game, dummy());
	ASSERT(generator.was_generated());
}

}
