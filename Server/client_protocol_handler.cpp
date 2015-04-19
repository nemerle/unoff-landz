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
#include "client_protocol_handler.h"

#include "clients.h"
#include "logging.h"
#include "string_functions.h"
#include "global.h"
#include "colour.h"
#include "server_messaging.h"
#include "server_protocol_functions.h"
#include "characters.h"
#include "character_race.h"
#include "game_data.h"
#include "log_in.h"
#include "maps.h"
#include "character_movement.h"
#include "character_type.h"
#include "broadcast_actor_functions.h"
#include "chat.h"
#include "hash_commands.h"
#include "server_start_stop.h"
#include "idle_buffer.h"
#include "season.h"

#include "db/db_character_tbl.h"
#include "db/database_functions.h"

#include "Common/ClientPackets.h"
#include "Common/ProcessPacket.h"

#include <stdio.h>  //support for sprintf
#include <string.h> //support for memcpy strlen strcpy
#include <algorithm> // for std::min

#define DEBUG_PACKET 0//set debug mode
namespace {
struct UnoffPacketHandler : public ClientPacketHandler {
    client_node_type &source_client;

    UnoffPacketHandler(client_node_type &client) : source_client(client) {}

    void handleError(const char *msg) override {
        log_event(EVENT_ERROR, "Protocol Error [%s]", msg);
    }

    void handle(const PMMessage &pkt) override
    {
        //log the event here as the send_pm function adds a log entry if the target char is not found
        log_event(EVENT_CHAT, "send pm from [%s] to [%s] %s", source_client.char_name.c_str(),
                  pkt.targetName().c_str(),
                  pkt.messageContents().c_str());

        //send the message
        send_pm(source_client, pkt.targetName().c_str(), pkt.messageContents().c_str());
    }

    void handle(const ClientRawText &src) override {
        char text_out[1024]="";
        char text[1024]="";
        memcpy(text, src.contents().c_str(), std::min(sizeof(text),src.contents().size()));

#if DEBUG_PACKET==1
        printf("RAW_TEXT [%s]\n", text);
#endif

        // trim off excess left hand space
        str_trim_left(text);

        //channel chat
        if(text[0]=='@'){

            //remove the @ from text string and add null terminator
            memcpy(text, text+1, strlen(text)-1);
            text[strlen(text)-1]='\0';

            // check if char has an active channel
            if(source_client.active_chan==0){

                sprintf(text_out, "%cyou have not joined a channel yet", c_red3+127);
                send_raw_text(source_client, CHAT_SERVER, text_out);
                return;
            }

            //chat channels run from 32 (channel 1) to 63 (channel 32)
            int active_chan_slot=source_client.active_chan - CHAT_CHANNEL0;

            //channel slots run from zero. Hence, we need to subtract 1 from the active_chan slot value
            int chan=source_client.chan[active_chan_slot-1];

#if DEBUG_PACKET==1
            printf("active chan slot %i  chan %i\n", active_chan_slot-1, chan);
#endif

            //broadcast to self
            sprintf(text_out, "%c[%s]: %s", c_grey1+127, source_client.char_name.c_str(),text);
            send_raw_text(source_client, CHAT_CHANNEL0 + active_chan_slot, text_out);

            //broadcast to others
            broadcast_channel_chat(chan, source_client, text);

            log_event(EVENT_CHAT, "broadcast channel [%s @% i]: %s", source_client.char_name.c_str(), chan, text);

#if DEBUG_PACKET==1
            printf("channel chat by [%s] on chan [%i] %s\n", source_client.char_name, chan, text);
#endif
        }

        //hash commands
        else if(text[0]=='#'){

#if DEBUG_PACKET==1
            printf("#command by [%s] %s\n", source_client.char_name, text);
#endif

            process_hash_commands(source_client, text);

            log_event(EVENT_SESSION, "#command %s [%s]", source_client.char_name.c_str(), text);

            return;
        }

        //local chat
        else {

            char map_name[80]="";
            strcpy(map_name, maps.map[source_client.map_id].map_name);

            sprintf(text_out, "%c%s: %s", c_grey1+127,  source_client.char_name.c_str(), text);

            //broadcast to self
            send_raw_text(source_client, CHAT_LOCAL, text_out);

            //broadcast to others
            broadcast_local_chat(source_client, text_out);

            log_event(EVENT_CHAT, "broadcast local [%s] %s: %s", map_name,  source_client.char_name.c_str(), text);

#if DEBUG_PACKET==1
            printf("local chat on map [%s] %s: %s\n", map_name, source_client.char_name, text);
#endif
        }

    }
    void handle(const MoveTo &src) override
    {
#if DEBUG_PACKET==1
        printf("MOVE_TO %i %s\n", connection, source_client.char_name);
#endif

        //returns 2x 2byte integers indicating the x/y axis of the destination

        int x_dest=src.m_dest_x;
        int y_dest=src.m_dest_y;
        int tile_dest=x_dest+(y_dest*maps.map[source_client.map_id].map_axis);

#if DEBUG_PACKET==1
        printf("position x[%i] y[%i] tile[%i]\n", x_dest, y_dest, tile_dest);
#endif

        start_char_move(source_client, tile_dest);

        log_event(EVENT_SESSION, "Protocol MOVE_TO by [%s]...",  source_client.char_name.c_str());
        log_text(EVENT_SESSION, "Map [%s] x[%i] y[%i]", maps.map[source_client.map_id].map_name, x_dest, y_dest);
    }

