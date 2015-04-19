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
#include "broadcast_actor_functions.h"

#include "clients.h"
#include "maps.h"
#include "server_protocol_functions.h"
#include "character_race.h"
#include "characters.h"
#include "server_messaging.h"
#include "colour.h"
#include "chat.h"
#include "global.h"
#include "logging.h"
#include "Common/ServerPackets.h"
#include "Common/PacketToBuffer.h"

#include <stdio.h> //support for printf
#define DEBUG_BROADCAST 0

void broadcast_add_new_enhanced_actor_packet(const client_node_type &created_actor){

    /** public function - see header */

    int map_id    = created_actor.map_id;
    int char_tile = created_actor.map_tile;
    int map_axis  = maps.map[map_id].map_axis;

    //pre-create the add_new_enhanced_actor packet so we don't have to repeat this on each occasion when
    //it needs to sent to other actors
    AddNewEnhancedActor packet(created_actor.id(),created_actor.actor_data);

    //cycle through all the clients
    for(const auto &v : clients){
        const client_node_type &client = *v.second;
        //restrict to clients that are logged in
        if(client.client_status!=client_node_type::LOGGED_IN)
            continue;
        //exclude the broadcasting char
        if(client.id()==created_actor.id())
            continue;
        //restrict to chars on the same map as broadcasting char
        if(map_id!=client.map_id)
            continue;

        //select this char and those characters in visual range of this char
        int receiver_char_visual_range = client.get_char_visual_range();
        int receiver_char_tile         = client.map_tile;

        //restrict to those chars that can see the broadcasting char
        if(get_proximity(char_tile, receiver_char_tile, map_axis) < receiver_char_visual_range){

            //send(i, packet, packet_length, 0);
            send_packet(client, packet);
        }
    }
}

void broadcast_remove_actor_packet(const client_node_type &removed_client) {

    /** public function - see header */

    int map_id=removed_client.map_id;
    int char_tile=removed_client.map_tile;
    int map_axis=maps.map[map_id].map_axis;

    //pre-create the remove char packet so we don't have to repeat this on each occasion when it needs to
    //sent to other actors
    std::vector<uint8_t> content;
    RemoveActor packet(removed_client.id());
    packetToBuffer(content,packet);



    //cycle through all the clients
    for(const auto &v : clients){
        const client_node_type &client = *v.second;
        //restrict to clients that are logged in
        if(client.client_status!=client_node_type::LOGGED_IN)
            continue;
        //exclude the broadcasting char
        if(client.id()==removed_client.id())
            continue;
        //restrict to chars on the same map as broadcasting char
        if(map_id!=client.map_id)
            continue;
        int receiver_char_visual_range  = client.get_char_visual_range();
        int receiver_char_tile          = client.map_tile;

#if DEBUG_BROADCAST==1
        printf("remove actor packet connection %i proximity %i visual range %i\n", i, receiver_char_visual_range, get_proximity(char_tile, receiver_char_tile, map_axis));
#endif

        //restrict to those chars that can see the broadcasting char
        if(get_proximity(char_tile, receiver_char_tile, map_axis) < receiver_char_visual_range){

#if DEBUG_BROADCAST==1
            printf("connection %i remove actor packet sent\n", i);
#endif

            send_packet(client, content);
        }
    }
}

