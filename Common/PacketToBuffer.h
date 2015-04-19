#pragma once

#include <vector>
#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////
// declare all handled Packets
///////////////////////////////////////////////////////////////////////////////
// Common packets
struct RawText;

// Client to server packets
struct MoveTo;
struct PMMessage;
struct SitDown;
struct GetPlayerInfo;
struct SendMeMyActors;
struct SendOpeningScreen;
struct SendVersion;
struct Heartbeat;
struct UseObject;
struct LookAtInventoryItem;
struct MoveInventoryItem;
struct Harvest;
struct DropItem;
struct PickUpItem;
struct InspectBag;
struct LookAtMapObject;
struct PingResponse;
struct SetActiveChannel;
struct LogIn;
struct CreateChar;
struct GetDate;
struct GetTime;
struct ServerStats;

extern void packetToBuffer(std::vector<uint8_t> &tgt, const MoveTo &src);
extern void packetToBuffer(std::vector<uint8_t> & tgt, const PMMessage & src);
extern void packetToBuffer(std::vector<uint8_t> &tgt,const SitDown &src);
extern void packetToBuffer(std::vector<uint8_t> &tgt,const GetPlayerInfo &src);
extern void packetToBuffer(std::vector<uint8_t> &tgt,const SendMeMyActors &src);
extern void packetToBuffer(std::vector<uint8_t> &/*src*/,const SendOpeningScreen &/*tgt*/);
extern void packetToBuffer(std::vector<uint8_t> &/*src*/,const SendVersion &/*tgt*/);
extern void packetToBuffer(std::vector<uint8_t> &/*src*/,const Heartbeat &/*tgt*/);
extern void packetToBuffer(std::vector<uint8_t> &tgt,const UseObject &src);
extern void packetToBuffer(std::vector<uint8_t> &/*src*/,const LookAtInventoryItem &/*tgt*/);
extern void packetToBuffer(std::vector<uint8_t> &/*src*/,const MoveInventoryItem &/*tgt*/);
extern void packetToBuffer(std::vector<uint8_t> &tgt,const Harvest &src);
extern void packetToBuffer(std::vector<uint8_t> &tgt,const DropItem &src);
extern void packetToBuffer(std::vector<uint8_t> &stream,const PickUpItem &src);
extern void packetToBuffer(std::vector<uint8_t> &tgt,const InspectBag &src);
extern void packetToBuffer(std::vector<uint8_t> &tgt,const LookAtMapObject &src);
extern void packetToBuffer(std::vector<uint8_t> &tgt,const PingResponse &src);
extern void packetToBuffer(std::vector<uint8_t> &tgt,const SetActiveChannel &src);
extern void packetToBuffer(std::vector<uint8_t> &tgt,const LogIn &src);
extern void packetToBuffer(std::vector<uint8_t> &tgt,const CreateChar &src);
extern void packetToBuffer(std::vector<uint8_t> &tgt,const GetDate &src);
extern void packetToBuffer(std::vector<uint8_t> &tgt,const GetTime &src);
extern void packetToBuffer(std::vector<uint8_t> &tgt,const ServerStats &src);

// Server to client packets

struct AddNewEnhancedActor;
struct RemoveActor;
struct AddActor;
struct NewMinute;
struct LogInOk;
struct LogInNotOk;
struct YouDontExist;
struct YouAre;
struct CreateCharOk;
struct CreateCharNotOk;
struct CharacterInventory;
struct GetActiveChannels;
struct CharacterAttributesPacket;
struct ChangeMap;

extern void packetToBuffer(std::vector<uint8_t> &tgt, const struct ServerRawText &src);
extern void packetToBuffer(std::vector<uint8_t> & tgt, const AddNewEnhancedActor & src);
extern void packetToBuffer(std::vector<uint8_t> &tgt,const RemoveActor &src);
extern void packetToBuffer(std::vector<uint8_t> &tgt,const AddActor &src);
extern void packetToBuffer(std::vector<uint8_t> &tgt,const NewMinute &src);
extern void packetToBuffer(std::vector<uint8_t> &/*src*/,const LogInOk &/*tgt*/);
extern void packetToBuffer(std::vector<uint8_t> &/*src*/,const LogInNotOk &/*tgt*/);
extern void packetToBuffer(std::vector<uint8_t> &/*src*/,const YouDontExist &/*tgt*/);
extern void packetToBuffer(std::vector<uint8_t> &tgt,const YouAre &src);
extern void packetToBuffer(std::vector<uint8_t> &/*src*/,const CreateCharOk &/*tgt*/);
extern void packetToBuffer(std::vector<uint8_t> &/*src*/,const CreateCharNotOk &/*tgt*/);
extern void packetToBuffer(std::vector<uint8_t> &tgt,const CharacterInventory &src);
extern void packetToBuffer(std::vector<uint8_t> &tgt,const GetActiveChannels &src);
extern void packetToBuffer(std::vector<uint8_t> &stream,const CharacterAttributesPacket &src);
extern void packetToBuffer(std::vector<uint8_t> &tgt,const ChangeMap &src);
