#include "ProcessPacket.h"
#include "ClientPackets.h"

#include <string>
#include <cstring>
#include <cassert>

// Don't expose internal packet serialization functions to the rest of the code - enclose them in anonymous namespace
namespace {
struct SimpleROStream {
    const uint8_t *m_data;
    uint32_t m_size,m_index;
    SimpleROStream(const uint8_t *d,uint32_t sz) : m_data(d),m_size(sz), m_index(0) {}

    template<class T> bool get(T &tgt) {
        if(m_index + sizeof(T)>m_size)
            return false;
        tgt = *(T *)(m_data + m_index);
        m_index += sizeof(T);
        return true;
    }
    std::string getString() {
        int availableLength = 0;
        for( ; availableLength+m_index < m_size; ++availableLength)
            if(m_data[m_index+availableLength]==0)
                break;
        std::string res((const char *)m_data+m_index,availableLength);
        m_index+=availableLength;
        uint8_t eos;
        get(eos); // end of string 0
        return res;
    }
    uint32_t bytesLeft() const { return m_size-m_index; }
};

bool fromBuffer(ClientRawText & tgt, SimpleROStream & src)
{
    bool ok = true;
    tgt.m_contents = src.getString();
    return ok & !tgt.m_contents.empty() ;
}

bool fromBuffer(MoveTo &tgt, SimpleROStream & src)
{
    if(src.bytesLeft()<4) {
        return false;
    }
    bool ok = true;
    ok &= src.get(tgt.m_dest_x);
    ok &= src.get(tgt.m_dest_y);
    return ok;

}
bool fromBuffer(PMMessage &tgt, SimpleROStream &src)
{
    std::string data = src.getString();
    int first_space_location = data.find(' ');
    if(first_space_location==std::string::npos) // no space character found - wrong PM format ??
        return false;
    tgt.m_target = data.substr(0,first_space_location);
    tgt.m_message = data.substr(first_space_location+1);
    return true;
}

bool fromBuffer(SitDown &tgt, SimpleROStream &src)
{
    bool ok = true;
    ok &= src.get(tgt.m_sit);
    return ok;
}

bool fromBuffer(GetPlayerInfo &tgt, SimpleROStream &src)
{
    bool ok = true;
    src.get(tgt.m_id);
    assert(tgt.m_id<0xFFFF);
    return ok;

}

bool fromBuffer(SendMeMyActors &tgt, SimpleROStream &/*src*/)
{
    return true;
}

bool fromBuffer(SendOpeningScreen &tgt, SimpleROStream &/*src*/)
{
    return true;
}

bool fromBuffer(LookAtInventoryItem &tgt, SimpleROStream &src)
{
    bool ok = true;
    ok &= src.get(tgt.m_inventory_slot);
    return ok;
}

bool fromBuffer(MoveInventoryItem &tgt, SimpleROStream &src)
{
    bool ok = true;
    return ok;

}

bool fromBuffer(Harvest &tgt, SimpleROStream &src)
{
    bool ok = true;
    ok &= src.get(tgt.m_map_object_id);
    return ok;
}
bool fromBuffer(SendVersion &tgt, SimpleROStream &src)
{
    bool ok = true;
    ok &= src.get(tgt.version_a);
    ok &= src.get(tgt.version_b);
    ok &= src.get(tgt.major_ver);
    ok &= src.get(tgt.minor_ver);
    ok &= src.get(tgt.release_ver);
    ok &= src.get(tgt.patch_ver);
    ok &= src.get(tgt.host_a);
    ok &= src.get(tgt.host_b);
    ok &= src.get(tgt.host_c);
    ok &= src.get(tgt.host_d);
    ok &= src.get(tgt.port);
    return ok;

}
bool fromBuffer(Heartbeat &tgt, SimpleROStream &/*src*/)
{
    bool ok = true;
    return ok;
}

bool fromBuffer(UseObject &tgt, SimpleROStream &src)
{
    bool ok = true;
    ok &= src.get(tgt.object_id);
    ok &= src.get(tgt.object_inv_pos);
    return ok;
}
bool fromBuffer(DropItem &tgt, SimpleROStream &src)
{
    bool ok = true;
    ok &= src.get(tgt.m_inventory_slot);//dragged item
    ok &= src.get(tgt.m_amount);
    return ok;
}

bool fromBuffer(PickUpItem &tgt, SimpleROStream &src)
{
    return false;
}

bool fromBuffer(InspectBag &tgt, SimpleROStream &src)
{
    return false;
}

bool fromBuffer(LookAtMapObject &tgt, SimpleROStream &src)
{
    return false;
}
bool fromBuffer(PingResponse &tgt, SimpleROStream &src)
{
    return true;
}

bool fromBuffer(SetActiveChannel &tgt, SimpleROStream &src)
{
    return false;
}

bool fromBuffer(LogIn &tgt, SimpleROStream &src)
{
    std::string contents = src.getString();
    int first_space = contents.find(' ');
    if(first_space==contents.npos)
        return false;

    tgt.m_username = contents.substr(0,first_space);
    tgt.m_password = contents.substr(first_space+1);
    return !tgt.m_username.empty() && !tgt.m_password.empty();
}
// Type 2 - female
bool fromBuffer(CreateChar &tgt, SimpleROStream &src)
{
    std::string data = src.getString();
    int end_of_name=data.find(' ');
    if(end_of_name==std::string::npos) // no space character found
        return -1;

    tgt.m_name = data.substr(0,end_of_name);
    tgt.m_password = data.substr(end_of_name+1);
    bool ok=true;
    ok &= src.get(tgt.skin);
    ok &= src.get(tgt.hair);
    ok &= src.get(tgt.shirt);
    ok &= src.get(tgt.pants);
    ok &= src.get(tgt.boots);
    ok &= src.get(tgt.type);
    ok &= src.get(tgt.head);
    // TODO: this below is 1.9.4 feature, not essential ?
    src.get(tgt.eyes);
    return ok;
}
bool fromBuffer(GetDate &tgt, SimpleROStream &src)
{
    bool ok = true;
    return ok;
}

bool fromBuffer(GetTime &tgt, SimpleROStream &src)
{
    bool ok = true;
    return ok;
}

bool fromBuffer(ServerStats &tgt, SimpleROStream &src)
{
    bool ok = true;
    return ok;
}
bool fromBuffer(AttackSomeone &tgt, SimpleROStream &src)
{
    bool ok = true;
    ok &= src.get(tgt.m_attacked_entity_id);
    return ok;
}

} // end of anonymous namespace

