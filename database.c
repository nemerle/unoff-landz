#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>

#include "global.h"
#include "database.h"
#include "log_in.h"
#include "character_inventory.h"
#include "files.h"
#include "numeric_functions.h"

void open_database(char *database_name){

   /** public function - see header */

    int rc;
    char text_out[1024]="";

    rc = sqlite3_open(database_name, &db);

    if( rc ){
        sprintf(text_out, "Can't open database [%s]. Error [%s] in function open_database: module database.c", database_name, sqlite3_errmsg(db));
        log_event(EVENT_ERROR, text_out);
        exit(EXIT_FAILURE);
  }
    else{
        printf("\nOpened database [%s] successfully\n", database_name);
    }
}

int get_table_count(){

    /** public function - see header */

    int rc;
    sqlite3_stmt *stmt;

    int table_count=0;

    char sql[1024]="SELECT count(*) FROM sqlite_master WHERE type='table';";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    while ( (rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        table_count=sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    return table_count;
}

int get_max_char_id(){

    /** public function - see header */

    int rc;
    sqlite3_stmt *stmt;

    int max_id=0;

    char sql[1024]="SELECT MAX(CHAR_ID) FROM CHARACTER_TABLE;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

     while ( (rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        max_id=sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    return max_id;
}

int get_chars_created_count(){

    /** public function - see header */

    int rc;
    sqlite3_stmt *stmt;

    int char_count=0;

    char sql[1024]="SELECT count(CHAR_ID) FROM CHARACTER_TABLE;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

     while ( (rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        char_count=sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    return char_count;
}

void get_last_char_created(){

    /** public function - see header */

    int rc;
    sqlite3_stmt *stmt;

    char sql[1024]="SELECT CHAR_NAME, CHAR_CREATED FROM CHARACTER_TABLE ORDER BY CHAR_CREATED LIMIT 1;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    while ( (rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        strcpy(game_data.name_last_char_created, (char*)sqlite3_column_text(stmt, 0));
        game_data.date_last_char_created=sqlite3_column_int(stmt,1);
    }

    sqlite3_finalize(stmt);
}

void add_char(struct character_type character){

    /** public function - see header */

    int i=0;
    int rc;
    sqlite3_stmt *stmt;
    char text_out[1024]="";
    int char_id=0;
    char * sErrMsg = 0;

    char sql[1024] ="INSERT INTO CHARACTER_TABLE(" \
        "CHAR_NAME," \
        "PASSWORD," \
        "CHAR_STATUS," \
        "ACTIVE_CHAN," \
        "CHAN_0," \
        "CHAN_1," \
        "CHAN_2," \
        "GM_PERMISSION," \
        "IG_PERMISSION," \
        "MAP_ID," \
        "MAP_TILE," \
        "CHAR_TYPE," \
        "SKIN_TYPE," \
        "HAIR_TYPE," \
        "SHIRT_TYPE," \
        "PANTS_TYPE," \
        "BOOTS_TYPE," \
        "HEAD_TYPE," \
        "SHIELD_TYPE," \
        "WEAPON_TYPE," \
        "CAPE_TYPE," \
        "HELMET_TYPE," \
        "MAX_HEALTH," \
        "CURRENT_HEALTH," \
        "VISUAL_PROXIMITY," \
        "LOCAL_TEXT_PROXIMITY," \
        "CHAR_CREATED" \
        ") VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    sqlite3_bind_text(stmt, 1, character.char_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, character.password, -1, SQLITE_STATIC);

    sqlite3_bind_int(stmt, 3, CHAR_ALIVE); // char status

    sqlite3_bind_int(stmt, 4, character.active_chan);
    sqlite3_bind_int(stmt, 5, character.chan[0]);
    sqlite3_bind_int(stmt, 6, character.chan[1]);
    sqlite3_bind_int(stmt, 7, character.chan[2]);

    sqlite3_bind_int(stmt, 8, FALSE); // gm permission
    sqlite3_bind_int(stmt, 9, FALSE); // ig permission

    sqlite3_bind_int(stmt, 10, character.map_id);
    sqlite3_bind_int(stmt, 11, character.map_tile);

    sqlite3_bind_int(stmt, 12, character.char_type);
    sqlite3_bind_int(stmt, 13, character.skin_type);
    sqlite3_bind_int(stmt, 14, character.hair_type);
    sqlite3_bind_int(stmt, 15, character.shirt_type);
    sqlite3_bind_int(stmt, 16, character.pants_type);
    sqlite3_bind_int(stmt, 17, character.boots_type);
    sqlite3_bind_int(stmt, 18, character.head_type);

    sqlite3_bind_int(stmt, 19, SHIELD_NONE);
    sqlite3_bind_int(stmt, 20, WEAPON_NONE);
    sqlite3_bind_int(stmt, 21, CAPE_NONE);
    sqlite3_bind_int(stmt, 22, HELMET_NONE);

    sqlite3_bind_int(stmt, 23, 0); // max health
    sqlite3_bind_int(stmt, 24, 0); // current health
    sqlite3_bind_int(stmt, 25, character.visual_proximity);
    sqlite3_bind_int(stmt, 26, character.local_text_proximity);
    sqlite3_bind_int(stmt, 27, character.char_created);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        sprintf(text_out, "Error %s executing '%s' in function add_char: module database.c", sql, sqlite3_errmsg(db));
        log_event(EVENT_ERROR, text_out);
        exit(EXIT_FAILURE);
    }

    sqlite3_finalize(stmt);

    //get id of the char we just created and add iventory slots to database
    char_id=get_max_char_id();

    strcpy(sql, "INSERT INTO INVENTORY_TABLE(CHAR_ID, SLOT) VALUES(?, ?)");

    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    //wrap in a transaction to speed up insertion
    sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &sErrMsg);

    //create the char inventory record on the database
    for(i=0; i<MAX_INVENTORY_SLOTS; i++){

        sqlite3_bind_int(stmt, 1, char_id);
        sqlite3_bind_int(stmt, 2, i);

        rc = sqlite3_step(stmt);

        if (rc != SQLITE_DONE) {
            sprintf(text_out, "Error %s executing '%s' in function add_char: module database.c", sql, sqlite3_errmsg(db));
            log_event(EVENT_ERROR, text_out);
            exit(EXIT_FAILURE);
        }

        sqlite3_clear_bindings(stmt);
        sqlite3_reset(stmt);
    }

    sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &sErrMsg);

    sqlite3_finalize(stmt);
}

void add_item(int image_id, char *item_name, int harvestable, int cycle_amount, int emu, int interval,
              int exp,
              int food_value,
              int food_cooldown,
              int organic_nexus,
              int vegetal_nexus){

    /** public function - see header */

    int rc;
    sqlite3_stmt *stmt;
    char text_out[1024]="";

    char sql[1024] ="INSERT INTO ITEM_TABLE("  \
        "IMAGE_ID,"  \
        "ITEM_NAME," \
        "HARVESTABLE,"  \
        "CYCLE_AMOUNT," \
        "EMU," \
        "INTERVAL," \
        "EXP," \
        "FOOD_VALUE," \
        "FOOD_COOLDOWN," \
        "ORGANIC_NEXUS," \
        "VEGETAL_NEXUS" \
        ") VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    sqlite3_bind_int(stmt, 1, image_id);
    sqlite3_bind_text(stmt, 2, item_name, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, harvestable);
    sqlite3_bind_int(stmt, 4, cycle_amount);
    sqlite3_bind_int(stmt, 5, emu);
    sqlite3_bind_int(stmt, 6, interval);
    sqlite3_bind_int(stmt, 7, exp);
    sqlite3_bind_int(stmt, 8, food_value);
    sqlite3_bind_int(stmt, 9, food_cooldown);
    sqlite3_bind_int(stmt, 10, organic_nexus);
    sqlite3_bind_int(stmt, 11, vegetal_nexus);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        sprintf(text_out, "Error %s executing '%s' in function add_item: module database.c", sql, sqlite3_errmsg(db));
        log_event(EVENT_ERROR, text_out);
        exit(EXIT_FAILURE);
    }

    sqlite3_finalize(stmt);

    printf("Added item [%s] to ITEM_TABLE\n", item_name);
}

void add_threed_object(char *filename, int image_id){

    /** public function - see header */

    int rc;
    sqlite3_stmt *stmt;
    char text_out[1024]="";

    char sql[1024] ="INSERT INTO THREED_OBJECT_TABLE(FILE_NAME, INVENTORY_IMAGE_ID) VALUES(?, ?);";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    sqlite3_bind_text(stmt, 1, filename, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, image_id);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        sprintf(text_out, "Error %s executing '%s' in function add_threed_object: module database.c", sql, sqlite3_errmsg(db));
        log_event(EVENT_ERROR, text_out);
        exit(EXIT_FAILURE);
   }

    sqlite3_finalize(stmt);

    printf("Added 3d object [%s] to THREED_OBJECT_TABLE\n", filename);
}

