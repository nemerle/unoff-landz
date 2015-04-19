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

#include "character_movement.h"

#include "logging.h"
#include "clients.h"
#include "maps.h"
#include "date_time_functions.h"
#include "server_protocol_functions.h"
#include "broadcast_actor_functions.h"
#include "global.h"
#include "colour.h"
#include "server_messaging.h"
#include "characters.h"
#include "movement.h"
#include "pathfinding.h"
#include "server_start_stop.h"
#include "game_data.h"
#include "idle_buffer.h"

#include "Common/ServerPackets.h"
#include "db/db_character_tbl.h"
#include "db/database_functions.h"

#include <stdlib.h>     //supports exit function
#include <stdio.h>      //support sprintf function
#include <sys/socket.h> //support for send function
#define DEBUG_MOVEMENT 0

std::pair<int, int> client_node_type::position() const
{
    int map_axis=maps.map[map_id].map_axis;
    return {map_tile % map_axis,map_tile / map_axis};
}

int get_move_command(int tile_pos, int tile_dest, int map_axis){

    /** RESULT  : calculates the move command based on the current and destination tile

        RETURNS : move command

        PURPOSE :

        NOTES   :
    */

    int i=0;
    int move=tile_dest-tile_pos;

    if(move==map_axis) i=0;

    else if(move==map_axis+1) i=1;

    else if(move==map_axis-1) i=7;

    else if(move==(map_axis*-1)) i=4;

    else if(move==(map_axis*-1)+1) i=3;

    else if(move==(map_axis*-1)-1) i=5;

    else if(move==1) i=2;

    else if(move==-1) i=6;

    else {
        log_event(EVENT_MOVE_ERROR, "illegal move in function %s: module %s: line %i", __func__, __FILE__, __LINE__);
        log_text(EVENT_MOVE_ERROR, "current tile [%i] destination tile [%i] move distance [%i]", tile_pos, tile_dest, move);
        stop_server();
    }

    return vector[i].move_cmd;
}


void process_char_move(client_node_type &client, time_t current_utime){

    /** public function - see header **/

    int map_id=client.map_id;
    int map_axis=maps.map[map_id].map_axis;
    int current_tile=client.map_tile;
    int next_tile=0;
    int move_cmd=0;

    // move actor one step along the path
    if(!client.path.empty()){

        //adjust timer to compensate for wrap-around>
        if(client.time_of_last_move>current_utime) current_utime+=1000000;

        // check for time of next movement
        if(current_utime>client.time_of_last_move+290000) {

            //get destination tile from the path queue
            next_tile=client.path.back();

            client.path.pop_back();

            // filter out moves where position and destination are the same
            if(current_tile!=next_tile){

                #if DEBUG_MOVEMENT==1
                printf("move char [%s] from tile [%i] to tile [%i]\n", client.char_name, current_tile, next_tile);
                #endif

                //update the time of move
                gettimeofday(&time_check, nullptr);
                client.time_of_last_move=time_check.tv_usec;

                //calculate the move_cmd and broadcast to clients
                move_cmd=get_move_command(current_tile, next_tile, map_axis);
                client.actor_data.x = client.position().first;
                client.actor_data.y = client.position().second;

                broadcast_actor_packet(client, move_cmd, next_tile);

                //update char current position and save
                client.map_tile=next_tile;

                //update_db_char_position(connection);
                char sql[MAX_SQL_LEN]="";
                snprintf(sql, MAX_SQL_LEN, "UPDATE CHARACTER_TABLE SET MAP_TILE=%i, MAP_ID=%i WHERE CHAR_ID=%i",next_tile, map_id, client.character_id);
                db_push_buffer(sql);

            }
        }
    }
}


int remove_char_from_map(client_node_type &client){

    /** public function - see header **/

    int map_id=client.map_id;

    //check for illegal map
    if(map_id>MAX_MAPS || map_id<1){

        log_event(EVENT_MOVE_ERROR, "attempt to remove char from illegal map (id[%i] map name [%s]) in function %s: module %s: line %i", map_id, maps.map[map_id].map_name, __func__, __FILE__, __LINE__);
        return REMOVE_MAP_ILLEGAL;
    }

    //broadcast actor removal to other chars on map
    broadcast_remove_actor_packet(client);
    log_event(EVENT_SESSION, "char %s removed from map %s", client.char_name.c_str(), maps.map[map_id].map_name);

    return REMOVE_MAP_SUCESS;
}


/** RESULT  : make this actor and other actors visible to this client
    RETURNS : void
    PURPOSE : used by add_char_to_map function
*/
void send_actors_to_client(const client_node_type &target_client){


    int map_id=target_client.map_id;
    int map_tile=target_client.map_tile;
    int char_visual_range=target_client.get_char_visual_range();

    for(const auto &v : clients) {
        client_node_type &client(*v.second);

        //restrict to characters on the same map
        if(map_id==client.map_id){

            //restrict to characters within visual range of this character
            if(get_proximity(map_tile, client.map_tile, maps.map[map_id].map_axis)<=char_visual_range){
                //send actors within visual proximity to this char
                client.actor_data.x = client.position().first;
                client.actor_data.y = client.position().second;

                send_packet(target_client, AddNewEnhancedActor(client.id(),client.actor_data));
            }
        }
    }
}