    void handle(const SitDown &sitdown) override
    {
        char sql[MAX_SQL_LEN]="";

#if DEBUG_PACKET==1
        printf("SIT_DOWN %i\n", data[0]);
#endif

        // the protocol recognises two sets of sit stand command. The first is implemented via the actor command set
        //which is used in the SC_ADD_ACTOR packet; the second is implemented via the frame set which is used in the
        //ADD_ENHANCED_ACTOR packet. When using the SC_ADD_ACTOR packet, command 13=sit down and command 14=stand up. When
        //using the ADD_ENHANCED_ACTOR packet, command 12=sit, command 13=stand and command 14=stand idle.

        switch(sitdown.m_sit){

        case 0://stand

#if DEBUG_PACKET==1
            printf("Stand\n");
#endif

            source_client.actor_data.frame=frame_stand;

            broadcast_actor_packet(source_client, actor_cmd_stand_up, source_client.map_tile);

            //update database here else, if we do it after the switch structure, an unknown frame value
            //could end up being updated to the database
            snprintf(sql, MAX_SQL_LEN, "UPDATE CHARACTER_TABLE SET FRAME=%i WHERE CHAR_ID=%i;",
                     source_client.actor_data.frame, source_client.character_id);
            db_push_buffer(sql);

            log_event(EVENT_SESSION, "Protocol SIT_DOWN by [%s] (stand)",  source_client.char_name.c_str());
            break;

        case 1://sit

#if DEBUG_PACKET==1
            printf("Sit\n");
#endif

            source_client.actor_data.frame=frame_sit;

            broadcast_actor_packet(source_client, actor_cmd_sit_down, source_client.map_tile);

            //update database here else, if we do it after the switch structure, an unknown frame value
            //could end up being updated to the database
            sprintf(sql, "UPDATE CHARACTER_TABLE SET FRAME=%i WHERE CHAR_ID=%i;",source_client.actor_data.frame, source_client.character_id);
            db_push_buffer(sql);

            log_event(EVENT_SESSION, "Protocol SIT_DOWN by [%s] (sit)",  source_client.char_name.c_str());
            break;

        default:

            log_event(EVENT_ERROR, "Protocol SIT_DOWN by [%s] unknown frame [%i])",
                      source_client.char_name.c_str(), source_client.actor_data.frame);
            stop_server();
            break;
        }
    }
    void handle(const GetPlayerInfo &cmd) override {
        auto iter = clients.find(cmd.m_id);
        client_node_type * n = (iter==clients.end()) ? nullptr : iter->second;
        if(n==nullptr) {
            log_event(EVENT_ERROR, "Protocol GET_PLAYER_INFO by [%s] bad ID (%d)",
                      source_client.char_name.c_str(), cmd.m_id);
            return;
        }
#if DEBUG_PACKET==1
        printf("GET_PLAYER_INFO [%i] [%s]\n", cmd.m_id, n->char_name);
#endif

        send_raw_text(source_client, CHAT_SERVER, (std::string("You see ")+n->char_name).c_str());

        log_event(EVENT_SESSION, "Protocol GET_PLAYER_INFO by [%s] (%s)",
                  source_client.char_name.c_str(),
                  n->char_name.c_str());

    }
    void handle(const SendMeMyActors &pkt) override {
        log_event(EVENT_ERROR, "Protocol SEND_ME_MY_ACTORS by [%s]",  source_client.char_name.c_str());
    }

