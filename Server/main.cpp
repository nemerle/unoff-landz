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
/******************************************************************************************************************

                                    COMPILER SETTINGS

To compile server, set the following compiler flags :

    -fno-strict-aliasing    - removes warning: dereferencing type-punned pointer will break strict-aliasing rules


                                    LINKING INFORMATION

To compile server, link with the following libraries :

    libev.a                 - libev event library
    libsqlite3.so           - sqlite database library

*******************************************************************************************************************/
#define _GNU_SOURCE 1   //supports TEMP_FAILURE_RETRY
#include <cstdio>      //supports printf function
#include <cstdlib>     //supports free function
#include <cstring>     //supports memset and strcpy functions
#include <errno.h>      //supports errno function support
#include <unistd.h>     //supports close function
#include <arpa/inet.h>  //supports recv and accept function
#include <ev.h>         //supports ev event library
#include <fcntl.h>      //supports fcntl
#include <cassert>      //supports fcntl

#include "server_parameters.h"
#include "global.h"
#include "logging.h"
#include "server_messaging.h"
#include "clients.h"
#include "game_data.h"
#include "db/database_functions.h"
#include "client_protocol_handler.h"
#include "server_protocol_functions.h"
#include "db/db_character_tbl.h"
#include "db/db_character_race_tbl.h"
#include "db/db_map_tbl.h"
#include "db/db_character_type_tbl.h"
#include "db/db_gender_tbl.h"
#include "db/db_chat_channel_tbl.h"
#include "db/db_game_data_tbl.h"
#include "db/db_attribute_tbl.h"
#include "db/db_season_tbl.h"
#include "date_time_functions.h"
#include "broadcast_actor_functions.h"
#include "movement.h"
#include "server_start_stop.h"
#include "attributes.h"
#include "chat.h"
#include "characters.h"
#include "idle_buffer.h"
#include "file_functions.h"

#define DEBUG_MAIN 1
#define VERSION "4"

struct ev_io *libevlist[MAX_CLIENTS] = {nullptr};

void socket_accept_callback(struct ev_loop *loop, struct ev_io *watcher, int revents);
void socket_read_callback(struct ev_loop *loop, struct ev_io *watcher, int revents);

//declare prototypes
void timeout_cb(EV_P_ struct ev_timer* timer, int revents);
void timeout_cb2(EV_P_ struct ev_timer* timer, int revents);
void idle_cb(EV_P_ struct ev_idle *watcher, int revents);
void close_connection_slot(client_node_type &cl);

