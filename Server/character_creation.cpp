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



#include "string_functions.h"
#include "global.h"
#include "colour.h"
#include "server_protocol_functions.h"
#include "logging.h"
#include "server_messaging.h"
#include "idle_buffer.h"
#include "db/db_character_tbl.h"
#include "characters.h"
#include "game_data.h"
#include "character_race.h"
#include "Common/ClientPackets.h"

#include <string.h> //support for memcpy
#include <stdio.h> //support for sprintf
#include <cassert>

void check_new_character(const buffer_node_type &command, CreateChar *check_pkt)
{

    if(get_db_char_data(check_pkt->m_name.c_str())==FOUND){

        //if the char name is found, warn about duplicate
        char text_out[1024]="";
        sprintf(text_out, "%cSorry, but that character name already exists", c_red1+127);
        send_raw_text(*command.client, CHAT_SERVER, text_out);

        send_create_char_not_ok(*command.client);

        log_event(EVENT_SESSION, "Attempt to create new char with existing char name [%s]", check_pkt->m_name.c_str());
        delete check_pkt;
    }
    else {

        //if char name not found, add an entry to the database buffer to create a new char
        db_push_buffer(command.client, IDLE_BUFFER_PROCESS_ADD_NEWCHAR, check_pkt);
    }
}


void add_new_character(const buffer_node_type &command, CreateChar *check_pkt)
{
    char text_out[1024]="";
    assert(dynamic_cast<CreateChar *>(check_pkt)!=nullptr);

    //extract character data from the union and place in the character struct
    character.password = check_pkt->m_password;
    character.char_name = check_pkt->m_name;
    character.actor_data.skin_type  = check_pkt->skin;
    character.actor_data.hair_type  = check_pkt->hair;
    character.actor_data.shirt_type = check_pkt->shirt;
    character.actor_data.pants_type = check_pkt->pants;
    character.actor_data.boots_type = check_pkt->boots;
    character.actor_data.char_type  = check_pkt->type;
    character.actor_data.head_type  = check_pkt->head;

    //set the char to stand
    character.actor_data.frame=frame_stand;

    //set the char creation time
    character.char_created=time(nullptr);

    //set starting channels
    int i=0, j=0;
    for(i=0; i<MAX_CHANNELS; i++){

        if(channel[i].new_chars==1){

            if(j<MAX_CHAN_SLOTS){

                if(j==0)
                    character.active_chan=i-CHAT_CHANNEL0;
                character.chan.push_back(i);
            }
        }
    }

    //set starting map and tile
    character.map_id=game_data.beam_map_id;
    character.map_tile=game_data.beam_map_tile;

    //we use the add_db_char_data function to add character to the database. This function returns
    //an integer corresponding to the character_table id for the new record which, we'll need to
    //link to corresponding entries for the character in the inventory table
    command.client->character_id=add_db_char_data(character);

    //add initial items to inventory
    //int slot=0;
    //add_item_to_inventory(connection, 612, 1, &slot);
    //add_item_to_inventory(connection, 613, 1, &slot);
    //add_item_to_inventory(connection, 216, 1, &slot);
    //add_item_to_inventory(connection, 217, 1, &slot);

    //update game data
    race[character.actor_data.char_type].char_count++;
    game_data.char_count++;
    game_data.name_last_char_created = character.char_name;
    game_data.date_last_char_created=character.char_created;

    //notify client that character has been created
    sprintf(text_out, "%cCongratulations. You've created your new game character.", c_green3+127);
    send_raw_text(*command.client, CHAT_SERVER, text_out);
    send_create_char_ok(*command.client);

    //log character creation event
    log_event(EVENT_NEW_CHAR, "[%s] password [%s]\n",
              character.char_name.c_str(), character.password.c_str());
    delete check_pkt;
}
