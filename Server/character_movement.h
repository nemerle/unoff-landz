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

#ifndef CHARACTER_MOVEMENT_H_INCLUDED
#define CHARACTER_MOVEMENT_H_INCLUDED

#include <ev.h>         // evlib event library

#define PATH_MAX 100    // maximum tiles of precalculated character movement

struct client_node_type;
enum{//return values add_char_to_map
    ADD_MAP_ILLEGAL,
    ADD_MAP_UNREACHABLE,
    ADD_MAP_SUCESS
};


enum{//return values remove_char_from_map
    REMOVE_MAP_ILLEGAL,
    REMOVE_MAP_SUCESS
};


/** RESULT  : Removes actor from map

    RETURNS : REMOVE_MAP_ILLEGAL - map doesn't exist
              REMOVE_MAP_SUCCESS - character was removed from map

    PURPOSE : used at log-out to remove a char from map. Also on map jumps
**/
int remove_char_from_map(client_node_type &client);


/** RESULT  : adds a character to a map

    RETURNS : ADD_MAP_ILLEGAL - map doesn't exist
              ADD_MAP_FAILED  - no unoccupied tiles on map
              ADD_MAP_SUCCESS - character was added to map

    PURPOSE : used at log-in to add a char to a map. Also by function move_char_between_maps
**/
int add_char_to_map(client_node_type &new_client, int new_map_id, int map_tile);


/** RESULT  : moves a character between maps

    RETURNS : void

    PURPOSE : supports map jumps
**/
void move_char_between_maps(client_node_type &source_client, int new_map_id, int new_map_tile);


/** RESULT  : moves a character one step along the path

    RETURNS : void

    PURPOSE : makes the character move
**/
void process_char_move(client_node_type &client, time_t current_utime);


/** RESULT  : starts the character moving

    RETURNS : void

    PURPOSE : makes the character move
**/
//void start_char_move(int connection, int destination, struct ev_loop *loop);
void start_char_move(client_node_type &source_client, int destination);

#endif // CHARACTER_MOVEMENT_H_INCLUDED
