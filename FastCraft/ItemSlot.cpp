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

#include "ItemSlot.h"
#include <Poco/Exception.h>
#include <iostream>
#include "NetworkIn.h"
#include "NetworkOut.h"

ItemSlot::ItemSlot(ItemInformationProvider* pIIP): 
_vEnchantments(0)
{
	_pItemInfoProvider = pIIP;
	_Item.first = _Item.second = 0;
	_iStackSize = 0;
	_iUsage = 0;

	_pItemCache_Block = NULL;
	_pItemCache_Item = NULL;
}

ItemSlot::ItemSlot(ItemInformationProvider* pIIP,ItemID id,char iStackSize) { 
	_pItemInfoProvider = pIIP;
	_iStackSize = iStackSize;
	_iUsage = 0;

	if (!pIIP->isRegistered(id)) {
		std::cout<<"ItemSlot::ItemSlot Unregistered item "<<id.first<<":"<<(short(id.second) & 0xff)<<"\n";
		clear();
		return;
	}

	switch(pIIP->isBlock(id)) {
	case true:
		_pItemCache_Block =  &pIIP->getBlock(id);
		_pItemCache_Item = NULL;
		
		if (_iStackSize < 0) { _iStackSize = 0; }
		if (_iStackSize > _pItemCache_Block->MaxStackSize) {_iStackSize = _pItemCache_Block->MaxStackSize;}
		break;
	case false:
		_pItemCache_Block = NULL;
		_pItemCache_Item = &pIIP->getItem(id);

		if (_iStackSize < 0) { _iStackSize = 0; }
		if (_iStackSize > _pItemCache_Item->MaxStackSize) {_iStackSize = _pItemCache_Item->MaxStackSize;}
	}
}


ItemSlot::~ItemSlot() {
	_vEnchantments.clear();
}

ItemID ItemSlot::getItem() {
	return _Item;
}

void ItemSlot::setItem(ItemID id) {
	try{
		switch(_pItemInfoProvider->isBlock(id)) {
		case true:
			_pItemCache_Item = NULL;
			_pItemCache_Block = &_pItemInfoProvider->getBlock(id);
			break;
		case false:
			_pItemCache_Block = NULL;
			_pItemCache_Item = &_pItemInfoProvider->getItem(id);
			break;
		}
		_Item = id;
	}catch(Poco::RuntimeException) {
		std::cout<<"ItemSlot::setItemID Item wasn't found. Affected:"<<id.first<<":"<<id.second<<"\n";
		throw Poco::RuntimeException("ID not registered");
	}
	if (id.first==0) {
		clear();
		return;
	}
}

char ItemSlot::getStackSize() {
	return _iStackSize;
}

void ItemSlot::setStackSize(char size) {
	if (size < 0){
		_iStackSize = 0;
		std::cout<<"ItemSlot::setStackSize Stacksize invalid."<<"\n";
		return;
	}
	if (size == 0) {
		clear();
		return;
	}
	char maxSize;

	switch(_pItemCache_Item == NULL) {
	case true:
		maxSize = _pItemCache_Block->MaxStackSize;
	case false:
		maxSize = _pItemCache_Item->MaxStackSize;
	}

	if (size > maxSize) {
		_iStackSize = maxSize;
		std::cout<<"ItemSlot::setStackSize Stacksize exceeded."<<"\n";
	}else{
		_iStackSize = size;
	}
}

short ItemSlot::getUsage() {
	return _iUsage;
}


void ItemSlot::IncrementUsage() {
	if (_pItemCache_Item == NULL || !_pItemCache_Item->Damageable) {
		std::cout<<"ItemSlot::IncrementUsage Not a tool!"<<"\n";
		throw Poco::RuntimeException("Not a tool");
	}
	short iMaxUsage = _pItemCache_Item->Durability;

	if(_iUsage > iMaxUsage || _iUsage < 0) {
		if (_iStackSize == 1) {//Last item in stack 
			clear();
		}else{                 //Decrement stack size if there is more than one
			_iStackSize--;
			_iUsage = 0;
		}
		return;
	}else{
		_iUsage++;
	}
}

void ItemSlot::setUsage(short iUsage){
	if (_pItemCache_Item == NULL || !_pItemCache_Item->Damageable) {
		std::cout<<"ItemSlot::IncrementUsage Not a tool!"<<"\n";
		throw Poco::RuntimeException("Not a tool");
	}
	short iMaxUsage = _pItemCache_Item->Durability;

	if(iUsage > iMaxUsage || iUsage < 0) {
		//Decrement stack size if more than one
		if (_iStackSize == 1) {
			clear(); //Stack will is empty
		}else{
			_iStackSize--;
			_iUsage = 0;
		}
		return;
	}
	_iUsage = iUsage;
}

