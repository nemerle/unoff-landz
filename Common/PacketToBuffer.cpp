#include "PacketToBuffer.h"

#include "ServerPackets.h"
#include "ClientPackets.h"
#include "character_attribute.h"

#include <string.h>
// Don't expose internal packet serialization functions to the rest of the code
namespace {
template<class T>
void put(std::vector<uint8_t> &tgt,const T &val) {
    tgt.insert(tgt.end(),(const char *)&val,((const char *)&val)+sizeof(T));
}
template<>
void put(std::vector<uint8_t> &tgt,const std::string &val) {
    tgt.insert(tgt.end(),val.begin(),val.end());
    tgt.push_back(0); // end of string marker
}
// TODO: remove all unknown-size buffer sends

void put(std::vector<uint8_t> &tgt,const char *val) {
    tgt.insert(tgt.end(),val,val+strlen(val));
    tgt.push_back(0); // end of string marker
}
template<typename T>
static void putAttribute(std::vector<uint8_t> &tgt,const character_attribute<T> &attr) {
    put(tgt,attr.current);
    put(tgt,attr.maximum);
}

void toBuffer(const ServerRawText & src, std::vector<uint8_t> &tgt)
{
    put(tgt,src.m_channel  );
    put(tgt,src.m_contents  );
}

void toBuffer(const RemoveActor &src, std::vector<uint8_t> &tgt) {
    put(tgt,src.m_id);
}

void toBuffer(const AddNewEnhancedActor &src, std::vector<uint8_t> &tgt)
{
    const EnhancedActorData &data(src.m_client);

    put(tgt,(uint16_t)src.m_id);
    put(tgt,data.x); //x_axis
    put(tgt,data.y); //y_axis
    put(tgt,data.buffs);
    put(tgt,data.rotation); //rotation

    put(tgt,(uint8_t)data.char_type);
    put(tgt,(uint8_t)0); // unused on client ?
    put(tgt,(uint8_t)data.skin_type);
    put(tgt,(uint8_t)data.hair_type);
    put(tgt,(uint8_t)data.shirt_type);
    put(tgt,(uint8_t)data.pants_type);
    put(tgt,(uint8_t)data.boots_type);

    put(tgt,(uint8_t)data.head_type);
    put(tgt,(uint8_t)data.shield_type);
    put(tgt,(uint8_t)data.weapon_type);
    put(tgt,(uint8_t)data.cape_type);
    put(tgt,(uint8_t)data.helmet_type);
    put(tgt,(uint8_t)data.frame);

    put(tgt,(uint16_t)data.max_health);
    put(tgt,(uint16_t)data.current_health);

    put(tgt,(uint8_t)1); // special: HUMAN / NPC
    put(tgt,data.char_name);
    put(tgt,(uint16_t)0x4000);   // actor scale
    put(tgt,(uint8_t)255); // riding none=255  brown horse=200
    put(tgt,(uint8_t)64);  // neck_attachment none=64
    put(tgt,(uint8_t)0);  // eyes

}

void toBuffer(const CharacterInventory & src, std::vector<uint8_t> &tgt)
{
    put<uint8_t>(tgt,src.m_items.size());
    int pos=0;
    for(const client_inventory_type &item : src.m_items) {
        put<uint16_t>(tgt,item.image_id);  // image_id of item
        put<uint32_t>(tgt,item.amount);    // amount (when zero nothing is shown in inventory)
        put<uint8_t>(tgt,pos);       // inventory pos (starts at 0) TODO: use slot here ?
        put<uint8_t>(tgt,0); // flags
        // if UID enabled
        // put<uint16_t>(tgt,item.uid);
        pos++;
    }
}

void toBuffer(const CharacterAttributesPacket & src, std::vector<uint8_t> & tgt)
{
    const CharacterAttributes &attr(src.cl);

    putAttribute(tgt,attr.physique);
    putAttribute(tgt,attr.coordination);
    putAttribute(tgt,attr.reasoning);
    putAttribute(tgt,attr.will);
    putAttribute(tgt,attr.instinct);
    putAttribute(tgt,attr.vitality);

    putAttribute(tgt,attr.human);
    putAttribute(tgt,attr.animal);
    putAttribute(tgt,attr.vegetal);
    putAttribute(tgt,attr.inorganic);
    putAttribute(tgt,attr.artificial);
    putAttribute(tgt,attr.magic);

    putAttribute(tgt,attr.manufacturing);
    putAttribute(tgt,attr.harvest);
    putAttribute(tgt,attr.alchemy);
    putAttribute(tgt,attr.overall);
    putAttribute(tgt,attr.attack);
    putAttribute(tgt,attr.defence);

    putAttribute(tgt,attr.magic_skill);
    putAttribute(tgt,attr.potion);

    putAttribute(tgt,src.m_emu); // amount of emu in inventory +  max emu that can be held in inventory

    putAttribute(tgt,attr.material);

    putAttribute(tgt,attr.ethereal);

    put<uint16_t>(tgt,attr.food_lvl);

    putAttribute(tgt,attr.manufacture_exp);
    putAttribute(tgt,attr.harvest_exp);
    putAttribute(tgt,attr.alchemy_exp);
    putAttribute(tgt,attr.overall_exp);
    putAttribute(tgt,attr.attack_exp);
    putAttribute(tgt,attr.defence_exp);
    putAttribute(tgt,attr.magic_exp);
    putAttribute(tgt,attr.potion_exp);

    putAttribute(tgt,attr.summoning);
    putAttribute(tgt,attr.summoning_exp);
    putAttribute(tgt,attr.crafting);
    putAttribute(tgt,attr.crafting_exp);
    putAttribute(tgt,attr.engineering);
    putAttribute(tgt,attr.engineering_exp);
    putAttribute(tgt,attr.tailoring);
    putAttribute(tgt,attr.tailoring_exp);
    putAttribute(tgt,attr.ranging);
    putAttribute(tgt,attr.ranging_exp);


    put<uint16_t>(tgt,attr.book_id); //reasearcing any book ? will be non-zero ?
    put<uint16_t>(tgt,attr.elapsed_book_time); //research_total how much research is 'done'
    put<uint16_t>(tgt,attr.max_book_time); // total amount needed to finish research
}
void toBuffer(const AddActor &src, std::vector<uint8_t> &tgt) {
    put(tgt,src.m_id);
    put(tgt,src.m_move);
}

void toBuffer(const NewMinute &src, std::vector<uint8_t> &tgt) {
    put(tgt,src.m_minute);
}

void toBuffer(const YouAre &src, std::vector<uint8_t> &tgt) {
    put(tgt,src.m_id);
}
void toBuffer(const GetActiveChannels &src, std::vector<uint8_t> &tgt) {
    put(tgt,src.m_active_channel);
    for(int i=0; i<src.m_slots.size(); i++){
        put<uint32_t>(tgt,src.m_slots[i]);
    }
}

void toBuffer(const ChangeMap &src, std::vector<uint8_t> &tgt) {
    put(tgt,src.m_map_name);
}

///////////////////////////////////////////////////////////////////////////////
// Dummy functions for packets without contents
///////////////////////////////////////////////////////////////////////////////
void toBuffer(const LogInOk &, std::vector<uint8_t> &) {}
void toBuffer(const LogInNotOk &, std::vector<uint8_t> &) {}
void toBuffer(const YouDontExist &, std::vector<uint8_t> &) {}
void toBuffer(const CreateCharOk &, std::vector<uint8_t> &) {}
void toBuffer(const CreateCharNotOk &, std::vector<uint8_t> &) {}


} // end of anonymous namespace

