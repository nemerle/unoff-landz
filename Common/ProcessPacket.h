#pragma once

#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////
// declare all handled Packets
///////////////////////////////////////////////////////////////////////////////

// Client to server packets
struct ClientRawText;
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
struct AttackSomeone;
struct PingResponse;
struct SetActiveChannel;
struct LogIn;
struct CreateChar;
struct GetDate;
struct GetTime;
struct ServerStats;

// Server to client packets
struct ServerRawText;

struct ClientPacketHandler {
    virtual void handle(const ClientRawText &)=0;
    virtual void handle(const MoveTo &) = 0;
    virtual void handle(const PMMessage &) = 0;
    virtual void handle(const SitDown &) = 0;
    virtual void handle(const GetPlayerInfo &) = 0;
    virtual void handle(const SendMeMyActors &) = 0;
    virtual void handle(const SendOpeningScreen &) = 0;
    virtual void handle(const SendVersion &) = 0;
    virtual void handle(const Heartbeat &) = 0;
    virtual void handle(const UseObject &) = 0;
    virtual void handle(const LookAtInventoryItem &) = 0;
    virtual void handle(const MoveInventoryItem &) = 0;
    virtual void handle(const Harvest &) = 0;
    virtual void handle(const DropItem &) = 0;
    virtual void handle(const PickUpItem &) = 0;
    virtual void handle(const InspectBag &) = 0;
    virtual void handle(const LookAtMapObject &) = 0;
    virtual void handle(const AttackSomeone &) = 0;
    virtual void handle(const PingResponse &) = 0;
    virtual void handle(const SetActiveChannel &) = 0;
    virtual void handle(const LogIn &) = 0;
    virtual void handle(const CreateChar &) = 0;
    virtual void handle(const GetDate &) = 0;
    virtual void handle(const GetTime &) = 0;
    virtual void handle(const ServerStats &) = 0;
    virtual void handleError(const char *msg) = 0;
};

struct ServerPacketHandler {
    virtual void handle(const ServerRawText &)=0;
};


// incoming client packets enter here.
extern bool processClientPacket(uint8_t *data, uint32_t size,ClientPacketHandler *handler);
extern bool processServerPacket(uint8_t *data, uint32_t size,ServerPacketHandler *handler);
