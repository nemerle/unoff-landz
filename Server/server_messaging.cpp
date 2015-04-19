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

#include "server_messaging.h"
#include "global.h"
#include "date_time_functions.h"
#include "colour.h"
#include "string_functions.h"
#include "clients.h"
#include "game_data.h"
#include "server_protocol_functions.h"
#include "logging.h"

#include <stdio.h> //support for FILE data type, sprintf, fopen, fgets, fclose
#include <string.h> //support for strcpy
#include <string>


void send_motd_header(const client_node_type &client){

    /** public function - see header */

    char text_out[8092]="";
    char time_stamp_str[9]="";
    char verbose_date_stamp_str[50]="";

    std::string chars_in_game_str;
    int chars_in_game_count=0;
    long int server_up_time=0;
    char time_up_str[50]="";

    //send separator line so MOTD is distinct from previous text
    send_raw_text(client, CHAT_SERVER, " ");

    //prepare and send server 'up' time
    server_up_time=time(nullptr)-game_data.server_start_time;
    get_time_up_str(server_up_time, time_up_str);

    snprintf(text_out,8092, "%cServer up                  : %s", c_blue2+127, time_up_str);
    send_raw_text(client, CHAT_SERVER, text_out);

    //prepare and send server start time
    get_time_stamp_str(game_data.server_start_time, time_stamp_str);
    get_verbose_date_str(game_data.server_start_time, verbose_date_stamp_str);
    snprintf(text_out,8092, "%cServer started at          : %s on %s", c_blue2+127, time_stamp_str, verbose_date_stamp_str);
    send_raw_text(client, CHAT_SERVER, text_out);

    //prepare list of names for characters in game
    for(const auto &v : clients) {
        const client_node_type *client(v.second);
        if(client->client_status==client_node_type::LOGGED_IN) {
            chars_in_game_str+=client->char_name;
            chars_in_game_str+=" ";
            chars_in_game_count++;
        }

    }

    if(chars_in_game_count==0) {
        snprintf(text_out,8092, "%cCharacters in game         : [0] None", c_blue2+127);
    }
    else {
        snprintf(text_out,8092, "%cCharacters in game         : [%i] %s", c_blue2+127,
                chars_in_game_count,  chars_in_game_str.c_str());
    }

    send_raw_text(client, CHAT_SERVER, text_out);

    snprintf(text_out,8092, "%cCharacters created to date : %i", c_blue2+127, game_data.char_count);
    send_raw_text(client, CHAT_SERVER, text_out);

    if(game_data.char_count>0){

        get_time_stamp_str(game_data.date_last_char_created, time_stamp_str);
        get_verbose_date_str(game_data.date_last_char_created, verbose_date_stamp_str);
     }
    else {

        get_time_stamp_str(time(nullptr), time_stamp_str);
        get_verbose_date_str(time(nullptr), verbose_date_stamp_str);

        game_data.name_last_char_created ="None";
    }

    snprintf(text_out,8092, "%cLast character created     : %s at %s %s", c_blue2+127,
             game_data.name_last_char_created.c_str(), time_stamp_str, verbose_date_stamp_str);
    send_raw_text(client, CHAT_SERVER, text_out);

    //prepare and send time and date of this connection
    get_time_stamp_str(time(nullptr), time_stamp_str);
    get_verbose_date_str(time(nullptr), verbose_date_stamp_str);
    snprintf(text_out,8092, "%cConnection at              : %s on %s", c_blue2+127,
             time_stamp_str, verbose_date_stamp_str);
    send_raw_text(client, CHAT_SERVER, text_out);

    //prepare and send connection ip address
    snprintf(text_out,8092, "%cConnection IP address      : %s", c_blue2+127, client.ip_address);
    send_raw_text(client, CHAT_SERVER, text_out);

    //send separator line so MOTD is distinct from subsequent text
    send_raw_text(client, CHAT_SERVER, " ");
}

int send_motd_file(client_node_type &client){

    /** public function - see header */

    FILE *file;
    char line_in[1024]="";

    if((file=fopen(MOTD_FILE, "r"))==nullptr)
        return NOT_FOUND;

    while( fgets(line_in, 80, file) != nullptr){
        str_trim_right(line_in);
        if(strcmp(line_in, "")==0) strcpy(line_in, " ");
        send_raw_text(client, CHAT_SERVER, line_in);
    }

    fclose(file);

    return FOUND;
}

void send_motd(client_node_type &client){

    /** public function - see header */

    send_motd_header(client);

    //if there's an motd file then send to client otherwise log that no motd file was found
    if(send_motd_file(client)!=FOUND){
        log_event(EVENT_SESSION, "no motd file available for connection [%i]", client.id());
    }
}

