/*
FastCraft - Minecraft SMP Server
Copyright (C) 2011 - 2012 Michael Albrecht aka Sprenger120

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef _FASTCRAFTHEADER_PLAYERTHREAD
#define _FASTCRAFTHEADER_PLAYERTHREAD
#include <iostream>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Runnable.h>
#include <Poco/NumberFormatter.h>
#include <Poco/Thread.h>
#include <Poco/Random.h>
#include <vector>
#include "ThreadSafeQueue.h"
#include "Structs.h"
#include "NetworkIn.h"
#include "NetworkOutRoot.h"
#include "NetworkOut.h"
#include "ChunkProvider.h"
#include "EntityFlags.h"
#include "NetworkWriter.h"
#include "PlayerInventory.h"
#include "ItemSlot.h"
#include "EntityCoordinates.h"
#include "EntityPlayer.h"
#include <utility>
#include "ItemInfoStorage.h"
#include "ThreadTickSpan.h"

class EntityProvider;
class PlayerPool;
class World;
class PackingThread;

using std::string;
using std::stringstream;
using std::vector;
using std::pair;

struct EntityListEntry {
	int EntityID;
	char Type;
	EntityCoordinates oldPosition;
};

class PlayerThread : public Poco::Runnable {
private:
	//Player specific data
	EntityFlags _Flags; //Burning,eating...
	EntityCoordinates _Coordinates; //Coordinates
	EntityCoordinates _lastCoordinates;
	ChunkCoordinates _lastChunkCoordinates;
	string _sName; //Minecraft.net Username
	string _sIP; //IP
	string _sLeaveMessage;
	int _iEntityID;
	short _iHealth;
	short _iFood;
	float _nSaturation;
    bool _fSpawned;
	bool _fHandshakeSent;
	int _Spawned_PlayerInfoList;
	PlayerInventory _Inventory;
	double _dRunnedMeters;
	string _WorldWhoIn;

	//TCP stuff
	string _sTemp;
	Poco::Net::StreamSocket _Connection;
	NetworkOutRoot _NetworkOutRoot;
	NetworkIn _NetworkInRoot;

	NetworkWriter _NetworkWriter;
	Poco::Thread _threadNetworkWriter;

	//Needed Classes
	PlayerPool* _pPoolMaster;
	ChunkProvider _ChunkProvider;
	World* _pWorld;

	//Thread specific
	bool _fAssigned;
	bool _fRunning;
	long long _iThreadTicks;
	static int _PlayerCount;

	//Verification
	Poco::Net::HTTPClientSession _Web_Session;
	Poco::Net::HTTPResponse _Web_Response;
	string _sConnectionHash;
	Poco::Random _Rand;

	//Entities
	vector<EntityListEntry> _vSpawnedEntities;

	//Thread Tick spans
	ThreadTickSpan _timespanSendTime;
	ThreadTickSpan _timespanSendKeepAlive;
	ThreadTickSpan _timespanHandleMovement;
	ThreadTickSpan _timespanMovementSent;
	ThreadTickSpan _timespanSpeedCalculation;
	ThreadTickSpan _timespanPositionCheck;

	ThreadTickSpan _timerLastBlockPlace;
	ThreadTickSpan _timerStartedEating;
	ThreadTickSpan _timerStartedDigging;
	ThreadTickSpan _timerLastAlivePacketSent;
	long long _iPlayerPing;
public:
	/*
	* De- / constructor
	*/
	PlayerThread(PlayerPool*,PackingThread&);
	~PlayerThread();


	/*
	* Thread  Main
	*/
	virtual void run(); // Thread Main


	/*
	* Connect a TCP player connection for server <-> interchange
	*/
	void Connect(Poco::Net::StreamSocket&);


	/*
	* Returns the actual Thread state 
	* false:  Thread is free
	* true:  Thread is busy 
	*/
	bool isAssigned(); 


	/*
	* Interts text into player chat queue

	Parameter:
	@1 : Text to insert 
	*/
	void insertChat(string);


	/*
	* Returns true if Player's login is done
	*/
	bool isSpawned(); 


	/*
	* Returns Playername
	*/
	string getUsername();


	/*
	* Returns IP and port of player
	* IP:port
	*/
	string getIP();


	/*
	* Returns Players coordinates and look
	*/
	EntityCoordinates getCoordinates();
	

	/*
	* Returns count of actual connected players
	*/
	static int getConnectedPlayers();


	/*
	* Adds/Removes player from PlayerInfo List (Press Tab in Client)

	Parameters:
	@1 : true for spawn player, false for despawn player
	@2 : player name
	*/
	void PlayerInfoList(bool,string);


	/*
	* Closes connection and clears object

	Parameter:
	@1 : Kick message
	@2 : Disconnect reason code (FC_LEAVE_ in Constants.h)
	@3 : Set to true to show the kick message into chat 
	*/
	void Disconnect(string,bool = true);
	void Disconnect(char);

	/*
	* Spawns a player
	* Will throw Poco::RuntimeException if ID is already spawned

	Parameter:
	@1 : EntityID
	@2 : Reference to EntityPlayer object
	*/
	void spawnPlayer(int,EntityPlayer&);


	/*
	* Returns true if given entity id is spawned

	Parameter:
	@1 : EntityID
	*/
	bool isEntitySpawned(int);

	
	/*
	* Updates entitys position
	* Will throw Poco::RuntimeException if entity wasn't spawned so far

	Parameter:
	@1 : EntityID
	@2 : new Coordinates
	*/
	void updateEntityPosition(int,EntityCoordinates);


	/*
	* Despawns entity
	* Will throw Poco::RuntimeException if entity wasn't spawned so far

	Parameter:
	@1 : EntityID
	*/
	void despawnEntity(int);


	/*
	* Returns EntityID of player
	* Will throw Poco::RuntimeExcpetion if player isn't spawned so far
	*/
	int getEntityID();


	/*
	* Make the given entity do a animation 
	* Will throw Poco::RuntimeException if Entity isn't spawned 
	* Will not check animation id existance - please use FC_ANIM constants
	* Use only for swing arm!

	Parameter:
	@1 : EntityID
	@2 : Animation ID
	*/
	void playAnimationOnEntity(int,char);


	/*
	* Make the given entity do a action
	* Will throw Poco::RuntimeException if Entity isn't spawned 
	* Will not check action id existance - please use FC_ACTION constants
	* Use only for crouching, leaving a bed, or sprinting!

	Parameter:
	@1 : EntityID
	@2 : EntityFlags reference
	*/
	void updateEntityMetadata(int,EntityFlags);

	/*
	* Returns a reference to player's inventory
	*/
	PlayerInventory& getInventory();


	/*
	* Returns players flags
	*/
	EntityFlags getFlags();


	/*
	* Updates entitys equipment

	Parameter:
	@1 : EntityID
	@2 : SlotID (0=held, 4=helmet,3=chestplate,2=pants,1=boots)
	@3 : Item informations
	*/
	void updateEntityEquipment(int,short,ItemID);


	/*
	* Returns chunk queue size
	*/
	int getChunksInQueue();


	/*
	* Sets a block into players view circle
	* Will ignore call if given chunk isn't spawned
	* Will throw Poco::RuntimeException if block not exists

	Parameter:
	@1 : Coordiantes of block
	@2 : Block information
	*/
	void spawnBlock(BlockCoordinates,ItemID);


	/*
	* Returns Worldname who player is actual in
	*/
	string getWorldWhoIn();


	/*
	* Forces the thread to exit
	*/
	void shutdown();


	/*
	* Sets the statuscode of a entity

	Parameter:
	@1 : Entity ID
	@2 : Status code (Constants.h FC_ENTITYSTATUS_)
	*/
	void setEntityStatus(int,char);
	void setEntityStatus(char); //Sets status on player's id