void ItemSlot::clear() {
	_Item.first = _Item.second = 0;
	_iStackSize = 0;
	_iUsage = 0;
	_pItemCache_Block = NULL;
	_pItemCache_Item = NULL;
}

bool ItemSlot::isEmpty() {
	if (_iStackSize == 0 || _Item.first == 0) {
		return true;
	}else{
		return false;
	}
}

void ItemSlot::readFromNetwork(NetworkIn& rNetwork) {
	try	{
		short iItemID;

		iItemID = rNetwork.readShort();
		if (iItemID != -1) {
			_iStackSize = rNetwork.readByte();
			short Usage = rNetwork.readShort();
			
			switch (_pItemInfoProvider->isBlock(iItemID)) {
			case true:
				_Item = std::make_pair(iItemID,Usage);
				_iUsage = 0;

				//Check registration state
				if (!_pItemInfoProvider->isRegistered(_Item)) {
					clear();
					throw Poco::RuntimeException("BlockID not registered");
				}

				//Rewrite Item/BlockEntry cache
				_pItemCache_Item = NULL;
				_pItemCache_Block = &_pItemInfoProvider->getBlock(_Item);

				//Check stack size
				if (_iStackSize < 0) {_iStackSize = 0;}
				if (_iStackSize > _pItemCache_Block->MaxStackSize) {_iStackSize = _pItemCache_Block->MaxStackSize;}
				return;
			case false:
				{
					//Check registration state
					if(!_pItemInfoProvider->isRegistered(iItemID)) { 
						clear();
						throw Poco::RuntimeException("ItemID not registered");
					}

					ItemEntry* pItem = &_pItemInfoProvider->getItem(iItemID);
					if (pItem->Damageable) {
						if(pItem->Enchantable) {
							short iEnchPayload = rNetwork.readShort();
							if (iEnchPayload != -1) {
								std::cout<<"Enchantment payload:"<<iEnchPayload<<"\n";
								throw Poco::RuntimeException("Enchantments not supported yet");
							}
						}

						//Rewrite Item cache
						_pItemCache_Block = NULL;
						_pItemCache_Item = pItem;
						_Item = std::make_pair(iItemID,0);
					}else{
						_Item = std::make_pair(iItemID,Usage);

						//Check registration state
						if(!_pItemInfoProvider->isRegistered(_Item)) {
							clear();
							throw Poco::RuntimeException("ItemID not registered");
						}

						//Rewrite item cache
						_pItemCache_Block = NULL;
						_pItemCache_Item = &_pItemInfoProvider->getItem(_Item);
					}
				}
			}
		}else{
			_Item = std::make_pair(0,0);
			_iStackSize = 0;
			_iUsage = 0;
			return;
		}

		//Check maxStackSize
		if (_iStackSize < 0) {_iStackSize = 0;}
		if (_iStackSize > _pItemCache_Item->MaxStackSize) {_iStackSize = _pItemCache_Item->MaxStackSize;}

		//Check Usage
		if (_pItemCache_Item->Damageable) {
			if (_iUsage < 0) {_iUsage=0;}
			if (_iUsage > _pItemCache_Item->Durability) {_iUsage = _pItemCache_Item->Durability;}
		}
	}catch (Poco::RuntimeException) {
		std::cout<<"ItemSlot::ItemSlot(fetch) Item wasn't found.\n";
		clear();
		return;
	}
}

void ItemSlot::writeToNetwork(NetworkOut& Out) {
	if (isEmpty()) {
		Out.addShort(-1);
		return;
	}else{
		Out.addShort(_Item.first);
		Out.addByte(_iStackSize);
		
		if (_pItemCache_Item != NULL && _pItemCache_Item->Damageable) {
			Out.addShort(_iUsage);
			Out.addShort(-1);
		}else{
			Out.addShort(_Item.second);
		}
	}
}

bool ItemSlot::operator == (ItemSlot& other) {
	if (_Item.first == other.getItem().first && _Item.second == other.getItem().second && _iStackSize == other.getStackSize() && _iUsage == other.getUsage()) {
		return true;
	}else{
		return false;
	}
}

bool ItemSlot::operator != (ItemSlot& other) {
	if (_Item.first == other.getItem().first && _Item.second == other.getItem().second && _iStackSize == other.getStackSize() && _iUsage == other.getUsage()) {
		return false;
	}else{
		return true;
	}
}

bool ItemSlot::isBlock() {
	if (_pItemCache_Block == NULL && _pItemCache_Item == NULL) {return true;}
	if (_pItemCache_Item == NULL) {
		return true;
	}else{
		false;
	}
}

BlockEntry* ItemSlot::getBlockEntryCache() {
	return _pItemCache_Block;
}


ItemEntry* ItemSlot::getItemEntryCache() {
	return _pItemCache_Item;
}
