/*
FastCraft - Minecraft SMP Server
Copyright (C) 2011  Michael Albrecht aka Sprenger120

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef _FASTCRAFTHEADER_NETWORKNHANDLER
#define _FASTCRAFTHEADER_NETWORKNHANDLER

#include <iostream>
#include <Poco/Runnable.h>

#include "PlayerThread.h"
#include "SettingsHandler.h"

using Poco::Net::StreamSocket;
using std::string;
using std::cout;
using Poco::Thread;

class NetworkHandler : public Poco::Runnable {
private:
	SettingsHandler* _pSettings;
	PlayerThread* _aPlayers;

	string _sIP;
	bool _fReady;
	const int _iMaxClients;
public:
	//De- /constructor
	NetworkHandler(SettingsHandler*);
	~NetworkHandler();
	bool Ready();


	virtual void run(); //Thread Main
};

#endif