void add_map(int map_id, char *map_name, char *elm_file_name){

   /** public function - see header */

    int rc;
    sqlite3_stmt *stmt;
    char text_out[1024]="";

    char sql[1024] ="INSERT INTO MAP_TABLE("  \
        "MAP_ID," \
        "MAP_NAME,"  \
        "ELM_FILE_NAME" \
        ") VALUES( ?, ?, ?);";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    sqlite3_bind_int(stmt, 1, map_id);
    sqlite3_bind_text(stmt, 2, map_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, elm_file_name, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        sprintf(text_out, "Error %s executing '%s' in function add_map: module database.c", sql, sqlite3_errmsg(db));
        log_event(EVENT_ERROR, text_out);
        exit(EXIT_FAILURE);
   }

    sqlite3_finalize(stmt);

    printf("Added map [%s] to MAP_TABLE\n", map_name);
}

void add_channel(int channel_id, int owner_id, int channel_type, char *password, char *channel_name, char*channel_description){

    /** public function - see header */

    int rc;
    sqlite3_stmt *stmt;
    char text_out[1024]="";

    char sql[1024] ="INSERT INTO CHANNEL_TABLE("  \
        "CHANNEL_ID," \
        "OWNER_ID," \
        "TYPE," \
        "PASSWORD," \
        "NAME,"  \
        "DESCRIPTION" \
        ") VALUES( ?, ?, ?, ?, ?, ?)";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    sqlite3_bind_int(stmt, 1, channel_id);
    sqlite3_bind_int(stmt, 2, owner_id);
    sqlite3_bind_int(stmt, 3, channel_type);
    sqlite3_bind_text(stmt, 4, password, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, channel_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, channel_description, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        sprintf(text_out, "Error %s executing '%s' in function add_channel: module database.c", sql, sqlite3_errmsg(db));
        log_event(EVENT_ERROR, text_out);
        exit(EXIT_FAILURE);
    }

    sqlite3_finalize(stmt);

    printf("Added channel [%i] [%s] to CHANNEL_TABLE\n", channel_id, channel_name);
}

