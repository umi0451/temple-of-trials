#include "../objects.h"
#include "../game.h"
#include "../test.h"

SUITE(inventory) {

TEST(should_get_item)
{
	Inventory inventory;
	inventory.set_item(1, Item::Builder().sprite(1));
	EQUAL(inventory.get_item(1).sprite, 1);
}

TEST(should_get_empty_item_when_out_of_bounds)
{
	Inventory inventory;
	inventory.set_item(1, Item::Builder().sprite(1));
	ASSERT(!inventory.get_item(2));
}

TEST(should_set_item)
{
	Inventory inventory;
	bool ok = inventory.set_item(1, Item::Builder().sprite(1));
	ASSERT(ok);
	EQUAL(inventory.get_item(1).sprite, 1);
}

TEST(should_not_set_item_if_slot_is_too_large)
{
	Inventory inventory;
	bool ok = inventory.set_item(1, Item::Builder().sprite(1));
	ASSERT(!ok);
	ASSERT(!inventory.get_item(26));
}

TEST(should_unwield_any_item)
{
	Inventory inventory;
	inventory.wielded = 1;
	inventory.unwield();
	EQUAL(inventory.wielded, Inventory::NOTHING);
}

TEST(should_insert_at_first_empty_slot)
{
	Inventory inventory;
	inventory.set_item(1, Item::Builder().sprite(1));
	int slot = inventory.insert(Item::Builder().sprite(2));
	EQUAL(slot, 0);
	EQUAL(inventory.get_item(0).sprite, 2);
}

TEST(should_not_insert_when_there_is_no_place)
{
	Inventory inventory;
	for(int i = 0; i < 26; ++i) {
		inventory.set_item(i, Item::Builder().sprite(i + 1));
	}
	int slot = inventory.insert(Item::Builder().sprite(2));
	EQUAL(slot, Inventory::NOTHING);
}

TEST(should_take_item_from_inventory)
{
	Inventory inventory;
	inventory.set_item(0, Item::Builder().sprite(1));
	Item item = inventory.take_item(0);
	ASSERT(!inventory.get_item(0));
	EQUAL(item.sprite, 1);
}

TEST(should_take_first_item_from_inventory)
{
	Inventory inventory;
	inventory.set_item(0, Item::Builder().sprite(1));
	Item item = inventory.take_first_item();
	ASSERT(!inventory.get_item(0));
	EQUAL(item.sprite, 1);
}

TEST(should_not_take_first_item_from_inventory_if_empty)
{
	Inventory inventory;
	inventory.set_item(0, Item());
	Item item = inventory.take_first_item();
	ASSERT(!item);
}

TEST(should_get_quest_items_when_carrying_one)
{
	Inventory inventory;
	inventory.insert(Item::Builder().quest().sprite(1));
	const Item & item = inventory.quest_item();
	ASSERT(item);
	ASSERT(item.quest);
}

}

SUITE(monsters) {

TEST(monster_with_nonzero_hp_should_be_alive)
{
	Monster monster;
	monster.hp = 1;
	ASSERT(!monster.is_dead());
}

TEST(monster_with_zero_hp_should_die)
{
	Monster monster;
	monster.hp = 0;
	ASSERT(monster.is_dead());
}

TEST(monster_without_equipment_should_have_base_damage)
{
	Monster monster;
	monster.hit_strength = 3;
	EQUAL(monster.damage(), 3);
}

TEST(monster_with_equipment_should_have_weapon_damage_instead)
{
	Monster monster;
	monster.hit_strength = 3;
	Item weapon;
	weapon.damage = 1;
	weapon.sprite = 1;
	monster.inventory.insert(weapon);
	monster.inventory.wield(0);
	EQUAL(monster.damage(), 1);
}

}

SUITE(objects) {

TEST(should_get_opened_sprite_when_object_is_opened)
{
	Object object = Object::Builder().opened_sprite(1).closed_sprite(2).openable().opened(true);
	EQUAL(object.get_sprite(), 1);
}

TEST(should_get_closed_sprite_when_object_is_closed)
{
	Object object = Object::Builder().opened_sprite(1).closed_sprite(2).openable().opened(false);
	EQUAL(object.get_sprite(), 2);
}

TEST(opened_object_should_be_passable)
{
	Object object = Object::Builder().opened(true).passable();
	ASSERT(object.is_passable());
}

TEST(closed_object_should_be_impassable)
{
	Object object = Object::Builder().opened(false);
	ASSERT(!object.is_passable());
}

TEST(object_should_be_impassable_by_default)
{
	Object object;
	ASSERT(!object.is_passable());
}

TEST(passable_object_should_be_passable)
{
	Object object = Object::Builder().passable();
	ASSERT(object.is_passable());
}

TEST(opened_object_should_be_transparent)
{
	Object object = Object::Builder().openable().opened(true).transparent();
	ASSERT(object.is_transparent());
}

TEST(closed_object_should_be_opaque)
{
	Object object = Object::Builder().openable().opened(false).transparent();
	ASSERT(!object.is_transparent());
}

TEST(transparent_object_should_be_transparent)
{
	Object object = Object::Builder().transparent();
	ASSERT(object.is_transparent());
}

TEST(object_should_be_opaque_by_default)
{
	Object object;
	ASSERT(!object.is_transparent());
}

TEST(negative_up_destination_should_be_exit_from_dungeon)
{
	Object object = Object::Builder().up_destination(-1);
	ASSERT(object.is_exit_up());
}

TEST(negative_down_destination_should_be_exit_from_dungeon)
{
	Object object = Object::Builder().down_destination(-1);
	ASSERT(object.is_exit_down());
}

}
