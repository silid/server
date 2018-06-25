/*
* Copyright (c) 2011 Sveriges Television AB <info@casparcg.com>
*
* This file is part of CasparCG (www.casparcg.com).
*
* CasparCG is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* CasparCG is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with CasparCG. If not, see <http://www.gnu.org/licenses/>.
*
* Author: Cambell Prince, cambell.prince@gmail.com
*/

#pragma once

#include "../fwd.h"

#include <common/memory.h>

#include <boost/noncopyable.hpp>
#include <boost/property_tree/ptree_fwd.hpp>

#include <functional>
#include <string>
#include <vector>

namespace caspar { namespace core {

enum frame_consumer_mode {
    foreground,
    background,
    next_producer, // background if any, then foreground
};

struct write_frame_consumer : boost::noncopyable
{
	virtual ~write_frame_consumer() {}
	
	virtual void send(const draw_frame& frame) = 0;
	virtual std::wstring print() const = 0;
	//virtual boost::property_tree::wptree info() const = 0;
};

}}