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

#include "idle_buffer.h"
#include "logging.h"
#include "server_start_stop.h"
#include "log_in.h"
#include "character_creation.h"
#include "db/database_functions.h"
#include "Common/ClientPackets.h"

#include <stdlib.h> //supports NULL
#include <stdio.h> //supports sprintf
#include <string.h> //supports strcpy
#include <cassert>


buffer_list_type idle_buffer;
void db_push_buffer(client_node_type *client, int process_type, void *dat) {
    /** public function - see header **/

    if(idle_buffer.size()>=IDLE_BUFFER_MAX)
    {

        //buffer overflow
        log_event(EVENT_ERROR, "database buffer overflow in function %s: module %s: line %i", __func__, __FILE__, __LINE__);
        stop_server();
        return;
    }

    buffer_node_type entry;

    entry.sql.clear();
    entry.client=client;
    entry.process_type=process_type;
    entry.data = dat;
    idle_buffer.push_back(entry);
}

void db_push_buffer(const char *sql) {
    int process_type = IDLE_BUFFER_PROCESS_SQL;
    /** public function - see header **/

    if(idle_buffer.size()>=IDLE_BUFFER_MAX)
    {

        //buffer overflow
        log_event(EVENT_ERROR, "database buffer overflow in function %s: module %s: line %i", __func__, __FILE__, __LINE__);
        stop_server();
        return;
    }

    buffer_node_type entry;

    entry.sql = sql;
    entry.process_type=process_type;

    idle_buffer.push_back(entry);
}


void db_process_buffer(){

    /** public function - see header **/

    //make sure we have something in the buffer to process
    if(idle_buffer.empty())
        return;

    const buffer_node_type &command(idle_buffer.front());

    //use else if structure rather than switch, as this allows us to encapsulate
    //variables within each if statement
    /**********************************************************************************************/

    if(command.process_type==IDLE_BUFFER_PROCESS_CHECK_NEWCHAR){

        //Checks whether a character name exists in the character_table of the database. If the
        //name exists, character creation is aborted and a message sent to the client. If the
        //name does not exist, the character creation packet is placed in the idle buffer with
        //an instruction for IDLE_BUFFER_PROCESS_ADD_NEWCHAR so as the new character is added to
        //the database at the next idle event
        check_new_character(command, (CreateChar *)command.data);
    }
    /**********************************************************************************************/

    else if(command.process_type==IDLE_BUFFER_PROCESS_ADD_NEWCHAR){

        //when a new character is created, after name has been checked, add to game
        add_new_character(command,(CreateChar *)command.data);
    }
    /**********************************************************************************************/

    else if(command.process_type==IDLE_BUFFER_PROCESS_LOGIN){

        process_log_in(*command.client, (const LogIn *)command.data);
        delete (LogIn *)command.data;
    }
    /**********************************************************************************************/

    else if(command.process_type==IDLE_BUFFER_PROCESS_SQL){

        process_sql(command.sql.c_str());
    }
    /**********************************************************************************************/

    else {

        log_event(EVENT_ERROR, "unknown process type in function %s: module %s: line %i", __func__, __FILE__, __LINE__);
        stop_server();
    }

    //compress buffer
    idle_buffer.pop_front();
}