/** RESULT   : starts the server

    RETURNS  : void

    PURPOSE  :

    NOTES    :
**/
void start_server(const char *db_filename){


    struct ev_loop *loop = ev_default_loop(0);

    ev_io *socket_watcher = (struct ev_io*)malloc(sizeof(struct ev_io));
    ev_idle *idle_watcher=(struct ev_idle*)malloc(sizeof(struct ev_idle));
    ev_timer *timeout_watcher = (struct ev_timer*)malloc(sizeof(struct ev_timer));
    ev_timer *timeout_watcher2 = (struct ev_timer*)malloc(sizeof(struct ev_timer));

    sockaddr_in server_addr;

    int sd;
    int loaded=0;

    //set server start time
    game_data.server_start_time=time(nullptr);

    //display console message
    char time_stamp_str[9]="";
    char verbose_date_stamp_str[50]="";
    get_time_stamp_str(game_data.server_start_time, time_stamp_str);
    get_verbose_date_str(game_data.server_start_time, verbose_date_stamp_str);
    printf("SERVER START at %s on %s\n", time_stamp_str, verbose_date_stamp_str);

    //clear the logs
    initialise_logs();

    //open database
    if(file_exists(db_filename)==FALSE){

        log_event(EVENT_ERROR, "database file [%s] not found", db_filename);
        stop_server();
    }
    else open_database(db_filename);

    //check the database table count
    int tbl_count=database_table_count();
    log_event(EVENT_INITIALISATION, "[%i] Database tables detected", tbl_count);

    if(tbl_count==0) {

        log_event(EVENT_ERROR, "no tables in database");
        stop_server();
    }

    log_text(EVENT_INITIALISATION, "");//insert logical separator in log file

    //load maps from database
    loaded=load_db_maps();
    if(loaded==0){

        log_event(EVENT_ERROR, "no maps found in database", loaded);
        stop_server();
    }else log_text(EVENT_INITIALISATION, "");//insert logical separator in log file

    //load races from database
    loaded=load_db_char_races();
    if(loaded==0){

        log_event(EVENT_ERROR, "no races found in database", loaded);
        stop_server();
    }else log_text(EVENT_INITIALISATION, "");//insert logical separator in log file

    //load genders from database
    loaded=load_db_genders();
    if(loaded==0){

        log_event(EVENT_ERROR, "no genders found in database", loaded);
        stop_server();
    }else log_text(EVENT_INITIALISATION, "");//insert logical separator in log file

    //load char types from database
    loaded=load_db_char_types();
    if(loaded==0){

        log_event(EVENT_ERROR, "no character types found in database", loaded);
        stop_server();
    }else log_text(EVENT_INITIALISATION, "");//insert logical separator in log file

    //load attribute types from database
    loaded=load_db_attributes();
    if(loaded==0){

        log_event(EVENT_ERROR, "no attributes found in database", loaded);
        stop_server();
    }
    log_text(EVENT_INITIALISATION, "");//insert logical separator in log file

    //load chat channels from database
    loaded=load_db_channels();
    if(loaded==0){

        log_event(EVENT_ERROR, "no chat channels found in database", loaded);
        stop_server();
    }else log_text(EVENT_INITIALISATION, "");//insert logical separator in log file

    //load game data from database
    loaded=load_db_game_data();
    if(loaded!=1){

        log_event(EVENT_ERROR, "no game data found in database", loaded);
        stop_server();
    }else log_text(EVENT_INITIALISATION, "");//insert logical separator in log file

    //load seasons from database
    loaded=load_db_seasons();
    if(loaded==0){

        log_event(EVENT_ERROR, "no seasons found in database", loaded);
        stop_server();
    }else log_text(EVENT_INITIALISATION, "");//insert logical separator in log file

    //set global data
    get_db_last_char_created(); //loads details of the last char created from the database into the game_data struct
    game_data.char_count=get_db_char_count();

    initialise_movement_vectors();

    //create server socket & bind it to socket address
    if((sd = socket(AF_INET, SOCK_STREAM, 0))==-1){

        int errnum=errno;

        log_event(EVENT_ERROR, "socket failed in function %s: module %s: line %i", __func__, __FILE__, __LINE__);
        log_text(EVENT_ERROR, "error [%i] [%s]", errnum, strerror(errnum));
        stop_server();
    }

    //clear struct
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    log_event(EVENT_INITIALISATION, "setting up server socket on address [%s]: port [%i]", inet_ntoa(server_addr.sin_addr), PORT);

    int bReuseaddr = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (const char*) &bReuseaddr, sizeof(bReuseaddr)) != 0) {

        int errnum=errno;

        log_event(EVENT_ERROR, "setsockopt failed in function %s: module %s: line %i", __func__, __FILE__, __LINE__);
        log_text(EVENT_ERROR, "error [%i] [%s]", errnum, strerror(errnum));
        stop_server();
    }

    //bind the server socket to an address
    if(bind(sd, (struct sockaddr*) &server_addr, sizeof(server_addr))==-1){

        int errnum=errno;

        log_event(EVENT_ERROR, "bind failed in function %s: module %s: line %i", __func__, __FILE__, __LINE__);
        log_text(EVENT_ERROR, "error [%i] [%s]", errnum, strerror(errnum));
        stop_server();
    }

    if(listen(sd, 5)==-1){

        int errnum=errno;

        log_event(EVENT_ERROR, "listen failed in function %s: module %s: line %i", __func__, __FILE__, __LINE__);
        log_text(EVENT_ERROR, "error [%i] [%s]", errnum, strerror(errnum));
        stop_server();
    }

    ev_timer_init(timeout_watcher, timeout_cb, 0.05, 0.05);
    ev_timer_start(loop, timeout_watcher);

    ev_timer_init(timeout_watcher2, timeout_cb2, GAME_MINUTE_INTERVAL, GAME_MINUTE_INTERVAL);
    ev_timer_start(loop, timeout_watcher2);

    ev_io_init(socket_watcher, socket_accept_callback, sd, EV_READ);
    ev_io_start(loop, socket_watcher);

    ev_idle_init(idle_watcher, idle_cb);
    ev_idle_start(loop, idle_watcher);

    log_event(EVENT_INITIALISATION, "server initialisation complete");

    while(1) {
        ev_run(loop, 0);
    }
}
/**
 * \brief handles new client connections
 */
