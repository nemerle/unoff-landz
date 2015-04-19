set(SRC
ProcessPacket.cpp
PacketToBuffer.cpp
)
set(INCLUDE
ProcessPacket.h
PacketToBuffer.h
ClientPackets.h
CommonPackets.h
ServerPackets.h

character_attribute.h
)
add_library(unoff_packeteer STATIC ${SRC} ${INCLUDE})