void add_race(int race_id, char *race_name, char *race_description, int initial_carry_capacity, int carry_capacity_multiplier){

    /** public function - see header */

    int rc;
    sqlite3_stmt *stmt;
    char text_out[1024]="";

    char sql[1024] ="INSERT INTO RACE_TABLE("  \
        "RACE_ID," \
        "RACE_NAME," \
        "RACE_DESCRIPTION," \
        "INITIAL_EMU," \
        "EMU_MULTIPLIER"  \
        ") VALUES( ?, ?, ?, ?, ?)";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    sqlite3_bind_int(stmt, 1, race_id);
    sqlite3_bind_text(stmt, 2, race_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, race_description, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, initial_carry_capacity);
    sqlite3_bind_int(stmt, 5, carry_capacity_multiplier);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        sprintf(text_out, "Error %s executing '%s' in function add_race: module database.c", sql, sqlite3_errmsg(db));
        log_event(EVENT_ERROR, text_out);
        exit(EXIT_FAILURE);
    }

    sqlite3_finalize(stmt);

    printf("Added race [%i] [%s] to RACE_TABLE\n", race_id, race_name);
}

int get_char_data_from_db(char *name){

    /** public function - see header */

    int slot=0;
    int rc;
    sqlite3_stmt *stmt;

    character.char_id=0; //set to zero as we use this to determine if the char exists

    char sql[1024]="SELECT * FROM CHARACTER_TABLE WHERE CHAR_NAME=?";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);

    //zero the struct
    memset(&character, 0, sizeof(character));

    while ( (rc = sqlite3_step(stmt)) == SQLITE_ROW) {

        character.char_id=sqlite3_column_int(stmt, 0);
        strcpy(character.char_name, (char*) sqlite3_column_text(stmt, 1));
        strcpy(character.password, (char*) sqlite3_column_text(stmt,2));
        character.char_status=sqlite3_column_int(stmt, 3);
        character.time_played=sqlite3_column_int(stmt, 4);
        character.active_chan=sqlite3_column_int(stmt, 5);
        character.chan[0]=sqlite3_column_int(stmt, 6);
        character.chan[1]=sqlite3_column_int(stmt, 7);
        character.chan[2]=sqlite3_column_int(stmt, 8);
        character.gm_permission=sqlite3_column_int(stmt, 9);
        character.ig_permission=sqlite3_column_int(stmt, 10);
        character.map_id=sqlite3_column_int(stmt, 11);
        character.map_tile=sqlite3_column_int(stmt, 12);
        character.guild_id=sqlite3_column_int(stmt, 13);
        character.char_type=sqlite3_column_int(stmt, 14);
        character.skin_type=sqlite3_column_int(stmt, 15);
        character.hair_type=sqlite3_column_int(stmt, 16);
        character.shirt_type=sqlite3_column_int(stmt, 17);
        character.pants_type=sqlite3_column_int(stmt, 18);
        character.boots_type=sqlite3_column_int(stmt, 19);
        character.head_type=sqlite3_column_int(stmt, 20);
        character.shield_type=sqlite3_column_int(stmt, 21);
        character.weapon_type=sqlite3_column_int(stmt, 22);
        character.cape_type=sqlite3_column_int(stmt, 23);
        character.helmet_type=sqlite3_column_int(stmt, 24);
        character.frame=sqlite3_column_int(stmt, 25);
        character.max_health=sqlite3_column_int(stmt, 26);
        character.current_health=sqlite3_column_int(stmt, 27);
        character.visual_proximity=sqlite3_column_int(stmt, 28);
        character.local_text_proximity=sqlite3_column_int(stmt, 29);
        character.last_in_game=sqlite3_column_int(stmt,  30);
        character.char_created=sqlite3_column_int(stmt, 31);
        character.joined_guild=sqlite3_column_int(stmt, 32);
        character.physique=sqlite3_column_int(stmt, 33);
        character.overall_exp=sqlite3_column_int(stmt, 34);
        character.harvest_exp=sqlite3_column_int(stmt, 35);

        //calculate max emu that can be held in inventory
        int initial_carry_capacity=race[character.char_type].initial_carry_capacity;
        int carry_capacity_multiplier=race[character.char_type].carry_capacity_multiplier;
        character.max_carry_capacity=initial_carry_capacity + (carry_capacity_multiplier * character.physique);
    }

    sqlite3_finalize(stmt);

    //get inventory
    strcpy(sql, "SELECT SLOT, IMAGE_ID, AMOUNT FROM INVENTORY_TABLE WHERE CHAR_ID=?");
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    sqlite3_bind_int(stmt, 1, character.char_id);

    while ( (rc = sqlite3_step(stmt)) == SQLITE_ROW) {

        slot=sqlite3_column_int(stmt, 0);
        character.client_inventory[slot].image_id=sqlite3_column_int(stmt, 1);
        character.client_inventory[slot].amount=sqlite3_column_int(stmt, 2);

        //calc total emu of inventory
        character.inventory_emu+=item[character.client_inventory[slot].image_id].emu * character.client_inventory[slot].amount;
    }

    sqlite3_finalize(stmt);

    if(character.char_id==0) return NOT_FOUND;

    return FOUND;
}

