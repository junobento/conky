/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: t -*-
 * vim: ts=4 sw=4 noet ai cindent syntax=cpp
 *
 * Conky, a system monitor, based on torsmo
 *
 * Please see COPYING for details
 *
 * Copyright (C) 2010 Pavel Labath et al.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "config.h"

#include "c++wrap.hh"

#include <unistd.h>
#include <string.h>

#include <cassert>
#include <cstdarg>

#if !defined(HAVE_PIPE2) || !defined(HAVE_O_CLOEXEC)
#include <fcntl.h>

namespace {
	int pipe2_emulate(int pipefd[2], int flags)
	{
		if(pipe(pipefd) == -1)
			return -1;

		if(flags & O_CLOEXEC) {
			// we emulate O_CLOEXEC if the system does not have it
			// not very thread-safe, but at least it works

			for(int i = 0; i < 2; ++i)
				fcntl_setfd(pipefd[i], fcntl_getfd(pipefd[i]) | FD_CLOEXEC);
		}

		return 0;
	}

	int (* const pipe2_ptr)(int[2], int) = &pipe2_emulate;
}
#else
	int (* const pipe2_ptr)(int[2], int) = &pipe2;
#endif

namespace priv {
	int fcntl_getf(int fd, int what)
	{
		int r = fcntl(fd, what);
		if(r == -1)
			throw errno_error("fcntl");

		return r;
	}

	void fcntl_setf(int fd, int what, int flags)
	{
		if(fcntl(fd, what, flags) == -1)
			throw errno_error("fcntl");
	}
}

std::string strerror_r(int errnum)
{
	char buf[100];
	strerror_r(errnum, buf, sizeof buf);
	return buf;
}

std::pair<int, int> pipe2(int flags)
{
	int fd[2];
	if(pipe2_ptr(fd, flags) == -1)
		throw errno_error("pipe2");
	else
		return std::pair<int, int>(fd[0], fd[1]);
}

std::string strprintf(const char *format, ...)
{
	std::string str;
	str.resize(strlen(format)*2);

	va_list list;
	va_start(list, format);

	int r = vsnprintf(&*str.begin(), str.size(), format, list);
	if(r < 0)
		throw errno_error("strprintf");
	if(static_cast<size_t>(r) >= str.size()) {
		str.resize(r+1);
		r = vsnprintf(&*str.begin(), str.size(), format, list);
	}  else
		str.resize(r+1);
	assert(str.size() == r+1u);

	va_end(list);
	return str;
}
