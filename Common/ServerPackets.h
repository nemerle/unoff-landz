#pragma once

#include "character_attribute.h"
#include "client_inventory_data.h"
#include "enhanced_actor_data.h"

#include <stdint.h>

enum ServerToClientCommands { // server to client protocol
    RAW_TEXT=0,
    SC_ADD_ACTOR=2,
    YOU_ARE=3,
    SYNC_CLOCK=4,
    NEW_MINUTE=5,
    REMOVE_ACTOR=6,
    CHANGE_MAP=7,
//    COMBAT_MODE=8,
    PONG = 11,
    PLAY_SOUND = 14,
    HERE_YOUR_STATS=18,
    HERE_YOUR_INVENTORY=19,
    INVENTORY_ITEM_TEXT=20,
    GET_NEW_INVENTORY_ITEM=21,
    REMOVE_ITEM_FROM_INVENTORY =22,
    HERE_YOUR_GROUND_ITEMS=23,
    GET_NEW_GROUND_ITEM=24,
    REMOVE_ITEM_FROM_GROUND=25,
    S_CLOSE_BAG=26,
    GET_NEW_BAG=27,
    DESTROY_BAG=29,
    ADD_NEW_ENHANCED_ACTOR=51,
    GET_ACTIVE_CHANNELS=71,
    YOU_DONT_EXIST=249,
    LOG_IN_OK=250,
    LOG_IN_NOT_OK=251,
    CREATE_CHAR_OK=252,
    CREATE_CHAR_NOT_OK=253,

};
struct ServerRawText  {
    friend class PacketSerializer;
    enum {Cmd = RAW_TEXT};

    uint8_t m_channel;
    std::string m_contents;
public:
    ServerRawText() : m_channel(0) {}

    ServerRawText(int channel,const char *contents) : m_channel(channel),m_contents(contents) {

    }
    uint8_t channel() const { return m_channel; }
    const std::string & contents() const { return m_contents; }
};
struct NewMinute  {
    enum { Cmd = NEW_MINUTE };
    uint16_t m_minute;
    NewMinute(uint16_t minute) : m_minute(minute) {
    }
};

struct LogInOk  {
    enum { Cmd = LOG_IN_OK };
};
struct LogInNotOk  {
    enum { Cmd = LOG_IN_NOT_OK };
};
struct YouDontExist  {
    enum { Cmd = YOU_DONT_EXIST };
};
struct YouAre  {
    enum { Cmd = YOU_ARE };
    uint16_t m_id;
    YouAre(uint32_t id) : m_id(id) {
    }
};
struct CreateCharOk   {
    enum { Cmd = CREATE_CHAR_OK };
};
struct CreateCharNotOk   {
    enum { Cmd = CREATE_CHAR_NOT_OK };
};


/** \brief Represents the remove actor packet to client
    \sa broadcast_remove_actor_packet function
*/
struct RemoveActor  {
    enum { Cmd = REMOVE_ACTOR };
    uint16_t m_id;
    RemoveActor(uint32_t id) : m_id(id) {
    }
};

/** \brief represents the add_actor_packet
    \sa broadcast_actor_packet function
*/

struct AddActor  {
    enum { Cmd = SC_ADD_ACTOR };
    uint16_t m_id;
    uint8_t m_move;
public:
    AddActor(uint32_t id,uint8_t move) : m_id(id), m_move(move) {}
};

/** \brief Represents the add_new_enhanced_actor_packet

    \sa broadcast_add_new_enhanced_actor_packet
*/

struct AddNewEnhancedActor  {
    enum { Cmd = ADD_NEW_ENHANCED_ACTOR };
    uint32_t m_id;
    const EnhancedActorData &m_client;
    AddNewEnhancedActor(uint32_t id,const EnhancedActorData &cl) : m_id(id),m_client(cl) {}
};

struct ChangeMap  {
    enum { Cmd = CHANGE_MAP };

    const char *m_map_name;
    ChangeMap(const char *nm) : m_map_name(nm) {
    }
};
struct GetActiveChannels {
    enum { Cmd = GET_ACTIVE_CHANNELS };
    uint8_t m_active_channel;
    const std::vector<int> &m_slots;
    GetActiveChannels(uint8_t ac,const std::vector<int> &sl) : m_active_channel(ac),m_slots(sl) {
    }
};

struct CharacterInventory  {
    enum { Cmd = HERE_YOUR_INVENTORY };
    const std::vector<client_inventory_type> &m_items;
public:
    CharacterInventory(const std::vector<client_inventory_type> &items) : m_items(items) {

    }
};

struct CharacterAttributesPacket  {
    enum { Cmd = HERE_YOUR_STATS };

    const CharacterAttributes & cl;
    character_attribute<uint16_t> m_emu;

    CharacterAttributesPacket(const CharacterAttributes & for_client,character_attribute<uint16_t> emu)
        : cl(for_client), m_emu(emu) {
    }
};