void create_database_table(char *table_name, char *sql){

    int rc;
    sqlite3_stmt *stmt;
    char text_out[1024]="";

    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        sprintf(text_out, "Error %s executing [%s] in function create_database_table: module database.c", sql, sqlite3_errmsg(db));
        log_event(EVENT_ERROR, text_out);
        exit(EXIT_FAILURE);
    }

    sqlite3_finalize(stmt);


    printf("Created [%s]\n", table_name);
}

void load_3d_objects(){

    /** public function - see header */

    int rc;
    sqlite3_stmt *stmt;
    int i=0;
    char text_out[1024]="";

    char sql[1024]="SELECT * FROM THREED_OBJECT_TABLE";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    while ( (rc = sqlite3_step(stmt)) == SQLITE_ROW) {

        strcpy(threed_object[i].file_name, (char*)sqlite3_column_text(stmt, 1));
        threed_object[i].inventory_image_id=sqlite3_column_int(stmt,2);

        i++;

        if(i==MAX_THREED_OBJECTS) {
            sprintf(text_out, "Maximum number of 3d objects exceeded in function load_3d_objects: module database.c");
            log_event(EVENT_ERROR, text_out);
            exit(EXIT_FAILURE);
        }
    }

    sqlite3_finalize(stmt);
}

void load_races(){

    /** public function - see header */

    int rc;
    sqlite3_stmt *stmt;
    char text_out[1024]="";
    int race_id=0;
    int i=0;

    char sql[1024]="SELECT * FROM RACE_TABLE";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    printf("\nloading races...\n");

    while ( (rc = sqlite3_step(stmt)) == SQLITE_ROW) {

        race_id=sqlite3_column_int(stmt, 0);

        if(race_id>MAX_RACES) {
            sprintf(text_out, "race_id [%i] exceeds max rang [0 - %i] in function load_races: module database.c", race_id, MAX_RACES);
            log_event(EVENT_ERROR, text_out);
            exit(EXIT_FAILURE);
        }

        strcpy(race[race_id].race_name, (char*)sqlite3_column_text(stmt, 1));
        strcpy(race[race_id].race_description, (char*)sqlite3_column_text(stmt, 2));
        race[race_id].initial_carry_capacity=sqlite3_column_int(stmt, 3);
        race[race_id].carry_capacity_multiplier=sqlite3_column_int(stmt, 4);
        race[race_id].char_count=sqlite3_column_int(stmt, 5);

        printf("loaded [%i] [%s]\n", race_id, race[race_id].race_name);

        i++;
    }

    sqlite3_finalize(stmt);

    printf("[%i] races were loaded\n", i);
}

