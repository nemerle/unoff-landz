#pragma once

#include <stdint.h>
#include <string>

enum ClientToServer { // client to server protocol
    CLIENT_RAW_TEXT=0,
    MOVE_TO=1,
    SEND_PM=2,
    GET_PLAYER_INFO=5,
    SIT_DOWN=7,
    SEND_ME_MY_ACTORS=8,
    SEND_OPENING_SCREEN=9,
    SEND_VERSION=10,
    HEARTBEAT=14,
    USE_OBJECT=16,
    LOOK_AT_INVENTORY_ITEM=19,
    MOVE_INVENTORY_ITEM=20,
    HARVEST=21,
    DROP_ITEM=22,
    PICK_UP_ITEM=23,
    INSPECT_BAG=25,
    LOOK_AT_MAP_OBJECT=27,
    ATTACK_SOMEONE = 40,
    SEND_PARTIAL_STATS=49,
    PING_RESPONSE=60,
    SET_ACTIVE_CHANNEL=61,
    LOG_IN=140,
    CREATE_CHAR=141,
    GET_DATE=230,
    GET_TIME=231,
    SERVER_STATS=232
};
struct ClientRawText  {
    friend class PacketSerializer;
    enum {Cmd = CLIENT_RAW_TEXT};

    std::string m_contents;
public:
    ClientRawText(const char *contents="") : m_contents(contents) {
    }
    const std::string & contents() const { return m_contents; }
};
struct MoveTo  {
    static const uint8_t Cmd = MOVE_TO;
    uint16_t m_dest_x;
    uint16_t m_dest_y;
    MoveTo() {}
    MoveTo(uint16_t x,uint16_t y) : m_dest_x(x),m_dest_y(y) {
    }
};

struct PMMessage  {
    static const uint8_t Cmd = SEND_PM;
    std::string m_target;
    std::string m_message;
    PMMessage() {}

    const std::string &targetName() const {return m_target; }
    const std::string &messageContents() const {return m_message; }
};

struct SitDown  {
    static const uint8_t Cmd = SIT_DOWN;
    uint8_t m_sit;
};
struct GetPlayerInfo  {
    static const uint8_t Cmd = GET_PLAYER_INFO;
    uint32_t m_id;
};
struct SendMeMyActors  {
    static const uint8_t Cmd = SEND_ME_MY_ACTORS;
};
struct SendOpeningScreen  {
    static const uint8_t Cmd = SEND_OPENING_SCREEN;
};

struct SendVersion  {
    static const uint8_t Cmd = SEND_VERSION;
    uint16_t version_a,version_b;
    uint8_t major_ver,minor_ver,release_ver,patch_ver;
    uint8_t host_a,host_b,host_c,host_d;
    uint16_t port;
};
struct Heartbeat  {
    static const uint8_t Cmd = HEARTBEAT;
};

struct UseObject  {
    static const uint8_t Cmd = USE_OBJECT;
    uint32_t object_id;
    uint32_t object_inv_pos;
};
struct LookAtInventoryItem  {
    static const uint8_t Cmd = LOOK_AT_INVENTORY_ITEM;
    uint8_t m_inventory_slot;

};
struct MoveInventoryItem  {
    static const uint8_t Cmd = MOVE_INVENTORY_ITEM;
    uint8_t m_src_inventory_slot;
    uint8_t m_tgt_inventory_slot;
};

struct Harvest  {
    static const uint8_t Cmd = HARVEST;
    uint16_t m_map_object_id;
};

struct DropItem  {
    static const uint8_t Cmd = DROP_ITEM;
    uint8_t m_inventory_slot;
    uint32_t m_amount;
};


struct PickUpItem  {
    static const uint8_t Cmd = PICK_UP_ITEM;
    uint8_t m_ground_slot;
    uint32_t m_amount;
};
struct InspectBag  {
    static const uint8_t Cmd = INSPECT_BAG;
    uint8_t m_bag_id;
};
struct LookAtMapObject  {
    static const uint8_t Cmd = LOOK_AT_MAP_OBJECT;
    uint32_t m_map_object_id;
};

struct PingResponse  {
    static const uint8_t Cmd = PING_RESPONSE;
};

struct SetActiveChannel  {
    static const uint8_t Cmd = SET_ACTIVE_CHANNEL;
    uint8_t m_channel;
};
struct LogIn  {
    static const uint8_t Cmd = LOG_IN;
    std::string m_username;
    std::string m_password;

    bool isValid() const { return !(m_username.empty()&&m_password.empty()); }
};

struct CreateChar  {
    static const uint8_t Cmd = CREATE_CHAR;
    std::string m_name;
    std::string m_password;
    uint8_t skin;
    uint8_t hair;
    uint8_t shirt;
    uint8_t pants;
    uint8_t boots;
    uint8_t type;
    uint8_t head;
    uint8_t eyes;
};

struct GetDate  {
    static const uint8_t Cmd = GET_DATE;
};
struct GetTime  {
    static const uint8_t Cmd = GET_TIME;
};
struct ServerStats  {
    static const uint8_t Cmd = SERVER_STATS;
};

struct AttackSomeone  {
    static const uint8_t Cmd = ATTACK_SOMEONE;
    uint32_t m_attacked_entity_id;
};
