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
* Author: Robert Nagy, ronag89@gmail.com
*/

#include "../../StdAfx.h"

#include "color_producer.h"

#include <core/producer/frame_producer.h>
#include <core/frame/frame.h>
#include <core/frame/draw_frame.h>
#include <core/frame/frame_factory.h>
#include <core/frame/pixel_format.h>
#include <core/frame/audio_channel_layout.h>
#include <core/monitor/monitor.h>

#include <common/except.h>
#include <common/array.h>

#include <boost/algorithm/string.hpp>

#include <sstream>

namespace caspar { namespace core {

draw_frame create_color_frame(void* tag, const spl::shared_ptr<frame_factory>& frame_factory, const std::vector<uint32_t>& values)
{
	core::pixel_format_desc desc(pixel_format::bgra);
	desc.planes.push_back(core::pixel_format_desc::plane(static_cast<int>(values.size()), 1, 4));
	auto frame = frame_factory->create_frame(tag, desc, core::audio_channel_layout::invalid());

	for (int i = 0; i < values.size(); ++i)
		*reinterpret_cast<uint32_t*>(frame.image_data(0).begin() + (i * 4)) = values.at(i);

	return core::draw_frame(std::move(frame));
}

draw_frame create_color_frame(void* tag, const spl::shared_ptr<frame_factory>& frame_factory, uint32_t value)
{
	std::vector<uint32_t> values = { value };

	return create_color_frame(tag, frame_factory, values);
}

draw_frame create_color_frame(void* tag, const spl::shared_ptr<frame_factory>& frame_factory, const std::vector<std::wstring>& strs)
{
	std::vector<uint32_t> values(strs.size());

	for (int i = 0; i < values.size(); ++i)
	{
		if (!try_get_color(strs.at(i), values.at(i)))
			CASPAR_THROW_EXCEPTION(user_error() << msg_info(L"Invalid color: " + strs.at(i)));
	}

	return create_color_frame(tag, frame_factory, values);
}

class color_producer : public frame_producer_base
{
	monitor::subject		monitor_subject_;

	const std::wstring		color_str_;
	constraints				constraints_;
	draw_frame				frame_;

public:
	color_producer(const spl::shared_ptr<core::frame_factory>& frame_factory, uint32_t value)
		: color_str_(L"")
		, constraints_(1, 1)
		, frame_(create_color_frame(this, frame_factory, value))
	{
		CASPAR_LOG(info) << print() << L" Initialized";
	}

	color_producer(const spl::shared_ptr<core::frame_factory>& frame_factory, const std::vector<std::wstring>& colors)
		: color_str_(boost::join(colors, L", "))
		, constraints_(static_cast<int>(colors.size()), 1)
		, frame_(create_color_frame(this, frame_factory, colors))
	{
		CASPAR_LOG(info) << print() << L" Initialized";
	}

	// frame_producer

	draw_frame receive_impl() override
	{
		monitor_subject_ << monitor::message("color") % color_str_;

		return frame_;
	}

	constraints& pixel_constraints() override
	{
		return constraints_;
	}

	std::wstring print() const override
	{
		return L"color[" + color_str_ + L"]";
	}

	std::wstring name() const override
	{
		return L"color";
	}

	boost::property_tree::wptree info() const override
	{
		boost::property_tree::wptree info;
		info.add(L"type", L"color");
		info.add(L"color", color_str_);
		return info;
	}

	monitor::subject& monitor_output() override {return monitor_subject_;}
};

std::wstring get_hex_color(const std::wstring& str)
{
	if(str.at(0) == '#')
		return str.length() == 7 ? L"#FF" + str.substr(1) : str;

	std::wstring col_str = boost::to_upper_copy(str);

	if(col_str == L"EMPTY")
		return L"#00000000";

	if(col_str == L"BLACK")
		return L"#FF000000";

	if(col_str == L"WHITE")
		return L"#FFFFFFFF";

	if(col_str == L"RED")
		return L"#FFFF0000";

	if(col_str == L"GREEN")
		return L"#FF00FF00";

	if(col_str == L"BLUE")
		return L"#FF0000FF";

	if(col_str == L"ORANGE")
		return L"#FFFFA500";

	if(col_str == L"YELLOW")
		return L"#FFFFFF00";

	if(col_str == L"BROWN")
		return L"#FFA52A2A";

	if(col_str == L"GRAY")
		return L"#FF808080";

	if(col_str == L"TEAL")
		return L"#FF008080";

	return str;
}

bool try_get_color(const std::wstring& str, uint32_t& value)
{
	auto color_str = get_hex_color(str);
	if(color_str.length() != 9 || color_str[0] != '#')
		return false;

	std::wstringstream ss(color_str.substr(1));
	if(!(ss >> std::hex >> value) || !ss.eof())
		return false;

	return true;
}
spl::shared_ptr<frame_producer> create_color_producer(const spl::shared_ptr<frame_factory>& frame_factory, uint32_t value)
{
	return spl::make_shared<color_producer>(frame_factory, value);
}

spl::shared_ptr<frame_producer> create_color_producer(const spl::shared_ptr<frame_factory>& frame_factory, const std::vector<std::wstring>& params)
{
	if(params.size() < 0)
		return core::frame_producer::empty();

	uint32_t value = 0;
	if(!try_get_color(params.at(0), value))
		return core::frame_producer::empty();

	std::vector<std::wstring> colors;

	for (auto& param : params)
	{
		if (try_get_color(param, value))
			colors.push_back(param);
	}

	return spl::make_shared<color_producer>(frame_factory, colors);
}

}}