void load_channels(){

    /** public function - see header */

    int rc;
    sqlite3_stmt *stmt;
    int i=0;
    char text_out[1024]="";
    int chan_id=0;

    char sql[1024]="SELECT * FROM CHANNEL_TABLE";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    printf("\nloading channels...\n");

    while ( (rc = sqlite3_step(stmt)) == SQLITE_ROW) {

        chan_id=sqlite3_column_int(stmt, 0);

        if(i>MAX_CHANNELS) {
            sprintf(text_out, "chan_id [%i] exceeds range [0 - %i] in function load_channels: module database.c", chan_id, MAX_CHANNELS);
            log_event(EVENT_ERROR, text_out);
            exit(EXIT_FAILURE);
        }

        channels.channel[chan_id]->channel_id=chan_id;
        channels.channel[chan_id]->chan_type=sqlite3_column_int(stmt, 1);
        channels.channel[chan_id]->owner_id=sqlite3_column_int(stmt, 2);
        strcpy(channels.channel[chan_id]->password, (char*)sqlite3_column_text(stmt, 3));
        strcpy(channels.channel[chan_id]->channel_name, (char*)sqlite3_column_text(stmt, 4));
        strcpy(channels.channel[chan_id]->description, (char*)sqlite3_column_text(stmt, 5));

        printf("loaded [%i] [%s]\n", i, channels.channel[i]->channel_name);

        i++;
    }

    sqlite3_finalize(stmt);

    printf("[%i] channels were loaded\n", i);
}

