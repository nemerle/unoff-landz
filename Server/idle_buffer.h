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

#ifndef IDLE_BUFFER_H_INCLUDED
#define IDLE_BUFFER_H_INCLUDED

#include "db/database_functions.h" //access to MAX_SQL_LEN
#include <stdlib.h>
#include <vector>
#include <deque>
#include <string.h>
#include <string>
#include <stdint.h>


#define IDLE_BUFFER_MAX 100
#define MAX_PROTOCOL_PACKET_SIZE 160
struct client_node_type;
struct EL_Packet;
struct buffer_node_type{

    std::string sql;
    client_node_type *client;
    int process_type;
    void *data;
};
// double ended queue of buffer_node_type
typedef std::deque<buffer_node_type> buffer_list_type;

extern buffer_list_type idle_buffer;

enum{//database buffer processing types
    IDLE_BUFFER_PROCESS_SQL,
    IDLE_BUFFER_PROCESS_HASH_DETAILS,
    IDLE_BUFFER_PROCESS_CHECK_NEWCHAR,
    IDLE_BUFFER_PROCESS_ADD_NEWCHAR,
    IDLE_BUFFER_PROCESS_LOGIN,
};

/** RESULT  : pushes items on the database buffer

    RETURNS  : void

    PURPOSE  : allows for db actions can be processed during server idle events

    NOTES    :
**/
void db_push_buffer(const char *sql);
void db_push_buffer(client_node_type *client, int process_type, void *dat);

/** RESULT  : processes an item from the database buffer

    RETURNS  : void

    PURPOSE  : allows for db actions can be processed during server idle events

    NOTES    :
**/
void db_process_buffer();

#endif // IDLE_BUFFER_H_INCLUDED
