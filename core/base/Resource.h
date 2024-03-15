/*
 * Copyright (c) 2012-2016, chunquedong
 *
 * This file is part of cppfan project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
 *
 * History:
 *   2012-12-23  Jed Young  Creation
 */
#ifndef RESOURCE_H
#define RESOURCE_H


#include "Ref.h"
#include "Stream.h"

namespace mgp
{

class Resource : public Refable {
protected:
	std::string _id;
public:
	static std::string genId();

	Resource();
	Resource(const std::string& id);

	void setId(const std::string& id) { _id = id; }
	const std::string& getId() { return _id; }

	virtual void write(Stream* file) = 0;
	virtual bool read(Stream* file) = 0;
};

}

#endif