void load_items(){

    /** public function - see header */

    int rc;
    sqlite3_stmt *stmt;
    char text_out[1024]="";
    int id=0;

    char sql[1024]="SELECT * FROM ITEM_TABLE";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    while ( (rc = sqlite3_step(stmt)) == SQLITE_ROW) {

        id=sqlite3_column_int(stmt,0);

        if(id>MAX_ITEMS){
            sprintf(text_out, "item id [%i] exceeds range [0 - %i] in function load_items: module database.c", id, MAX_ITEMS);
            log_event(EVENT_ERROR, text_out);
            exit(EXIT_FAILURE);
        }

        strcpy(item[id].item_name, (char*)sqlite3_column_text(stmt, 1));
        item[id].harvestable=sqlite3_column_int(stmt,2);

        item[id].cycle_amount=sqlite3_column_int(stmt,3);
        item[id].emu=sqlite3_column_int(stmt,4);
        item[id].interval=sqlite3_column_int(stmt,5);
        item[id].exp=sqlite3_column_int(stmt,6);
        item[id].food_value=sqlite3_column_int(stmt,7);
        item[id].food_cooldown=sqlite3_column_int(stmt,8);
        item[id].organic_nexus=sqlite3_column_int(stmt,9);
        item[id].vegetal_nexus=sqlite3_column_int(stmt,10);
     }

    sqlite3_finalize(stmt);
}

void load_maps(){

    /** public function - see header */

    int rc;
    sqlite3_stmt *stmt;
    char text_out[1024]="";
    int map_id=0;
    int i=0;

    char sql[1024]="SELECT * FROM MAP_TABLE";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    printf("\nloading maps...\n");

    while ( (rc = sqlite3_step(stmt)) == SQLITE_ROW) {

        map_id=sqlite3_column_int(stmt,0);

        //make sure map_id doesn't exceed the size of the map array
        if(map_id>MAX_MAPS){
            sprintf(text_out, "map id [%i] exceeds range [0 - %i] in function load_maps: module database.c", map_id, MAX_MAPS);
            log_event(EVENT_ERROR, text_out);
            exit(EXIT_FAILURE);
        }

        strcpy(maps.map[map_id]->map_name, (char*)sqlite3_column_text(stmt, 1));
        strcpy(maps.map[map_id]->elm_filename, (char*)sqlite3_column_text(stmt, 2));

        load_map(map_id);
        printf("loaded [%i] %s\n", map_id, maps.map[map_id]->map_name);

        i++;
    }

    sqlite3_finalize(stmt);
    printf("[%i] maps were loaded\n", i);
}

