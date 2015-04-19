#pragma once

#include <stdint.h>

template<typename T>
struct character_attribute {
    T current;
    T maximum;
};

struct CharacterAttributes {

    character_attribute<uint16_t> physique;
    character_attribute<uint16_t> coordination;
    character_attribute<uint16_t> reasoning;
    character_attribute<uint16_t> will;
    character_attribute<uint16_t> instinct;
    character_attribute<uint16_t> vitality;

    character_attribute<uint16_t> human;
    character_attribute<uint16_t> animal;
    character_attribute<uint16_t> vegetal;
    character_attribute<uint16_t> inorganic;
    character_attribute<uint16_t> artificial;
    character_attribute<uint16_t> magic;

    character_attribute<uint16_t> manufacturing;
    character_attribute<uint16_t> harvest;
    character_attribute<uint16_t> alchemy;
    character_attribute<uint16_t> overall;
    character_attribute<uint16_t> attack;
    character_attribute<uint16_t> defence;


    character_attribute<uint16_t> magic_skill;
    character_attribute<uint16_t> potion;
    character_attribute<uint16_t> material;
    character_attribute<uint16_t> ethereal;

    int food_lvl;

    character_attribute<uint32_t> manufacture_exp;
    character_attribute<uint32_t> harvest_exp;
    character_attribute<uint32_t> alchemy_exp;
    character_attribute<uint32_t> overall_exp;
    character_attribute<uint32_t> attack_exp;
    character_attribute<uint32_t> defence_exp;
    character_attribute<uint32_t> magic_exp;
    character_attribute<uint32_t> potion_exp;

    character_attribute<uint16_t> summoning;
    character_attribute<uint32_t> summoning_exp;

    character_attribute<uint16_t> crafting;
    character_attribute<uint32_t> crafting_exp;

    character_attribute<uint16_t> engineering;
    character_attribute<uint32_t> engineering_exp;

    character_attribute<uint16_t> tailoring;
    character_attribute<uint32_t> tailoring_exp;
    character_attribute<uint16_t> ranging;
    character_attribute<uint32_t> ranging_exp;

    int book_id;
    int max_book_time;
    int elapsed_book_time;
};
