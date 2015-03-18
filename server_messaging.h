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

#ifndef SERVER_MESSAGING_H_INCLUDED
#define SERVER_MESSAGING_H_INCLUDED

#define MOTD_FILE "motd.msg"
#define SERVER_WELCOME_MSG "\nWELCOME TO THE UNOFF SERVER\n--------------------------------------------------------------------------------------------"

enum { // channel types
    CHAT_LOCAL,
    CHAT_PERSONAL,
    CHAT_GM,
    CHAT_SERVER,
    CHAT_MOD,
    CHAT_CHANNEL1,
    CHAT_CHANNEL2,
    CHAT_CHANNEL3,
    CHAT_MODPM,
    CHAT_SERVER_PM,
};

/** RESULT  : sends the motd header to connection

    RETURNS : void

    PURPOSE : used by function send_motd function in module server_messaging.c and
              process_packet function in module client_protocol_handler

    NOTES   :
**/
void send_motd_header(int connection);


/** RESULT  : sends the content of the motd file to connection

    RETURNS : void

    PURPOSE : used by send_motd function and process_hash_commands

    NOTES   :
**/
int send_motd_file(int connection);


/** RESULT  : sends the motd header and contents of the motd file to a connected client

    RETURNS : void

    PURPOSE : modularises code in function main of module main.c

    NOTES   :
*/
void send_motd(int connection);

#endif // SERVER_MESSAGING_H_INCLUDED