void update_db_char_position(int connection){

    /** public function - see header */

    int rc;
    sqlite3_stmt *stmt;
    char text_out[1024]="";

    char sql[1024]="UPDATE CHARACTER_TABLE SET MAP_TILE=?, MAP_ID=? WHERE CHAR_ID=?;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    sqlite3_bind_int(stmt, 1, clients.client[connection]->map_tile);
    sqlite3_bind_int(stmt, 2, clients.client[connection]->map_id);
    sqlite3_bind_int(stmt, 3, clients.client[connection]->character_id);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        sprintf(text_out, "Error %s executing '%s' in function update_db_char_pos: module database.c", sql, sqlite3_errmsg(db));
        log_event(EVENT_ERROR, text_out);
        exit(EXIT_FAILURE);
    }

    sqlite3_finalize(stmt);
}

void update_db_char_name(int connection){

    /** public function - see header */

    int rc;
    sqlite3_stmt *stmt;
    char text_out[1024]="";

    char sql[1024]="UPDATE CHARACTER_TABLE SET CHAR_NAME='?' WHERE CHAR_ID=?;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    sqlite3_bind_text(stmt, 1, clients.client[connection]->char_name, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, clients.client[connection]->character_id);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        sprintf(text_out, "Error %s executing '%s' in function update_db_char_name: module database.c", sql, sqlite3_errmsg(db));
        log_event(EVENT_ERROR, text_out);
        exit(EXIT_FAILURE);
    }

    sqlite3_finalize(stmt);
 }

void update_db_char_frame(int connection){

    /** public function - see header */

    int rc;
    sqlite3_stmt *stmt;
    char text_out[1024]="";

    char sql[1024]="UPDATE CHARACTER_TABLE SET FRAME=? WHERE CHAR_ID=?;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    sqlite3_bind_int(stmt, 1, clients.client[connection]->frame);
    sqlite3_bind_int(stmt, 2, clients.client[connection]->character_id);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        sprintf(text_out, "Error %s executing '%s' in function update_db_char_frame: module database.c", sql, sqlite3_errmsg(db));
        log_event(EVENT_ERROR, text_out);
        exit(EXIT_FAILURE);
    }

    sqlite3_finalize(stmt);
}

void update_db_char_stats(int connection){

    /** public function - see header */

    int rc;
    sqlite3_stmt *stmt;
    char text_out[1024]="";

    char sql[1024]="UPDATE CHARACTER_TABLE SET \
                    OVERALL_EXP=?, \
                    HARVEST_EXP=? \
                    WHERE CHAR_ID=?";

    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    sqlite3_bind_int(stmt, 1, clients.client[connection]->overall_exp);
    sqlite3_bind_int(stmt, 2, clients.client[connection]->harvest_exp);

    sqlite3_bind_int(stmt, 3, clients.client[connection]->character_id);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        sprintf(text_out, "Error %s executing '%s' in function update_db_char_stats: module database.c", sql, sqlite3_errmsg(db));
        log_event(EVENT_ERROR, text_out);
        exit(EXIT_FAILURE);
    }

    sqlite3_finalize(stmt);
}

void update_db_char_last_in_game(int connection){

    /** public function - see header */

    int rc;
    sqlite3_stmt *stmt;
    char text_out[1024]="";

    char sql[1024]="UPDATE CHARACTER_TABLE SET LAST_IN_GAME=? WHERE CHAR_ID=?;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    sqlite3_bind_int(stmt, 1, (int) clients.client[connection]->last_in_game);
    sqlite3_bind_int(stmt, 2, clients.client[connection]->character_id);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        sprintf(text_out, "Error %s executing '%s' in function update_char_last_in_game: module database.c", sql, sqlite3_errmsg(db));
        log_event(EVENT_ERROR, text_out);
        exit(EXIT_FAILURE);
    }

    sqlite3_finalize(stmt);
}

