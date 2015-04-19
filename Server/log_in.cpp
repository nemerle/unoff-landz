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

#include "log_in.h"

#include "colour.h"
#include "server_protocol_functions.h"
#include "string_functions.h"
#include "logging.h"
#include "server_messaging.h"
#include "global.h"
#include "clients.h"
#include "characters.h"
#include "maps.h"
#include "server_start_stop.h"
#include "game_data.h"

#include "db/db_character_tbl.h"
#include "Common/ClientPackets.h"

#include <stdio.h> //support for sprintf
#include <string.h> //support for strcpy

void load_char_data_into_connection(client_node_type &client){

    client.character_id = character.character_id;
    client.char_name    = character.char_name;
    client.password     = character.password;
    client.char_status  = character.char_status;
    client.active_chan  = character.active_chan;
    client.chan         = character.chan;
    client.gm_permission= character.gm_permission;
    client.ig_permission= character.ig_permission;
    client.map_id       = character.map_id;
    client.map_tile     = character.map_tile;
    client.actor_data   = character.actor_data;  // will use c++ default copy operator to copy the whole structure

    client.char_created = character.char_created;
    client.joined_guild = character.joined_guild;

    int i=0;

    for(i=0; i<MAX_INVENTORY_SLOTS; i++){
        client.client_inventory[i].image_id=character.client_inventory[i].image_id;
        client.client_inventory[i].amount=character.client_inventory[i].amount;
    }

    client.attribute_data = character.attribute_data;
}


void process_log_in(client_node_type &client, const LogIn *pkt){

    /** public function - see header **/

    char text[1024]="";
    char text_out[1024]="";
    int map_id=0;
    //int chan_colour=0;

    //check that the login packet is correct
    if(!pkt->isValid()){

        sprintf(text_out, "%cSorry, but that caused an error", c_red1+127);
        send_raw_text(client, CHAT_SERVER, text_out);

        send_login_not_ok(client);
        log_event(EVENT_ERROR, "malformed login attempt [%s]", text);

        return;
    }

    //Extract the char name and password from the login packet

    log_event(EVENT_SESSION, "login attempt char name [%s] password [%s]", pkt->m_username.c_str(),
              pkt->m_password.c_str());

    //get the char_id corresponding to the char name
    int char_id=get_db_char_data(pkt->m_username.c_str());

    if(char_id==NOT_FOUND) {

        send_you_dont_exist(client);
        send_raw_text(client, CHAT_SERVER, "unknown character name");

        log_event(EVENT_SESSION, "login rejected - unknown char name");

        return;
    }

    //the get_char_data function loads the char data into a temporary struct, so now we use the load_char_data
    //function to transfer that data into the client struct
    load_char_data_into_connection(client);

    //check we have the correct password for our char
    if(pkt->m_password.compare(client.password.c_str())!=0){

        send_login_not_ok(client);
        log_event(EVENT_SESSION, "login rejected - incorrect password");

        return;
    }

    //prevent login of dead/banned chars
    if(client.char_status!=CHAR_ALIVE){

        switch(client.char_status){

            case CHAR_DEAD:
            log_event(EVENT_SESSION, "login rejected - dead char");
            break;

            case CHAR_BANNED:
            log_event(EVENT_SESSION, "login rejected - banned char");
            break;

            default:
            log_event(EVENT_ERROR, "login rejected - unknown char status");
        }

        send_login_not_ok(client);
        return;
    }

    //prevent concurrent login on same char
    for(const auto &v : clients) {
        const client_node_type *other_client(v.second);

        if(client.character_id==other_client->character_id
           && client.client_status==client_node_type::LOGGED_IN
           && other_client->id()!=client.id()){

            send_login_not_ok(client);
            log_event(EVENT_SESSION, "concurrent login attempt for char [%s]",  pkt->m_username.c_str());

            return;
        }
    }

    client.client_status=client_node_type::LOGGED_IN;
    log_event(EVENT_SESSION, "login accepted");

/*
    // notify guild that char has logged on
    int guild_id=client.guild_id;

    if(guild_id>0) {

        chan_colour=guilds.guild[guild_id]->log_on_notification_colour;
        sprintf(text_out, "%c%s JOINED THE GAME", chan_colour, client.char_name);
        broadcast_guild_channel_chat(guild_id, text_out);
    }
*/
    //add char to map (makes scene visible in client)
    map_id=client.map_id;

    if(add_char_to_map(client, map_id, client.map_tile)==ILLEGAL_MAP){

        log_event(EVENT_ERROR, "cannot add char [%s] to map [%s] in function %s: module %s: line %i",
                   pkt->m_username.c_str(), maps.map[map_id].map_name, __func__, __FILE__, __LINE__);
        stop_server();
    }

    //record when session commenced so we can calculate time in-game
    client.session_commenced=time(nullptr);

    send_login_ok(client);
    send_you_are(client);
    send_get_active_channels(client);
    send_here_your_stats(client);
    send_here_your_inventory(client);
    send_new_minute(client, game_data.game_minutes);
}