void socket_accept_callback(struct ev_loop *loop, struct ev_io *watcher, int revents) {
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_sd;

    struct ev_io *client_watcher = (struct ev_io*) malloc(sizeof(struct ev_io));
    if (client_watcher == nullptr) {

        log_event(EVENT_ERROR, "malloc failed in function %s: module %s: line %i", __func__, __FILE__, __LINE__);
        stop_server();
    }

    if (EV_ERROR & revents) {

        log_event(EVENT_ERROR, "EV error in function %s: module %s: line %i", __func__, __FILE__, __LINE__);
        stop_server();
    }

    // socket accept: get file description
    client_sd = accept(watcher->fd, (struct sockaddr*) &client_addr, &client_len);
    if (client_sd ==-1) {

        int errnum=errno;

        log_event(EVENT_ERROR, "accept failed in function %s: module %s: line %i", __func__, __FILE__, __LINE__);
        log_text(EVENT_ERROR, "socket [%i] error [%i] [%s]", watcher->fd, errnum, strerror(errnum));
        stop_server();
    }

    // too many connections
    if (client_sd > MAX_CLIENTS) {

        log_event(EVENT_ERROR, "new connection [%i] exceeds client array max [%i] ", client_sd, MAX_CLIENTS);
        client_node_type fake_client;
        fake_client.socket_fd = client_sd;
        //send message to client and deny connection
        send_raw_text(fake_client, CHAT_SERVER, "\nSorry but the server is currently full\n");
        close(client_sd);
        return;
    }

#if DEBUG_MAIN==1
    printf("client [%i] connected\n", client_sd);
#endif
    clients[client_sd] = new client_node_type;
    client_node_type *new_client = clients[client_sd];
    new_client->socket_fd = client_sd;

    // listen to new client
    ev_io_init(client_watcher, socket_read_callback, client_sd, EV_READ);
    ev_io_start(loop, client_watcher);

    libevlist[client_sd] = client_watcher;

    //set up connection data entry in client struct
    new_client->client_status=client_node_type::CONNECTED;
    strcpy(new_client->ip_address, inet_ntoa(client_addr.sin_addr));

    //set up heartbeat
    gettimeofday(&time_check, nullptr);
    new_client->time_of_last_heartbeat=time_check.tv_sec;

    //send welcome message and motd to client
    send_raw_text(*new_client, CHAT_SERVER, SERVER_WELCOME_MSG);
    send_motd(*new_client);
    send_raw_text(*new_client, CHAT_SERVER, "\nHit any key to continue...\n");
}