void update_db_char_time_played(int connection){

    /** public function - see header */

    int rc;
    sqlite3_stmt *stmt;
    char text_out[1024]="";

    char sql[1024]="UPDATE CHARACTER_TABLE SET TIME_PLAYED=? WHERE CHAR_ID=?;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    sqlite3_bind_int(stmt, 1, (int) clients.client[connection]->time_played);
    sqlite3_bind_int(stmt, 2, clients.client[connection]->character_id);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        sprintf(text_out, "Error %s executing '%s' in function update_db_char_time_played: module database.c", sql, sqlite3_errmsg(db));
        log_event(EVENT_ERROR, text_out);
         exit(EXIT_FAILURE);
    }

    sqlite3_finalize(stmt);
}

void update_db_char_channels(int connection){

    /** public function - see header */

    int rc;
    sqlite3_stmt *stmt;
    char text_out[1024]="";

    char sql[1024]="UPDATE CHARACTER_TABLE SET ACTIVE_CHAN=?, CHAN_0=?, CHAN_1=?, CHAN_2=? WHERE CHAR_ID=?;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    sqlite3_bind_int(stmt, 1, clients.client[connection]->active_chan);
    sqlite3_bind_int(stmt, 2, clients.client[connection]->chan[0]);
    sqlite3_bind_int(stmt, 3, clients.client[connection]->chan[1]);
    sqlite3_bind_int(stmt, 4, clients.client[connection]->chan[2]);
    sqlite3_bind_int(stmt, 5, clients.client[connection]->character_id);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        sprintf(text_out, "Error %s executing '%s' in function update_db_char_channels: module database.c", sql, sqlite3_errmsg(db));
        log_event(EVENT_ERROR, text_out);
        exit(EXIT_FAILURE);
    }

    sqlite3_finalize(stmt);
}

void update_db_char_inventory(int connection){

    int i=0;
    int rc;
    sqlite3_stmt *stmt;
    char * sErrMsg = 0;
    char text_out[1024]="";

    char sql[1024]="UPDATE INVENTORY_TABLE SET IMAGE_ID=?, AMOUNT=? WHERE CHAR_ID=? AND SLOT=?";

    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &sErrMsg);

    for(i=0; i<MAX_INVENTORY_SLOTS; i++){

        sqlite3_bind_int(stmt, 1, clients.client[connection]->client_inventory[i].image_id);
        sqlite3_bind_int(stmt, 2, clients.client[connection]->client_inventory[i].amount);
        sqlite3_bind_int(stmt, 3, clients.client[connection]->character_id);
        sqlite3_bind_int(stmt, 4, i);

        rc = sqlite3_step(stmt);

        if (rc != SQLITE_DONE) {
            sprintf(text_out, "Error %s executing '%s' in function update_db_char_inventory: module database.c", sql, sqlite3_errmsg(db));
            log_event(EVENT_ERROR, text_out);
            exit(EXIT_FAILURE);
        }

        sqlite3_clear_bindings(stmt);
        sqlite3_reset(stmt);

    }

    sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &sErrMsg);
    sqlite3_finalize(stmt);
}

void update_db_char_slot(int connection, int slot){

    int rc;
    sqlite3_stmt *stmt;
    char text_out[1024]="";

    char sql[1024]="UPDATE INVENTORY_TABLE SET IMAGE_ID=?, AMOUNT=? WHERE CHAR_ID=? AND SLOT=?";

    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    sqlite3_bind_int(stmt, 1, clients.client[connection]->client_inventory[slot].image_id);
    sqlite3_bind_int(stmt, 2, clients.client[connection]->client_inventory[slot].amount);
    sqlite3_bind_int(stmt, 3, clients.client[connection]->character_id);
    sqlite3_bind_int(stmt, 4, slot);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
            sprintf(text_out, "Error %s executing '%s' in function update_db_char_inventory: module database.c", sql, sqlite3_errmsg(db));
            log_event(EVENT_ERROR, text_out);
            exit(EXIT_FAILURE);
        }

    sqlite3_finalize(stmt);
}
