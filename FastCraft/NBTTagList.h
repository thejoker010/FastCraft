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
#ifndef _FASTCRAFTHEADER_NBT_LIST
#define _FASTCRAFTHEADER_NBT_LIST
#include <string>
#include <vector>
#include "NBTTagBase.h"
using std::string;
using std::vector;

//A list consists of elements without names
class NBTTagList : public  NBTTagBase {
private:
	vector<NBTTagBase*> _vpElements;
	const char _iElementType;
public:
	/*
	* Constructor
	* Throws Poco::RuntimeException if given type is invalid

	Parameter:
	@1 : Name of element
	@2 : Type of elements to store
	*/
	NBTTagList(string,char);


	/*
	* Destructor
	*/
	~NBTTagList();


	/*
	* Writes own content to string
	* FC_NBT_OUTPUT_GZIP will be ignored 

	Parameter:
	@1 : Reference to target string
	@2 : Output Type: FC_NBT_OUTPUT_RAW or FC_NBT_OUTPUT_GZIP
	@3 : Headerless flag (won't write name and type field if is set to true)
	*/
	void write(string&,bool,bool = false);


	/*
	* [ ] Operator
	* Use it to get a element at specific position
	* Will throw Poco::RuntimeException if index is out of bound or invalid
	*/
	NBTTagBase* operator[] (int);


	/*
	* Returns cont of stored elements
	*/
	int size();


	/*
	* Returns true if no elements are in storage
	*/
	bool isEmpty();


	/*
	* Adds a new element to storage
	* Throws Poco::RuntimeException if given element type doesn't fits into list type

	Parameter:
	@1 : Element to add
	*/
	void addSubElement(NBTTagBase*);


	/*
	* Removes element with given id
	* Throws Poco::RuntimeException if ID is out of bound or invalid

	Parameter:
	@1 : Index of element to delete
	*/
	void erase(int);
};

#endif