private:
	//Interval functions
	void Interval_KeepAlive();
	void Interval_Time();
	void Interval_HandleMovement();
	void Interval_Movement();
	void Interval_CalculateSpeed();
	void Interval_CheckPosition();
	void Interval_CheckEating();
	
	//Ticks
	long long getTicks(); 

	//Packets
	void Packet0_KeepAlive();
	void Packet1_Login();
	void Packet2_Handshake();
	void Packet3_Chat();
	void Packet10_Player();
	void Packet11_Position();
	void Packet12_Look();
	void Packet13_PosAndLook();
	void Packet14_Digging();
	void Packet15_PlayerBlockPlacement();
	void Packet16_HoldingChange();
	void Packet18_Animation();
	void Packet19_EntityAction();
	void Packet101_CloseWindow();
	void Packet102_WindowClick();
	void Packet254_ServerListPing();
	void Packet255_Disconnect();

	//Network
	void ProcessQueue();
	void sendTime();
	void sendClientPosition();
	void sendKeepAlive();
	void ChatToAll(string&);

	//Handler
	void handleEating(); 


	//'fix' functions
	template <class T> T fixRange(T,T,T);

	//sync functions
	void syncHealth();
	void syncFlagsWithPP();


	//Other
	string generateConnectionHash(); //Generate a new connection hash, write it to _ConnectionHash	
	void CheckPosition(bool=true); //checks players position and correct it. Will synchronize with player if bool is true
};

#endif