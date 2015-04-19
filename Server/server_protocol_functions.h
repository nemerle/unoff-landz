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

#ifndef SERVER_PROTOCOL_FUNCTIONS_H_INCLUDED
#define SERVER_PROTOCOL_FUNCTIONS_H_INCLUDED

#include "logging.h"
#include "Common/PacketToBuffer.h"
#include <stdlib.h> //support for int16_t data type

struct client_node_type;

/** RESULT  : sends packet from the server

    RETURNS : void

    PURPOSE : groups all server communications so as packets sent from server can be monitored
              from a single source
*/
void send_packet(const client_node_type &client, const std::vector<uint8_t> &buf);


template<class T>
void send_packet(const client_node_type &client, const T &src) {
    std::vector<uint8_t> tgt;
    packetToBuffer(tgt,src);
    send_packet(client,tgt);
}


/** RESULT  : sends the log_in_ok packet to the client

    RETURNS : void

    PURPOSE : to let the client know that char has been successfully logged in
*/
void send_login_ok(const client_node_type &client);


/** RESULT  : sends the log_in_not_ok packet to the client

    RETURNS : void

    PURPOSE : to let the client know that char has not been successfully logged in
*/
void send_login_not_ok(const client_node_type &client);


/** RESULT  : sends the you_dont_exist packet to the client

    RETURNS : void

    PURPOSE : to let the client know that char does not exist
*/
void send_you_dont_exist(client_node_type &client);


/** RESULT  : sends the create_char_ok packet to the client

    RETURNS : void

    PURPOSE : sent following successful char creation
*/
void send_create_char_ok(client_node_type &client);


/** RESULT  : sends the create_char_not_ok packet to the client

    RETURNS : void

    PURPOSE : sent following unsuccessful char creation
*/
void send_create_char_not_ok(client_node_type &client);


/** RESULT  : sends the you_are packet to the client

    RETURNS : void

    PURPOSE : sends the client a code to identify connection following log in
*/
void send_you_are(client_node_type &client);


/** RESULT  : sends the raw_text packet to client

    RETURNS : void

    PURPOSE : sends text messages to connected clients via the specified channel

    NOTES   :
*/
void send_raw_text(const client_node_type &client, int chan_type, const char *text);


/** RESULT  : sends the here_your_inventory packet to client

    RETURNS : void

    PURPOSE : sends the character inventory to client at log-in

    NOTES   :
*/
void send_here_your_inventory(client_node_type &client);


/** RESULT  : sends the get_active_channels packet to client

    RETURNS : void

    PURPOSE : sends active chat channels to client at log-in

    NOTES   :
*/
void send_get_active_channels(client_node_type &client);


/** RESULT  : sends the here_your_stats packet to client

    RETURNS : void

    PURPOSE : sends the characters stats to client at log-in

    NOTES   :
*/
void send_here_your_stats(client_node_type &tgt);


/** RESULT  : sends the change_map packet to client

    RETURNS : void

    PURPOSE : informs the client of the map to be used with this character

    NOTES   :
*/
void send_change_map(client_node_type &tgt, char *elm_filename);


/** RESULT  : sends the new_minute packet

    RETURNS : void

    PURPOSE :

    NOTES   : used to set client game time
*/
void send_new_minute(const client_node_type &client, int16_t minute);


/** RESULT  : sends the active channels packet

    RETURNS : void

    PURPOSE :

    NOTES   : used at log in
*/
void send_get_active_channels(int connection);

#endif // SERVER_PROTOCOL_FUNCTIONS_H_INCLUDED
