//
// tick_count_timer.cpp
// ~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/asio.hpp>
#include <ctime>
#include <iostream>

#if defined(WIN32)
# include <windows.h>
#else
# error This example is for Windows only!
#endif

namespace PublicSupport {

struct tick_count_traits
{
	// The time type. This type has no constructor that takes a unsigned long to ensure
	// that the timer can only be used with relative times.
	class time_type
	{
	public:
		time_type() : ticks_(0) {}
	private:
		friend struct tick_count_traits;
		unsigned long ticks_;
	};

	// The duration type.
	class duration_type
	{
	public:
		duration_type() : ticks_(0) {}
		duration_type(unsigned long ticks) : ticks_(ticks) {}
	private:
		friend struct tick_count_traits;
		unsigned long ticks_;
	};

	// Get the current time.
	static time_type now()
	{
		time_type result;
		result.ticks_ = ::GetTickCount();
		return result;
	}

	// Add a duration to a time.
	static time_type add(const time_type& t, const duration_type& d)
	{
		time_type result;
		result.ticks_ = t.ticks_ + d.ticks_;
		return result;
	}

	// Subtract one time from another.
	static duration_type subtract(const time_type& t1, const time_type& t2)
	{
		return duration_type(t1.ticks_ - t2.ticks_);
	}

	// Test whether one time is less than another.
	static bool less_than(const time_type& t1, const time_type& t2)
	{
		// unsigned long tick count values wrap periodically, so we'll use a heuristic that
		// says that if subtracting t1 from t2 yields a value smaller than 2^31,
		// then t1 is probably less than t2. This means that we can't handle
		// durations larger than 2^31, which shouldn't be a problem in practice.
		return (t2.ticks_ - t1.ticks_) < static_cast<unsigned long>(1 << 31);
	}

	// Convert to POSIX duration type.
	static boost::posix_time::time_duration to_posix_duration(
		const duration_type& d)
	{
		return boost::posix_time::milliseconds(d.ticks_);
	}
};

typedef boost::asio::basic_deadline_timer<unsigned long, tick_count_traits> tick_count_timer;

};//namespace PublicSupport 
