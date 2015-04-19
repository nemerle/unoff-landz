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

#include <string.h> //supports strcmp
#include <stdio.h> // supports sprintf

#include "clients.h"
#include "character_race.h"
#include "character_type.h"
#include "game_data.h"
#include "attributes.h"
#include "global.h"
#include "logging.h"
#include "string_functions.h"
#include "server_start_stop.h"
#include "colour.h"
#include "server_messaging.h"
#include "server_protocol_functions.h"
#include "characters.h"

client_node_type character;
Attribute attribute[MAX_RACES];
/**
    \brief the visual range of character, depending on ingame timeof day
    \sa broadcast_add_new_enhanced_actor_packet
    \returns visual range in [unit]
    TODO: fill in the distance units
*/
int client_node_type::get_char_visual_range() const{

    /** public function - see header */

    int _race_id=race_id();
    int visual_proximity=0;

    if(game_data.game_minutes<180){

        visual_proximity = attribute[_race_id].day_vision[attribute_data.vitality.current];
    }
    else {

        visual_proximity = attribute[_race_id].night_vision[attribute_data.instinct.current];
    }

    //prevents problems that arise where visual range attributes are zero
    if(visual_proximity<3)
        visual_proximity=3;

    return visual_proximity;
}


client_node_type * char_in_game(const char *char_name){

    /** public function - see header */

    //convert char name to upper case
    char target_name[80]="";
    strcpy(target_name, char_name);
    str_conv_upper(target_name);

    char compare_name[80]="";

    for(std::pair<const uint16_t,client_node_type *> &v : clients) {
        client_node_type *client  = v.second;
        if(client->client_status==client_node_type::LOGGED_IN){

            //convert compare name to upper case
            strcpy(compare_name, client->char_name.c_str());
            str_conv_upper(compare_name);

            //compare the target name and compare name
            if(strcmp(target_name, compare_name)==0) {

                return client;
            }
        }
    }

    return nullptr;
}


/**
 * \brief determines how long a char has been in game since creation
 * \returns time in seconds
 */
int client_node_type::char_age() const {

    /** public function - see header */

    int age=(time_of_last_minute - char_created) / (60*60*24);

    return age;
}

/*
int rename_char(int connection, char *new_char_name){

    //check that no existing char has the new name
    if(get_char_data_from_db(new_char_name)!=NOT_FOUND) return CHAR_RENAME_FAILED_DUPLICATE;

    //update char name and save
    strcpy(clients.client[connection].char_name, new_char_name);
    update_db_char_name(connection);

    // add this char to each connected client
    broadcast_add_new_enhanced_actor_packet(connection);

    return CHAR_RENAME_SUCCESS;
}
*/
