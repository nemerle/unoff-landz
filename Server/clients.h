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

#ifndef CLIENTS_H_INCLUDED
#define CLIENTS_H_INCLUDED


#include "chat.h"                   //contains definition of MAX_CHAN_SLOTS
#include "character_inventory.h"    //contains definition of MAX_INVENTORY_SLOTS
#include "character_movement.h"     //contains definition of PATH_MAX
#include "Common/character_attribute.h"
#include "Common/enhanced_actor_data.h"

#include <time.h>                   //support for time_t data type
#include <stdint.h>
#include <unordered_map>
#include <list>
#include <deque>
#include <vector>

#define MAX_CLIENTS 100

struct client_node_type{

public:
    int socket_fd;
    enum{   LOGGED_OUT=0,
            LOGGED_IN=1,
            CONNECTED=2,
            LAGGED=3
    }client_status;

    std::deque<uint8_t> packet_buffer;

    int character_id; //database id for char

    time_t time_of_last_heartbeat;
    time_t time_of_last_minute;

    std::vector<int> path;
    time_t time_of_last_move;

    int harvest_flag;
    int harvest_amount; //amount harvested each cycle
    int inventory_image_id;
    int inventory_slot;
    int bag_open;
    time_t time_of_last_harvest;

    char ip_address[16];

    std::string char_name;
    std::string password;
    int char_status;

    int active_chan;
    std::vector<int> chan; // chan0, chan1, chan2  (chan3 used for guild chat)

    int gm_permission;
    int ig_permission;

    int map_id;
    int map_tile;
    std::pair<int,int> position() const;

    EnhancedActorData actor_data;

    time_t char_created;
    time_t joined_guild;
    time_t session_commenced;


    CharacterAttributes attribute_data;
    client_inventory_type client_inventory[MAX_INVENTORY_SLOTS];

public:
    int inventory_emu() const;
    int max_inventory_emu() const;
    int race_id() const;
    int gender_id() const;
    int char_age() const;
    int id() const { return socket_fd; }
    int get_char_visual_range() const;
};

typedef std::unordered_map<uint16_t,client_node_type *> client_list_type;
extern  client_list_type clients;

#endif // CLIENTS_H_INCLUDED