void broadcast_actor_packet(const client_node_type &source_actor, uint8_t move, int sender_destination_tile){

    /** public function - see header */

    std::vector<uint8_t> packet1;// receiving char add_actor packet

    std::vector<uint8_t> packet2;// receiving char add_enhanced_actor packet

    std::vector<uint8_t> packet3;// receiving char remove_actor packet

    int sender_current_tile = source_actor.map_tile;
    int sender_visual_range = source_actor.get_char_visual_range();

    int map_id  =source_actor.map_id;
    int map_axis=maps.map[map_id].map_axis;

    int proximity_before_move = 0;
    int proximity_after_move  = 0;

    // pre-create packets that will be sent more than once in order to save time
    packetToBuffer(packet1,AddActor(source_actor.id(), move));
    packetToBuffer(packet2,AddNewEnhancedActor(source_actor.id(),source_actor.actor_data));
    packetToBuffer(packet3,RemoveActor(source_actor.id()));

    // broadcast sender char move to all receiver clients
    for(const auto &v : clients){
        const client_node_type &client(*v.second);
        if(client.client_status!=client_node_type::LOGGED_IN)
            continue;
        if(client.map_id!=source_actor.map_id)
            continue;

        int receiver_tile=client.map_tile;
        int receiver_visual_range=client.get_char_visual_range();

        proximity_before_move=get_proximity(sender_current_tile, receiver_tile, map_axis);
        proximity_after_move=get_proximity(sender_destination_tile, receiver_tile, map_axis);

        //This block deals with receiving char vision
        if(client.id()!=source_actor.id()){

            if(proximity_before_move>receiver_visual_range && proximity_after_move<=receiver_visual_range){

                //sending char moves into visual proximity of receiving char
#if DEBUG_BROADCAST==1
                printf("char [%i] sees sending char [%i] added\n", i, connection);
#endif

                //send(i, packet2, packet2_length, 0);
                send_packet(client, packet2);
            }
            else if(proximity_before_move<=receiver_visual_range && proximity_after_move>receiver_visual_range){

                //sending char moves out of visual proximity of receiving char
#if DEBUG_BROADCAST==1
                printf("char [%i] sees sending char [%i] removed\n", i, connection);
#endif

                //send(i, packet3, packet3_length, 0);
                send_packet(client, packet3);
            }
            else if(proximity_before_move<=receiver_visual_range && proximity_after_move<=receiver_visual_range){

                //sending char moving within visual proximity of receiving char
#if DEBUG_BROADCAST==1
                printf("char [%i] sees sending char [%i] move\n", i, connection);
#endif

                //send(i, packet1, packet1_length, 0);
                send_packet(client, packet1);
            }
            else {
#if DEBUG_BROADCAST==1
                printf("char [%i] can't see sending char [%i]\n", i, connection);
#endif
            }
        }

        //this block deals with sending char vision
        if(proximity_before_move>sender_visual_range && proximity_after_move<=sender_visual_range){

            //sending char moves into visual proximity of receiving char
#if DEBUG_BROADCAST==1
            printf("sending char [%i] sees another char [%i] added\n", connection, i);
#endif
            send_packet(source_actor, AddNewEnhancedActor(client.id(),client.actor_data));
        }
        else if(proximity_before_move<=sender_visual_range && proximity_after_move>sender_visual_range){

            //sending char moves out of visual proximity of receiving char
#if DEBUG_BROADCAST==1
            printf("sending char [%i] sees another char [%i] removed\n", connection, i);
#endif
            //send(connection, packet, packet_length, 0);
            send_packet(source_actor, RemoveActor(client.id()));
        }
        else if(proximity_before_move<=sender_visual_range && proximity_after_move<=sender_visual_range){

            //sending char moves within visual proximity of receiving char
            int char_move = 0; // char sees itself stationery
            if(client.id()==source_actor.id()) {
                char_move = move; // sending char sees itself move
            }
#if DEBUG_BROADCAST==1
            if(i==connection) {
                printf("sending char [%i] sees itself move\n", i);
            }
            else{
                printf("sending char [%i] sees itself stationary\n", i);

            }
#endif
            send_packet(source_actor, AddActor(client.id(),char_move));
        }
    }
}


void broadcast_local_chat(client_node_type &source_client, char *text_in){

    /** public function - see header */

    int map_id=source_client.map_id;
    int map_axis=maps.map[map_id].map_axis;

    for(const auto &v : clients) {
        const client_node_type &client(*v.second);
        if(client.client_status!=client_node_type::LOGGED_IN)
            continue; // don't broadcast to non-logged in clients

        if(map_id==client.map_id) {

            if(get_proximity(source_client.map_tile, client.map_tile, map_axis)<LOCAL_CHAT_RANGE){

                if(source_client.id()!=client.id())
                    send_raw_text(client, CHAT_LOCAL, text_in);
            }
        }
    }
}

void broadcast_channel_chat(int chan, client_node_type &source_client, char *text_in){

    /** public function - see header */

    char text_out[1024]="";

    //send to channel

    for(const auto &v : clients) {
        const client_node_type &client(*v.second);

        if(client.client_status!=client_node_type::LOGGED_IN)
            continue; // don't broadcast to non-logged in clients

        if(source_client.id()==client.id())
            continue; //don't echo to self

        //filter out players who are not in this chan
        if(is_player_in_chan(client,chan)!=NOT_FOUND){

            int active_chan_slot=client.active_chan;
            int active_chan=client.chan[active_chan_slot-1];

            //show non-active chan in darker grey
            char color;
            if(active_chan==chan){
                color = c_grey1;
            }
            else {
                color = c_grey2;
            }

            sprintf(text_out, "%c[%s]: %s", color+127,  source_client.char_name.c_str(), text_in);
            send_raw_text(client, CHAT_CHANNEL0 + active_chan_slot, text_out);
        }
    }
}

void broadcast_channel_event(int chan, client_node_type &source_client, char *text_in){

    /** public function - see header */

    for(const auto &v : clients) {
        const client_node_type &client(*v.second);


        if(client.client_status!=client_node_type::LOGGED_IN)
            continue; // don't broadcast to non-logged in clients

        if(source_client.id()==client.id())
            continue; //don't echo to self

        //filter out players who are not in this chan
        if(is_player_in_chan(client,chan)!=NOT_FOUND){

            int active_chan_slot=client.active_chan;

            send_raw_text(client, CHAT_CHANNEL0 + active_chan_slot, text_in);
        }
    }
}