/** \brief handles socket read event on existing client connections
    \returns void
*/
void socket_read_callback(struct ev_loop *loop, struct ev_io *watcher, int revents) {


    uint8_t buffer[1024];
    uint8_t packet[1024];
    ssize_t read;

    if (EV_ERROR & revents) {

        log_event(EVENT_ERROR, "EV error in function %s: module %s: line %i", __func__, __FILE__, __LINE__);
        stop_server();
    }

    //read = recv(watcher->fd, buffer, 512, 0); // read stream to buffer

    //wrapping recv in this macro prevents connection reset by peer errors
    read = TEMP_FAILURE_RETRY(recv(watcher->fd, buffer, 512, 0));

    if (read <0) {

        int errnum=errno;

        log_event(EVENT_ERROR, "read failed in function %s: module %s: line %i", __func__, __FILE__, __LINE__);
        log_text(EVENT_ERROR, "sock [%i] error [%i] [%s]", watcher->fd, errnum, strerror(errnum));

#if DEBUG_MAIN==1
        printf("read failed for client [%i] with error [%i] [%s]\n", watcher->fd, errnum, strerror(errnum));
#endif

        log_event(EVENT_SESSION, "closing client [%i] following read error", watcher->fd);
        auto iter = clients.find(watcher->fd);
        assert(iter!=clients.end());

        close_connection_slot(*iter->second);

        ev_io_stop(loop, libevlist[watcher->fd]);
        free(libevlist[watcher->fd]);
        libevlist[watcher->fd] = nullptr;

        delete iter->second;
        clients.erase(iter);
        return;
    }

    if (read == 0) {

#if DEBUG_MAIN==1
        printf("client [%i] disconnected\n", watcher->fd);
#endif

        if (libevlist[watcher->fd]!= nullptr) {

            auto iter = clients.find(watcher->fd);
            assert(iter!=clients.end());
            close_connection_slot(*iter->second);

            ev_io_stop(loop, libevlist[watcher->fd]);
            ev_io *bkp = libevlist[watcher->fd];
            libevlist[watcher->fd] = nullptr;
            free(bkp);

            delete iter->second;
            clients.erase(iter);
        }
        return;
    }

    //check for data received from client
    if(read>0){

        log_event(EVENT_SESSION, "bytes received [%i]", read);
        client_node_type *client_ptr = clients[watcher->fd];
        assert(client_ptr!=nullptr);
        client_node_type &client(*client_ptr);

        //copy new bytes to client packet buffer
        client.packet_buffer.insert(client.packet_buffer.end(),buffer,buffer+read);

        //if data is in the buffer then read it
        if( !client.packet_buffer.empty() ) {

            do {

                int lsb=client.packet_buffer[1];
                int msb=client.packet_buffer[2];

                int packet_length=lsb+(msb*256)+2;

                //update heartbeat
                client.time_of_last_heartbeat=time_check.tv_sec;

                //if insufficient data received then wait for more data
                if(client.packet_buffer.size()<packet_length)
                    break;
                assert(packet_length<1024);
                //copy packet from buffer
                for(int j=0; j<packet_length; j++){
                    packet[j]=client.packet_buffer[j];
                }

                //process packet
                process_packet(client, packet,packet_length);

                // remove packet from buffer / pop_front
                client.packet_buffer.erase(client.packet_buffer.begin(),client.packet_buffer.begin()+packet_length);

            } while(1);
        }
    }
}


/** RESULT   : closes a client connection

    RETURNS  : void

    PURPOSE  : used in socket_accept_callback and socket_read_callback

    NOTES    :
**/
void close_connection_slot(client_node_type &cl){



    if(cl.client_status==client_node_type::LOGGED_IN){

        //broadcast to local
        broadcast_remove_actor_packet(cl);

        //update last in game time for char
        cl.time_of_last_minute=time(nullptr);

        char sql[MAX_SQL_LEN]="";
        snprintf(sql, MAX_SQL_LEN, "UPDATE CHARACTER_TABLE SET LAST_IN_GAME=%i WHERE CHAR_ID=%i;",(int)cl.time_of_last_minute, cl.character_id);
        db_push_buffer(sql);
    }

    close(cl.socket_fd);
}


void timeout_cb2(EV_P_ struct ev_timer* timer, int revents){

    /**     RESULT   : handles timeout event

            RETURNS  : void

            PURPOSE  : handles game time updates

            NOTES    :
    **/

    (void)(timer);//removes unused parameter warning
    (void)(loop);

    if (EV_ERROR & revents) {

        log_event(EVENT_ERROR, "EV error in function %s: module %s: line %i", __func__, __FILE__, __LINE__);
        stop_server();
    }

    char sql[MAX_SQL_LEN]="";
    game_data.game_minutes++;

    if(game_data.game_minutes>360){

        game_data.game_minutes=0;
        game_data.game_days++;

        snprintf(sql, MAX_SQL_LEN, "UPDATE GAME_DATA_TABLE SET GAME_DAYS=%i WHERE GAME_DATA_ID=1", game_data.game_days);
        db_push_buffer(sql);
    }

    snprintf(sql, MAX_SQL_LEN, "UPDATE GAME_DATA_TABLE SET GAME_MINUTES=%i WHERE GAME_DATA_ID=1", game_data.game_minutes);
    db_push_buffer(sql);
}


