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
#include "server_protocol_functions.h"


#include "character_inventory.h"
#include "clients.h"
#include "maps.h"
#include "logging.h"
#include "Common/ServerPackets.h"

#include <string.h> //support for memmove strlen
#include <sys/socket.h> //needed for send function
#include <stdio.h>
#include <stdint.h>
#include <alloca.h>
#include <cassert>
#include <vector>
#define DEBUG_SEND 0

void send_packet(int connection, const uint8_t *packet, int packet_length){

    /** public function - see header */

    char text[1024]="";

    int i=0;
    for(i=0; i<packet_length; i++){

        sprintf(text, "%s %i", text, packet[i]);
    }

    log_event(EVENT_PACKET, "send to [%i]%s", connection, text);

    send(connection, packet, packet_length, 0);
}

void send_packet(const client_node_type &client, const std::vector<uint8_t> &buf) {
    send_packet(client.socket_fd,buf.data(),buf.size());
}


/** public function - see header */
void send_new_minute(const client_node_type &client, int16_t minute){

#if DEBUG_SEND==1
    printf("NEW_MINUTE connection [%i] minute [%i]\n", client.id(), minute);
#endif

    log_event(EVENT_SESSION, "NEW_MINUTE connection [%i] minute [%i]", client.id(), minute);

    send_packet(client, NewMinute(minute));
}


/** public function - see header */
void send_login_ok(const client_node_type &client) {

#if DEBUG_SEND==1
    printf("LOG_IN_OK connection [%i]\n", client.id());
#endif

    log_event(EVENT_SESSION, "LOG_IN_OK connection [%i]", client.id());

    send_packet(client, LogInOk());
}


/** public function - see header */
void send_login_not_ok(const client_node_type &client) {

#if DEBUG_SEND==1
    printf("LOG_IN_NOT_OK connection [%i]\n", client.id());
#endif

    log_event(EVENT_SESSION, "LOG_IN_NOT_OK connection [%i]", client.id());

    send_packet(client, LogInNotOk());
}


/** public function - see header */
void send_you_dont_exist(client_node_type &client) {

#if DEBUG_SEND==1
    printf("YOU_DONT_EXIST connection [%i]\n", client.id());
#endif

    log_event(EVENT_SESSION, "YOU_DONT_EXIST connection [%i]", client.id());

    send_packet(client, YouDontExist());
}


/** public function - see header */
void send_you_are(client_node_type &client) {

#if DEBUG_SEND==1
    printf("YOU_ARE connection [%i]\n", client.id());
#endif

    log_event(EVENT_SESSION, "YOU_ARE connection [%i]", client.id());

    send_packet(client, YouAre(client.id()));
}


/** public function - see header */
void send_create_char_ok(client_node_type &client) {

#if DEBUG_SEND==1
    printf("CREATE_CHAR_OK connection [%i]\n", client.socket_fd);
#endif

    log_event(EVENT_SESSION, "CREATE_CHAR_OK connection [%i]", client.socket_fd);

    send_packet(client, CreateCharOk());
}

/** public function - see header */
void send_create_char_not_ok(client_node_type &client) {

#if DEBUG_SEND==1
    printf("CREATE_CHAR_OK connection [%i]\n", client.socket_fd);
#endif

    log_event(EVENT_SESSION, "CREATE_CHAR_NOT_OK connection [%i]", client.socket_fd);

    send_packet(client, CreateCharNotOk());
}

/** public function - see header */
void send_raw_text(const client_node_type &client, int channel, const char *_text){

#if DEBUG_SEND==1
    printf("RAW_TEXT connection [%i] channel [%i] text [%s]\n", connection, channel, _text);
#endif

    log_event(EVENT_SESSION, "RAW_TEXT connection [%i] channel [%i] text [%s]", client.id(), channel, _text);

    send_packet(client, ServerRawText(channel,_text));
}

void send_here_your_inventory(client_node_type & client){

    /** public function - see header */
    //TODO: remove the memory copying below
    std::vector<client_inventory_type> items(client.client_inventory,
                                             client.client_inventory+MAX_INVENTORY_SLOTS);
    send_packet(client, CharacterInventory(items));
}


/** public function - see header */
void send_get_active_channels(client_node_type & client){

#if DEBUG_SEND==1
    printf("GET_ACTIVE_CHANNELS connection [%i] active_channel slot [%i]\n", client.id(), client.active_chan);

    for(i=0; i<MAX_CHAN_SLOTS; i++){

        printf("slot [%i] channel [%i]\n", i, clients.client[connection].chan[i]);
    }
#endif

    log_event(EVENT_SESSION, "GET_ACTIVE_CHANNELS connection [%i] active channel slot [%i]", client.id(), client.active_chan);

    for(int i=0; i<client.chan.size(); i++){

        log_text(EVENT_SESSION, "slot [%i] channel [%i]", i, client.chan[i]);
    }

    send_packet(client, GetActiveChannels(client.active_chan,client.chan));
}

void send_here_your_stats(client_node_type &tgt){

    /** public function - see header */
    character_attribute<uint16_t> load_attribute;
    load_attribute.current = tgt.inventory_emu();
    load_attribute.maximum = tgt.max_inventory_emu();
    send_packet(tgt, CharacterAttributesPacket(tgt.attribute_data, load_attribute));
}


void send_change_map(client_node_type &tgt, char *elm_filename){
    /** public function - see header */

#if DEBUG_SEND==1
    printf("CHANGE MAP connection [%i] map [%s]\n", connection, elm_filename);
#endif

    log_event(EVENT_SESSION, "CHANGE_MAP connection [%i] map [%s]", tgt.id(), elm_filename);
    send_packet(tgt, ChangeMap(elm_filename));
}