    void handle(const SendOpeningScreen &pkt) override {
        log_event(EVENT_SESSION, "Protocol SEND_OPENING_SCREEN by [%i]", source_client.id());
    }
    void handle(const SendVersion &pkt) override {
        log_event(EVENT_SESSION, "Protocol SEND_VERSION by [%i]...", source_client.id());
        log_text(EVENT_SESSION, "first digit [%i] second digit [%i]", pkt.version_a, pkt.version_b);
        log_text(EVENT_SESSION, "major [%i] minor [%i] release [%i] patch [%i]", pkt.major_ver, pkt.minor_ver, pkt.release_ver, pkt.patch_ver);
        log_text(EVENT_SESSION, "host [%i.%i.%i.%i] port [%i]", pkt.host_a, pkt.host_b, pkt.host_c, pkt.host_d, pkt.port);

    }
    void handle(const Heartbeat &/*pkt*/) override {
        //no need to do anything on this message as any data receipt updates the heartbeat
    }
    void handle(const UseObject &pkt) override {
        int map_object_id=pkt.object_id;
        int use_with_position=pkt.object_inv_pos;
        /*
        //returns a 4byte integer indicating the threed object id, followed by a 4byte integer indicating ????

        #if DEBUG_PACKET==1
        printf("USE_OBJECT - map object [%i] position [%i]\n", map_object_id, use_with_position);
        #endif

        //if char is moving when protocol arrives, cancel rest of path
        source_client.path_count=0;

        //travel from IP to Ravens Isle
        if(map_object_id==520 && source_client.map_id==1) move_char_between_maps(connection, 2, 64946);

        //travel from Ravens Isle to IP
        if(map_object_id==5416 && source_client.map_id==2) move_char_between_maps(connection, 1, 4053);

        //travel from Ravens Isle to neiva
        if(map_object_id==4986 && source_client.map_id==2 && source_client.map_tile==108627){
            move_char_between_maps(connection, 3, 3000);
        }
        */
        log_event(EVENT_SESSION, "Protocol USE_OBJECT by [%s]...", source_client.char_name.c_str());

    }
    void handle(const LookAtInventoryItem &/*pkt*/) override {
        /*
        //returns a Uint8 giving the slot number looked at

        inventory_slot=(int)data[0];
        image_id=source_client.client_inventory[inventory_slot].image_id;

        #if DEBUG_PACKET==1
        printf("LOOK_AT_INVENTORY_ITEM - slot [%i]\n", inventory_slot);
        #endif

        sprintf(text_out, "%c%s", c_green3+127, item[image_id].item_name);
        send_server_text(connection, CHAT_SERVER, text_out);

        log_event(EVENT_SESSION, "Protocol LOOK_AT_INVENTORY_ITEM by [%s]...", source_client.char_name);
*/
    }
    void handle(const MoveInventoryItem &/*pkt*/) override {
        /*
        //returns 2 Uint8 indicating the slots to be moved from and to
        //if an attempt is made to move to an occupied slot or, to move from an empty slot, the client will automatically block

        move_from_slot=(int)data[0];
        move_to_slot=(int)data[1];

        #if DEBUG_PACKET==1
        printf("MOVE_INVENTORY_ITEM - slot [%i] to slot [%i]\n", move_from_slot, move_to_slot);
        #endif

        image_id=source_client.client_inventory[move_from_slot].image_id;
        amount=source_client.client_inventory[move_from_slot].amount;

        //zero the 'from slot'
        source_client.client_inventory[move_from_slot].image_id=0;
        source_client.client_inventory[move_from_slot].amount=0;
        send_get_new_inventory_item(connection, 0, 0, move_from_slot);

        //save to database
        update_db_char_slot(connection, move_from_slot);

        //place item in the 'to slot'
        source_client.client_inventory[move_to_slot].image_id=image_id;
        source_client.client_inventory[move_to_slot].amount=amount;
        send_get_new_inventory_item(connection, image_id, amount, move_to_slot);

        //save to the database
        update_db_char_slot(connection, move_to_slot);

        log_event(EVENT_SESSION, "Protocol MOVE_INVENTORY_ITEM by [%s]...", source_client.char_name);
*/
    }
    void handle(const Harvest &pkt) override {
#if DEBUG_PACKET==1
        printf("HARVEST %i %i \n", packet[1], packet[2]);
#endif

        //pkt.m_map_object_id - integer corresponding to the id of an object in the map 3d object list
        /*
            start_harvesting2(connection, map_object_id, loop);
        */
        log_event(EVENT_SESSION, "Protocol HARVEST by [%s] on %d...",
                  source_client.char_name.c_str(),
                  pkt.m_map_object_id
                  );
    }
    void handle(const DropItem &pkt) override {
        int image_id = source_client.client_inventory[pkt.m_inventory_slot].image_id;
        //drop_from_inventory(source_client,pkt.m_inventory_slot,pkt.m_amount)
        log_event(EVENT_SESSION, "Protocol UNHANDLED DROP_ITEM by [%s]...", source_client.char_name.c_str());
    }
    void handle(const PickUpItem &pkt) override {
        int amoutn = pkt.m_amount;
        /*
        bag_slot=data[0];
        //pick_up_from_bag(connection, bag_slot, loop);
        */
        log_event(EVENT_SESSION, "Protocol UNHANDLED PICK_UP_ITEM by [%s]...", source_client.char_name.c_str());
    }
    void handle(const InspectBag &pkt) override {

        /*
        //returns a Unit8 indicating the bag_id

        bag_id=data[0];

        #if DEBUG_PACKET==1
        printf("INSPECT_BAG - lsb [%i] msb [%i] bag id [%i]\n", lsb, msb, bag_id);
        #endif

        //check we are standing on the bag
        if(bag_list[bag_id].tile_pos==current_tile) {

            //if we are standing on the bag then attempt to open it
           //send_here_your_ground_items(connection, bag_id);
            source_client.bag_open=TRUE;
        }
        else {

            //not standing on bag so move towards it
            start_char_move(connection, bag_list[bag_id].tile_pos, loop);
        }

        log_event(EVENT_SESSION, "Protocol INSPECT_BAG by [%s]...", source_client.char_name);
        */
    }
    void handle(const LookAtMapObject &pkt) override {
        //map_object = source_client.current_map.getMapObject(pkt.m_map_object_id);
        /*
        //tell the client what the map object is
        if(map_object.image_id>0){
            sprintf(text_out, "%c%s", c_green3+127, item[map_object.image_id].item_name);
        }
        else {
            sprintf(text_out, "%cUnknown item", c_green3+127);
        }

        send_server_text(connection, CHAT_SERVER, text_out);

        */
        log_event(EVENT_SESSION, "Protocol LOOK_AT_MAP_OBJECT by [%s]...", source_client.char_name.c_str());
    }
    void handle(const AttackSomeone &pkt) override {
        //map_object = source_client.current_map.getMapObject(pkt.m_map_object_id);
        log_event(EVENT_SESSION, "Protocol ATTACK_SOMEONE by [%s]...", source_client.char_name.c_str());
    }
    void handle(const PingResponse &/*pkt*/) override {
        log_event(EVENT_SESSION, "Protocol PING_RESPONSE by [%s]...",  source_client.char_name.c_str());
    }
    void handle(const SetActiveChannel &pkt) override {

        //set the active channel
        source_client.active_chan=pkt.m_channel;

        //update the database
        char sql[MAX_SQL_LEN]="";
        snprintf(sql, MAX_SQL_LEN, "UPDATE CHARACTER_TABLE SET ACTIVE_CHAN=%i WHERE CHAR_ID=%i", pkt.m_channel,
                 source_client.character_id);
        db_push_buffer(sql);

        log_event(EVENT_SESSION, "Protocol SET_ACTIVE_CHANNEL by [%s]...",  source_client.char_name.c_str());
    }
    void handle(const LogIn &pkt) override {
        //place log event before process so the log entries from the process_log_in function follow
        //in a logical order
        log_event(EVENT_SESSION, "Protocol LOG_IN by [%i]...", source_client.id());

        //process_log_in(connection, packet);
        db_push_buffer(&source_client, IDLE_BUFFER_PROCESS_LOGIN, new LogIn(pkt));
    }
    void handle(const CreateChar &pkt) override {
        //place log event before process so the following are in a logical order
        log_event(EVENT_SESSION, "Protocol CREATE_CHAR by [%i]...", source_client.id());

        db_push_buffer(&source_client, IDLE_BUFFER_PROCESS_CHECK_NEWCHAR, new CreateChar(pkt));
    }
    void handle(const GetDate &/*pkt*/) override {
        send_verbose_date(source_client, game_data.game_days % game_data.year_length);

        log_event(EVENT_SESSION, "Protocol GET_DATE by [%s]...",  source_client.char_name.c_str());
    }
    void handle(const GetTime &/*pkt*/) override {
        char buf[256];
        snprintf(buf,256,"Time %02d:%02d",game_data.game_minutes / 60,game_data.game_minutes % 60);
        send_raw_text(source_client, CHAT_SERVER, buf );

        log_event(EVENT_SESSION, "Protocol GET_TIME by [%s]...",  source_client.char_name.c_str());

    }
    void handle(const ServerStats &/*pkt*/) override {

        send_motd_header(source_client);

        log_event(EVENT_SESSION, "Protocol SERVER_STATS by [%s]...",  source_client.char_name.c_str());
    }
};

} // end of anonymous namespace

void process_packet(client_node_type &source_client, uint8_t *packet,int length){

    /** public function - see header */

    char text_out[1024]="";
    int data_length=packet[1]+(packet[2]*256)-1;

    //packet logging
    for(int i=0; i<data_length+2; i++){

        snprintf(text_out, 1024,"%s %i", text_out, packet[i]);
    }

    log_event(EVENT_PACKET,"Receive from [%i]%s", source_client.id(), text_out);

    UnoffPacketHandler client_handler(source_client);
    processClientPacket(packet,length,&client_handler);
}