int add_char_to_map(client_node_type &new_client, int map_id, int map_tile){

    /** public function - see header **/

    //check for illegal maps
    if(map_id>MAX_MAPS || map_id<0) {

        log_event(EVENT_MOVE_ERROR, "illegal map (id[%i] name [%s]) in function %s: module %s: line %i", map_id, maps.map[map_id].map_name, __func__, __FILE__, __LINE__);
        return ADD_MAP_ILLEGAL;
    }

    //get nearest unoccupied tile to the clients position
    int unoccupied_tile=get_nearest_unoccupied_tile(map_id, map_tile);

    if(unoccupied_tile==0){

        log_event(EVENT_MOVE_ERROR, "Unable to find unoccupied tile within [%i] tiles on map (id[%i] name [%s]) in function %s: module %s: line %i", MAX_UNOCCUPIED_TILE_SEARCH, map_id, maps.map[map_id].map_name, __func__, __FILE__, __LINE__);
        return ADD_MAP_UNREACHABLE;
    }

    new_client.map_tile=unoccupied_tile;

    //send map to client
    send_change_map(new_client, maps.map[map_id].elm_filename);

    //make scene visible to this client
    send_actors_to_client(new_client);

    //send existing bags on map to client
    //send_bags_to_client(connection);

    new_client.actor_data.x = new_client.position().first;
    new_client.actor_data.y = new_client.position().second;

    //add this char to other connected clients on this map
    broadcast_add_new_enhanced_actor_packet(new_client);

    log_event(EVENT_SESSION, "char [%s] added to map [%s] at tile [%i]",
               new_client.char_name.c_str(), maps.map[map_id].map_name, new_client.map_tile);

    return ADD_MAP_SUCESS;
}


void move_char_between_maps(client_node_type &source_client, int new_map_id, int new_map_tile){

    /** public function - see header */

    //check to see if old map is legal and, if not, transport char to Isla Prima
    if(remove_char_from_map(source_client)==ILLEGAL_MAP) {

        new_map_id=game_data.beam_map_id;
        new_map_tile=game_data.beam_map_tile;
    }

    //check to see if new map is legal and, if not, transport char tp Isla Prima
    if(add_char_to_map(source_client, new_map_id, new_map_tile)!=ADD_MAP_SUCESS){

        new_map_id=game_data.beam_map_id;
        new_map_tile=game_data.beam_map_tile;
    }

    //save char map id and position
    char sql[MAX_SQL_LEN]="";
    snprintf(sql, MAX_SQL_LEN, "UPDATE CHARACTER_TABLE SET MAP_TILE=%i, MAP_ID=%i WHERE CHAR_ID=%i", new_map_tile, new_map_id, source_client.character_id);
    db_push_buffer(sql);
}


void start_char_move(client_node_type &source_client, int destination){

    /** public function - see header */

    char text_out[1024]="";

    int map_id=source_client.map_id;
    int current_tile=source_client.map_tile;

/*
    //if char is harvesting then stop
    if(source_client.harvest_flag==TRUE){

        stop_harvesting2(connection, loop);
        return;
    }
*/

    //if char is sitting then stand before moving
    if(source_client.actor_data.frame==frame_sit){

        source_client.actor_data.frame=frame_stand;
        broadcast_actor_packet(source_client, actor_cmd_stand_up, source_client.map_tile);

        char sql[MAX_SQL_LEN]="";
        snprintf(sql, MAX_SQL_LEN, "UPDATE CHARACTER_TABLE SET FRAME=%i WHERE CHAR_ID=%i",
                 source_client.actor_data.frame, source_client.character_id);
        db_push_buffer(sql);
    }

    //check if the destination is walkable
    if(maps.map[map_id].height_map[destination]<MIN_TRAVERSABLE_VALUE){

        sprintf(text_out, "%cThe tile you clicked on can't be walked on", c_red3+127);
        send_raw_text(source_client, CHAT_SERVER, text_out);

        return;
    }

    //check for zero length path
    if(current_tile==destination){

        #if DEBUG_MOVEMENT==1
        printf("current tile = destination (ignored)\n");
        #endif

        return;
    }

    //get path
    if(get_astar_path(source_client, current_tile, destination)==NOT_FOUND){

        log_event(EVENT_ERROR, "path not found in function %s: module %s: line %i", __func__, __FILE__, __LINE__);
    }

/*
    //if standing on a bag, close the bag grid
    if(source_client.bag_open==TRUE){

        source_client.bag_open=FALSE;
        send_s_close_bag(connection);
    }
*/

    #if DEBUG_MOVEMENT==1
    printf("character [%s] got a new path...\n", source_client.char_name);

    int i=0;
    for(i=0; i<source_client.path_count; i++){
        printf("%i %i\n", i, source_client.path[i]);
    }
    #endif

    //reset time of last move to zero so the movement is processed without delay
    source_client.time_of_last_move=0;
}