#define SERIALIZE_PACKET_CASE(classname) case classname::Cmd: {\
    classname res;\
    parse_ok = fromBuffer(res, src);\
    if(parse_ok)\
        handler->handle(res);\
    break;\
}

bool processClientPacket(uint8_t *data, uint32_t size, ClientPacketHandler *handler) {
    SimpleROStream src(data,size);

    if(src.bytesLeft()<3)
        return false;

    uint8_t packet_code;
    uint16_t length;
    src.get(packet_code);
    src.get(length);
    if((src.bytesLeft()+1)<length)
        return false;
    bool parse_ok = false;
    switch(packet_code) {
    SERIALIZE_PACKET_CASE(ClientRawText);
    SERIALIZE_PACKET_CASE(MoveTo);
    SERIALIZE_PACKET_CASE(PMMessage);
    SERIALIZE_PACKET_CASE(SitDown);
    SERIALIZE_PACKET_CASE(GetPlayerInfo);
    SERIALIZE_PACKET_CASE(SendMeMyActors);
    SERIALIZE_PACKET_CASE(SendOpeningScreen);
    SERIALIZE_PACKET_CASE(SendVersion);
    SERIALIZE_PACKET_CASE(Heartbeat);
    SERIALIZE_PACKET_CASE(UseObject);
    SERIALIZE_PACKET_CASE(LookAtInventoryItem);
    SERIALIZE_PACKET_CASE(MoveInventoryItem);
    SERIALIZE_PACKET_CASE(Harvest);
    SERIALIZE_PACKET_CASE(DropItem);
    SERIALIZE_PACKET_CASE(PickUpItem);
    SERIALIZE_PACKET_CASE(InspectBag);
    SERIALIZE_PACKET_CASE(LookAtMapObject);
    SERIALIZE_PACKET_CASE(AttackSomeone);
    SERIALIZE_PACKET_CASE(PingResponse);
    SERIALIZE_PACKET_CASE(SetActiveChannel);
    SERIALIZE_PACKET_CASE(LogIn);
    SERIALIZE_PACKET_CASE(CreateChar);
    SERIALIZE_PACKET_CASE(GetDate);
    SERIALIZE_PACKET_CASE(GetTime);
    SERIALIZE_PACKET_CASE(ServerStats);
    default: {
        char buf[64];
        snprintf(buf,64,"Unkown packet [%d]",packet_code);
        handler->handleError(buf);
        assert(false);
        return false;
    }
    }
    if(!parse_ok) {
        assert(false);
        return false;
    }
    return true;
}
#undef SERIALIZE_PACKET_CASE
