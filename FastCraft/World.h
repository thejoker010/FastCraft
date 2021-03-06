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


#ifndef _FASTCRAFTHEADER_WORLD
#define _FASTCRAFTHEADER_WORLD
#include <vector>
#include <string>
#include <utility>
#include <Poco/Mutex.h>
#include "Structs.h"
#include "EntityCoordinates.h"
#include "ItemInformationProvider.h"

using std::vector;
using std::string;
using std::pair;
class MinecraftServer;

struct ChunkInternal {
	int X;
	int Z;
	MapChunk* Chunk;
};

class World {
private:
	string _WorldName;
	char _iDimension;
	vector<vector<ChunkInternal>> _vChunks; //[x][z]

	Poco::Mutex _Mutex;
	MinecraftServer* _pMinecraftServer;
public:
	/*
	* Constructs a new World
	* Generates 20x20 chunks around 0,0

	Parameter:
	@1 : World name
	@2 : Dimension ( 0=Overworld	1=End	-1=Neather )
	@3 : MinecraftServer instance that runs this class
	*/
	World(string,char,MinecraftServer*);


	/*
	* Destruct world object
	* Will free map
	* Will save map to hdd
	*/
	~World();


	/*
	* Look for a chunk and returns a pointer to it
	* Generates chunk if not existing
	* Will throw Poco::RuntimeException if generateChunks failed

	Paramter:
	@1 : X in chunkcoordinates
	@2 : Z in chunkcoordiantes
	*/
	MapChunk* getChunkByChunkCoordinates(int,int);


	/*
	* Converts WorldCoordinates to chunk coordinates and block index
	* Will not check chunk existance
	* Will not generate chunk
	* Will throw Poco::RuntimeException if Y is out of bound

	Parameter:
	@1 : X in world coordiantes
	@2 : Y in world coordiantes
	@3 : Z in world coordiantes
	*/
	static pair<ChunkCoordinates,int> WorldCoordinateConverter(int,short,int);


	/*
	* Returns a Y coordinate at given coordinates who player can stand on without suffocate in the wall 
	* Will generate chunk if not exits
	* Will return -1 if there is no free space

	Parameter:
	@1 : X in world coordinates
	@2 : Z in world coordiantes
	*/
	char getFreeSpace(int,int);


	/*
	* Returns true if player will suffocate at his actual place
	* Will also return true if player is the void
	* Will generate chunk if not exist

	Parameter:
	@1 : Players coordinates
	*/
	bool isSuffocating(EntityCoordinates);


	/*
	* Sets block
	* Generates chunk if not existing
	* Will throw Poco::RuntimeException if generateChunks failed
	* Will throw Poco::RuntimeException if block not exists

	Parameter:
	@1 : X in world coordinates
	@2 : Y in world coordinates
	@3 : Z in world coordinates
	@4 : Block information
	*/
	void setBlock(int,short,int,ItemID);
	void setBlock(BlockCoordinates,ItemID);

	/*
	* Gets block at given position
	* Generates chunk if not existing
	* Will throw Poco::RuntimeException if generateChunks failed

	Parameter:
	@1 : X in world coordinates
	@2 : Y in world coordinates
	@3 : Z in world coordinates
	*/
	ItemID getBlock(int,short,int);
	ItemID getBlock(BlockCoordinates);


	/*
	* Returns true if block at given coordinates is fully surrounded by air

	Parameters:
	@1 : BlockCoordinates of target block
	*/
	bool isSurroundedByAir(BlockCoordinates);


	/*
	* Loads world data from given path

	Parameter:
	@1 : Path to world folder (contains directories: region,players, level.dat)
	*/
	void Load(Poco::Path);


	/*
	* Returns worldname
	*/
	string getName();


	/*
	* Returns dimension
	*/
	char getDimension();


	/*
	* Sets block light of given block
	* Will throw Poco::RuntimeException if block light is invalid
	* Will rethrow all errors of getChunkByChunkCoordinates

	Parameters:
	@1 : X in WorldCoordinates
	@2 : Y in WorldCoordinates
	@3 : Z in WorldCoordinates
	@4 : new light level
	*/
	void setBlockLight(int,short,int,char);
	void setBlockLight(BlockCoordinates,char);
private:
	void generateChunks(int,int,int,int);
	MapChunk* generateChunk(int,int);

	char prepareNibble(char, /* Modulo of Nibble index and 2 */
					   char, /* Old value */
					   char /* Value to set */
					   );
}; 

#endif