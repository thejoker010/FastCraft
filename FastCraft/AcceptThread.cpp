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
#include "AcceptThread.h" 
#include "NetworkOut.h"
#include "SettingsHandler.h"
#include "PlayerPool.h"
#include <Poco/Net/StreamSocket.h>
#include <Poco/Thread.h>
#include <Poco/Net/NetException.h>
#include "PlayerPool.h"

using Poco::Net::StreamSocket;
using std::cout;
using Poco::Thread;

AcceptThread::AcceptThread(PlayerPool& rPlayerPool):
_rPlayerPool(rPlayerPool),
	_ServerFullMsg(""),
	_ServerSock()
{
	_ServerFullMsg.append<unsigned char>(1,0xFF);
	NetworkOut::addString(_ServerFullMsg,"Server full!");

	try {
		_ServerSock = Poco::Net::ServerSocket(SettingsHandler::getPort());
		_ServerSock.setBlocking(true);
	}catch(Poco::IOException& ex) {
		ex.rethrow();
	}

	_fRunning=false;
}

AcceptThread::~AcceptThread() {
	if (_fRunning) {shutdown();}
}

void AcceptThread::run() {
	Poco::Net::StreamSocket StrmSock;

	while(!PlayerPool::isReady()) {
		Thread::sleep(50);
	}

	_fRunning=true;
	while(_fRunning) {
		try {
			_ServerSock.listen();
			StrmSock = _ServerSock.acceptConnection(); //Ooh a new player


			if (!_rPlayerPool.isAnySlotFree()) { //There is no free slot
				StrmSock.sendBytes(_ServerFullMsg.c_str(),_ServerFullMsg.length());
				Thread::sleep(100); //Wait a moment 
				StrmSock.close();
				continue;
			}

			_rPlayerPool.Assign(StrmSock);
		}catch(...) { /* Only happen if socket become closed */
			_fRunning=true; //ok shutdown(), I'm ready
			return;
		}
	}

}

void AcceptThread::shutdown() {
	if (!_fRunning) {return;}
	_fRunning=false;
	_ServerSock.close();
	while(!_fRunning){ //Wait till _fRunning turns true
	}
	_fRunning=false;
}