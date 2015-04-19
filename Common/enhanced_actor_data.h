#pragma once

#include <stdint.h>
#include <string>

struct EnhancedActorData {
    uint16_t x,y,buffs=0;
    uint16_t rotation=45;

    int guild_id;

    int char_type;
    int skin_type;
    int hair_type;
    int shirt_type;
    int pants_type;
    int boots_type;
    int head_type;
    int shield_type;
    int weapon_type;
    int cape_type;
    int helmet_type;
    int frame;
    int max_health;
    int current_health;
    int eyes;
    std::string char_name;
};
