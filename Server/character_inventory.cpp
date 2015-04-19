/******************************************************************************************************************
    Copyright 2014 UnoffLandz

    This file is part of unoff_server_4.

    unoff_server_4 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    unoff_server_4 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with unoff_server_4.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************************************************/

#include "clients.h"
#include "character_race.h"
#include "items.h"
#include "character_type.h"
#include "attributes.h"

client_inventory_type client_inventory;
item_type item[MAX_ITEMS];


int client_node_type::max_inventory_emu() const {

    return attribute[race_id()].carry_capacity[attribute_data.physique.current];
}


int client_node_type::inventory_emu() const {
    int i=0;
    int total_emu=0;
    int image_id=0;

    for(i=0; i<MAX_INVENTORY_SLOTS; i++){

        image_id  =client_inventory[i].image_id;
        total_emu += client_inventory[i].amount * item[image_id].emu;
     }

    return total_emu;

}