#define TO_BUFFER(classname)\
void packetToBuffer(std::vector<uint8_t> &target, const classname &src) {\
    put<uint8_t>(target,classname::Cmd);\
    put<uint16_t>(target,0); /* size placeholder*/\
    toBuffer(src,target);\
    *((uint16_t *)(target.data()+1)) = target.size()-2;\
}
/*
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

*/
TO_BUFFER(ServerRawText);

TO_BUFFER(AddActor);
TO_BUFFER(YouAre);
//TO_BUFFER(SyncClock);
TO_BUFFER(NewMinute);
TO_BUFFER(RemoveActor);
TO_BUFFER(ChangeMap);
//TO_BUFFER(CombatMode);
//TO_BUFFER(Pong);
//TO_BUFFER(PlaySound);
TO_BUFFER(CharacterAttributesPacket);
TO_BUFFER(CharacterInventory);
//TO_BUFFER(InventoryItemText);
//TO_BUFFER(GetNewInventoryItem);
//TO_BUFFER(RemoveItemFromInventory);
//TO_BUFFER(HereYourGroundItems);
//TO_BUFFER(GetNewGroundItem);
//TO_BUFFER(RemoveItemFromGround);
//TO_BUFFER(CloseBag);
//TO_BUFFER(GetNewBag);
//TO_BUFFER(DestroyBag);
TO_BUFFER(AddNewEnhancedActor);
TO_BUFFER(GetActiveChannels);
TO_BUFFER(YouDontExist);
TO_BUFFER(LogInOk);
TO_BUFFER(LogInNotOk);
TO_BUFFER(CreateCharOk);
TO_BUFFER(CreateCharNotOk);


