#include "BeeT_network.h"
#include "BeeT_NW_packet.h"
#include "BeeT_DBG_behaviortree.h"

#define BEET_DB_MAX_NUM_SOCKETS 3 //TODO: Change this

BeeT_network * BeeT_NW_Init(const char* ip, int port)
{
	BeeT_network* nw = (BeeT_network*)BEET_malloc(sizeof(BeeT_network));
	nw->port = port;
	nw->ip = ip;
	if (SDLNet_Init() < 0)
	{
		printf("%s\n", SDLNet_GetError());
		return NULL;
	}

	if (SDLNet_ResolveHost(&nw->ipAdr, strcmp(ip, "127.0.0.1") ? ip : "localhost", port) == -1)
	{
		printf("%s\n", SDLNet_GetError());
		return NULL;
	}

	nw->socketSet = SDLNet_AllocSocketSet(BEET_DB_MAX_NUM_SOCKETS);
	if (nw->socketSet == NULL)
	{
		printf("SDLNet create socket set: %s\n", SDLNet_GetError());
		return NULL;
	}

	nw->socketList = InitDequeue();

	for (int i = 0; i < BEET_DB_MAX_NUM_SOCKETS; i++)
	{
		BeeT_socket* sc = (BeeT_socket*)BEET_malloc(sizeof(BeeT_socket));
		sc->state = NOT_INIT;
		sc->socket = NULL;
		sc->bt = NULL;
		sc->sentDataSize = 0;
		dequeue_push_back(nw->socketList, sc);
	}

	return nw;
}

void BeeT_NW_Tick(BeeT_network* nw)
{
	SDLNet_CheckSockets(nw->socketSet, 0);
	node_deq* it = dequeue_head(nw->socketList);
	while (it != NULL)
	{
		BeeT_socket* sc = (BeeT_socket*)it->data;

		if (sc->state == NOT_INIT)
		{
			it = it->next;
			continue;
		}

		if (sc->state == OPEN)
		{
			TCPsocket tcpsock = SDLNet_TCP_Open(&nw->ipAdr);
			if (tcpsock == NULL)
			{
				// PRINT ERROR
			}
			sc->socket = tcpsock;
			SDLNet_TCP_AddSocket(nw->socketSet, sc->socket);
			sc->state = WAITING_CONNECTION_ACK;
		}

		if (sc->state == WAITING_CONNECTION_ACK)
		{
			if (SDLNet_SocketReady(sc->socket))
			{
				BeeT_packet* packet = (BeeT_packet*)BEET_malloc(sizeof(BeeT_packet));
				BEET_bool readRet = BeeT_packet_Read(packet, &sc->socket);
				if (readRet)
				{
					if (packet->type == PT_CONNECTION_ACK)
					{
						int connectionUID = 0;
						BEET_memcpy(&connectionUID, (char*)packet->data + PACKET_HEADER_SIZE, sizeof(int));
						printf("Connection established with Server: %i\n", connectionUID);
						sc->bt->uid = connectionUID;
						sc->state = READY_TO_SEND;
					}
				}
				else
				{
					printf("Connection closed\n");
					sc->state = CLEANUP;// Close connection
				}
				BeeT_packet_Cleanup(packet);
			}
		}

		if (sc->state == READY_TO_SEND)
		{
			if (BeeT_NW_SocketReadyToSend(sc))
			{
				sc->state = WAITING_SEND_ACK;
			}
		}

		if (sc->state == WAITING_SEND_ACK)
		{
			BeeT_NW_SocketWaitingSendAck(sc);
		}

		if (sc->state == CLEANUP)
		{
			SDLNet_TCP_DelSocket(nw->socketSet, sc->socket);
			SDLNet_TCP_Close(sc->socket);
			sc->state = NOT_INIT;
		}

		it = it->next;
	}
}

void BeeT_NW_Cleanup(BeeT_network * nw)
{
	SDLNet_FreeSocketSet(nw->socketSet);
	if(nw->socketList)
		DestroyDequeue(nw->socketList);
	SDLNet_Quit();
}

BEET_bool BeeT_NW_OpenSocket(BeeT_network* nw, BeeT_dBT* bt)
{
	BEET_bool ret = BEET_FALSE;
	node_deq* it = dequeue_head(nw->socketList);
	while (it != NULL)
	{
		BeeT_socket* sc = (BeeT_socket*)it->data;
		if (sc->state == NOT_INIT)
		{
			sc->state = OPEN;
			sc->bt = bt;
			ret = BEET_TRUE;
			break;
		}

		it = it->next;
	}
	return ret;
}

BEET_bool BeeT_NW_SocketReadyToSend(BeeT_socket * sc)
{
	// Send the BT
	if (sc->bt->initialized == BEET_FALSE)
	{
		BeeT_packet* packet = BeeT_packet_Create(PT_BT_FILE, sc->bt->btBuffer, sc->bt->dataToSendSize);
		int dataSent = SDLNet_TCP_Send(sc->socket, packet->data, packet->size);
		if (dataSent < (int)packet->size)
		{
			// An error has occurred or the connection has closed
		}
		sc->sentDataSize = dataSent;
		BeeT_packet_Cleanup(packet);
		sc->bt->initialized = BEET_TRUE;
		return BEET_TRUE;
	}
	else     // Send bt update data
	{
		if (BeeT_dBT_HasDataToSend(sc->bt))
		{
			char* buf = NULL;
			int size = BeeT_dBT_GetSampleData(sc->bt, &buf);
			BeeT_packet* packet = BeeT_packet_Create(PT_BT_UPDATE, buf, size);
			int dataSent = SDLNet_TCP_Send(sc->socket, packet->data, packet->size);
			if (dataSent < (int)packet->size)
			{
				// An error has occurred or the connection has closed
			}

			sc->sentDataSize = dataSent;
			BeeT_packet_Cleanup(packet);
			BeeT_dBT_ClearSampleData(sc->bt);
			if(buf)
				BEET_free(buf);
			return BEET_TRUE;
		}
	}
	return BEET_FALSE;
}

BEET_bool BeeT_NW_SocketWaitingSendAck(BeeT_socket * sc)
{
	if (SDLNet_SocketReady(sc->socket))
	{
		BeeT_packet* packet = (BeeT_packet*)BEET_malloc(sizeof(BeeT_packet));
		BEET_bool readRet = BeeT_packet_Read(packet, &sc->socket);
		if (readRet)
		{
			if (packet->type == PT_BT_FILE_ACK)
			{
				printf("Server has received the BT file\n");
				sc->sentDataSize = 0;
				BEET_free(sc->bt->btBuffer);
				sc->bt->btBuffer = NULL;
				sc->state = READY_TO_SEND;
			}
			else if (packet->type == PT_BT_UPDATE_ACK)
			{
				// TODO: Check if data received is equal to data sent
				printf("Server has received BT update\n");
				sc->sentDataSize = 0;
				sc->state = READY_TO_SEND;
			}
		}
		else
		{
			printf("Connection closed\n");
			sc->state = CLEANUP;// Close connection
		}
		BeeT_packet_Cleanup(packet);
		return BEET_TRUE;
	}
	return BEET_FALSE;
}
