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

#include "ChunkRoot.h"
#include "Structs.h"
#include "Constants.h"
#include "ChunkTerraEditor.h"
#include <Poco/Stopwatch.h>
#include <Poco/Exception.h>

using std::memset;
using Poco::Stopwatch;
using Poco::RuntimeException;


ChunkRoot::ChunkRoot() :
_vpChunks(0),
_iChunkSlots(500),
_iMaxChunkSlots(1000) // 1000 * Chunksize( 16*16*128*2.5) = 78 MB
{
	if(_iChunkSlots > _iMaxChunkSlots) {
		throw RuntimeException("Chunkslot count out of bound");
	}
	
	_vpChunks.resize(_iChunkSlots);

	//Allocate memory & init chunks
	for (int x=0;x<=_iChunkSlots-1;x++) {
		_vpChunks[x] = new MapChunk; //Allocate
		_vpChunks[x]->Empty = true; //Set empty
		
		memset(_vpChunks[x]->Blocks,0,FC_CHUNK_BLOCKCOUNT); //clear blocks
	}
}


ChunkRoot::~ChunkRoot() {
	for ( int x=0;x<=_iChunkSlots-1;x++) {
		delete _vpChunks[x];
	}
}

void ChunkRoot::generateMap(int iFromX,int iFromZ,int iToX,int iToZ) {
	Poco::Stopwatch Timer;
	int iCount=0;
	int index=0;
	Block Block;

	Timer.start();

	if (iFromX > iToX || iFromZ > iToZ) {
		throw RuntimeException("Illegal Arguments, From > To");
	}

	//Check chunkslot bound
	int iChunkJobCount =  (iToX - iFromX)  * (iToZ - iFromZ); //get count of chunks that will be generated

	if (getFreeChunkSlotCount() < iChunkJobCount) { //No chunk vector expanding
		iChunkJobCount -= getFreeChunkSlotCount(); //Substract free slots | iChunkJobCount contains chunks who habe no slot

		//check vector bound
		if (_iMaxChunkSlots < _iChunkSlots + iChunkJobCount) { 
			//bounding exception
			RuntimeException("Chunkslot count out of bound"); 
		}else {
			_iChunkSlots += iChunkJobCount;
			_vpChunks.resize(_iChunkSlots);
		}
	}



	for(int chunkX = iFromX;chunkX<=iToX;chunkX++) {
		for (int chunkZ = iFromZ;chunkZ<=iToZ;chunkZ++) {
			
			index = getFreeChunkSlot();
		
			if (index == -1) {
				throw Poco::RuntimeException("Chunk slots are full!");
				return;
			}

			_vpChunks[index]->X = chunkX;
			_vpChunks[index]->Z = chunkZ;
			_vpChunks[index]->Empty = false;

			Block.BlockID = 7;

			//Generate chunk
			try {
				ChunkTerraEditor::setPlate(_vpChunks[index],0,Block); //Bedrock 

				Block.BlockID = 2;

				for (short y=1;y<=30;y++) { //Grass
					ChunkTerraEditor::setPlate(_vpChunks[index],y,Block);
				}

			} catch (Poco::RuntimeException& err) {
				cout<<"***GENERATING ERROR:"<<err.message()<<endl;
				throw Poco::RuntimeException("Generation failed!");
				return;
			}

			//Light & Metadata
			std::memset(_vpChunks[index]->Metadata,0,FC_CHUNK_NIBBLECOUNT);
			std::memset(_vpChunks[index]->BlockLight,0xff,FC_CHUNK_NIBBLECOUNT);
			std::memset(_vpChunks[index]->SkyLight,0xff,FC_CHUNK_NIBBLECOUNT);

			iCount++;
		}
	}

	Timer.stop();
	cout<<"Generated "<<iCount<<" chunks in "<<Timer.elapsed() / 1000<<" ms."<<endl;
}

MapChunk* ChunkRoot::getChunk(int X,int Z) {
	int index;
	index = getChunkIndexByCoords(X,Z);
	if (index==-1) {
		return NULL;
	}else{
		return _vpChunks[index];
	}
}


int ChunkRoot::getChunkIndexByCoords(int X,int Z) {
	for (int x=0;x<=_iChunkSlots-1;x++) {
		if (_vpChunks[x]->X == X && _vpChunks[x]->Z == Z) {
			return x;
		}
	}
	return -1;
}


int ChunkRoot::getFreeChunkSlot() {
	for (int x=0;x<=_iChunkSlots-1;x++) {
		if (_vpChunks[x]->Empty) {
			_iUsedChunkSlots++;
			return x;
		}
	}
	return -1;
}

int ChunkRoot::getFreeChunkSlotCount() {
	return _iChunkSlots - _iUsedChunkSlots;
}

int ChunkRoot::getUsedChunkSlotCount() {
	return _iUsedChunkSlots;
}