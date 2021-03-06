#include "Packet.h"
#include <string.h>

Packet::Packet()
{
}

Packet::Packet(PacketType type, void * data, int size) : type(type)
{
	totalSize = sizeof(int) * 3 + size;
	int offset = (totalSize < MAX_PACKET_SIZE) ? MAX_PACKET_SIZE - totalSize : MAX_PACKET_SIZE - (totalSize % MAX_PACKET_SIZE);
	totalSize += offset;
	int numPackets = totalSize / MAX_PACKET_SIZE;
	outData = new char[totalSize];

	char* pointer = outData;
	// Header: numPackets + type + offset
	memcpy(pointer, &numPackets, sizeof(numPackets));
	pointer += sizeof(numPackets);
	memcpy(pointer, &type, sizeof(type));
	pointer += sizeof(type);
	memcpy(pointer, &offset, sizeof(offset));
	pointer += sizeof(offset);
	// Data
	if(data)
		memcpy(pointer, data, size);
}

Packet::~Packet()
{
	if (outData)
		delete[] outData;
	if (inData)
		delete[] inData;
}

Packet * Packet::Read(TCPsocket * socket)
{
	Packet* packet = nullptr;
	bool initialized = false;
	char* tmp = new char[MAX_PACKET_SIZE];
	int packCount = 0;
	int numPackets = 0;
	int offset = 0;
	char* pointer = nullptr;
	int dataRecv;
	do
	{
		dataRecv = SDLNet_TCP_Recv(*socket, tmp, MAX_PACKET_SIZE);
		if (dataRecv > 0)
		{
			if (!initialized)
			{
				packet = new Packet();
				memcpy(&numPackets, tmp, sizeof(int));
				memcpy(&packet->type, tmp + sizeof(int), sizeof(int));
				memcpy(&offset, tmp + (sizeof(int) * 2), sizeof(int));
				packet->inData = new char[(numPackets * MAX_PACKET_SIZE) - offset];
				pointer = packet->inData;
				packet->totalSize = (numPackets * MAX_PACKET_SIZE) - offset;
				initialized = true;
			}
			packCount++;

			if (packCount < numPackets)
			{
				memcpy(pointer, tmp, MAX_PACKET_SIZE);
				pointer += MAX_PACKET_SIZE;
			}
			else
			{
				memcpy(pointer, tmp, MAX_PACKET_SIZE - offset); // Last packet. Do not copy the offset data
			}
		}
	} while (packCount < numPackets && dataRecv > 0);

	if (tmp)
		delete[] tmp;

	return packet;
}

char * Packet::PrepareToSend(int & length)
{
	length = totalSize;
	return outData;
}

PacketType Packet::GetType() const
{
	return type;
}

char * Packet::GetData(int & dataSize) const
{
	if (inData != nullptr)
	{
		dataSize = totalSize - PACKET_HEADER_SIZE;
		return (inData + PACKET_HEADER_SIZE);
	}
	return nullptr;
}