void timeout_cb(EV_P_ struct ev_timer* timer, int revents){

    /** RESULT   : handles timeout event

        RETURNS  : void

        PURPOSE  : handles fixed interval processing tasks

        NOTES    :
    **/

    (void)(timer);//removes unused parameter warning
    (void)(loop);

    if (EV_ERROR & revents) {

        log_event(EVENT_ERROR, "EV error in function %s: module %s: line %i", __func__, __FILE__, __LINE__);
        stop_server();
    }

    //update time_check struct
    gettimeofday(&time_check, nullptr);

    //check through each connect client and process pending actions
    for(auto iter = clients.begin(),fin = clients.end(); iter!=fin; ) {
        client_node_type *client = iter->second;
        //restrict to clients that are logged on or connected
        if(client->client_status==client_node_type::LOGGED_IN || client->client_status==client_node_type::CONNECTED) {

            //check for lagged connection
            if(client->time_of_last_heartbeat+HEARTBEAT_INTERVAL<time_check.tv_sec){

#if DEBUG_MAIN==1
                printf("Client lagged out [%i] [%s]\n", client->id(),  client->char_name.c_str());
#endif

                log_event(EVENT_SESSION, "client [%i] char [%s] lagged out", client->id(),  client->char_name.c_str());

                close_connection_slot(*client);

                ev_io_stop(loop, libevlist[client->socket_fd]);
                free(libevlist[client->socket_fd]);
                libevlist[client->socket_fd] = nullptr;
                iter = clients.erase(iter);
                delete client;
                continue;
            }

            //restrict to clients that are logged on
            if(client->client_status==client_node_type::LOGGED_IN) {

                //update client game time
                if(client->time_of_last_minute+GAME_MINUTE_INTERVAL<time_check.tv_sec){

                    client->time_of_last_minute=time_check.tv_sec;
                    send_new_minute(*client, game_data.game_minutes);

                    //update database with time char was last in game
                    char sql[MAX_SQL_LEN]="";
                    snprintf(sql, MAX_SQL_LEN, "UPDATE CHARACTER_TABLE SET LAST_IN_GAME=%i WHERE CHAR_ID=%i;",(int)client->time_of_last_minute, client->character_id);
                    db_push_buffer(sql);
                }

                //process any char movements
                process_char_move(*client, time_check.tv_usec);
            }
            ++iter;
        }
    }
}


void idle_cb (struct ev_loop *loop, struct ev_idle *watcher, int revents){

    /** RESULT   : handles server idle event

        RETURNS  : void

        PURPOSE  : enables idle event to be used for low priority processing tasks

        NOTES    :
    **/

    (void)(loop);
    (void)(watcher);

    if (EV_ERROR & revents) {

        log_event(EVENT_ERROR, "EV error in function %s: module %s: line %i", __func__, __FILE__, __LINE__);
        stop_server();
    }

    db_process_buffer();
}


int main(int argc, char *argv[]){

    /** RESULT   : handles command line arguments

        RETURNS  : dummy

        PURPOSE  : allows program to be started in different modes

        NOTES    :
    **/

    printf("UnoffLandz Server - version %s\n\n", VERSION);

    if(argc==1){

        printf("command line options...\n");
        printf("-C optional [""database file name""]      ...create database\n");
        printf("-S optional [""database file name""]      ...start server\n");
        printf("-L [map id] [""map name""] [""map file""]     ...load map\n");
        printf("-B [map id] [map tile]                ...character start location\n");
        printf("-E [map_id] [map tile]                ...beam me target\n");

        exit(EXIT_FAILURE);
    }

    if (argv[1][0] == '-') {

        if(argv[1][1]=='S'){//start server

            if(argc>2){

                start_server(argv[2]);
            }else start_server(DATABASE_FILE_NAME);
        }
        else if(argv[1][1]=='L'){//add or update map
            open_database(DATABASE_FILE_NAME);
            //use uintptr_t to prevent int truncation issues when compiled as 64bit
            if(get_db_map_exists((uintptr_t)atoi(argv[2]))==TRUE){

                //use uintptr_t to prevent int truncation issues when compiled as 64bit
                add_db_map(atoi(argv[2]), (char*)argv[3], (char*)argv[4]);
            }
            else {

                //use uintptr_t to prevent int truncation issues when compiled as 64bit
                add_db_map(atoi(argv[2]), (char*)argv[3], (char*)argv[4]);
            }
        }
        else if(argv[1][1]=='C'){ // create database

            if(argc>2){

                open_database(argv[2]);
            }else open_database(DATABASE_FILE_NAME);

            create_default_database();
        }
        else { //unknown command line option

            printf("unknown command line option [%s]\n", (char*)argv[1]);
        }
    }

    return 0; //otherwise we get 'control reaches end of non-void function
}

