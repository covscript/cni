/*
* Covariant Script CNI Standalone Version
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* Copyright (C) 2017-2023 Michael Lee(李登淳)
*
* This software is registered with the National Copyright Administration
* of the People's Republic of China(Registration Number: 2020SR0408026)
* and is protected by the Copyright Law of the People's Republic of China.
*
* Email:   lee@covariant.cn, mikecovlee@163.com
* Github:  https://github.com/mikecovlee
* Website: https://covscript.org.cn
*/
#include <covscript/core/core.hpp>

#ifdef COVSCRIPT_PLATFORM_WIN32

#include <shlobj.h>

#pragma comment(lib, "shell32.lib")

#else

#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>

#endif

#ifdef _MSC_VER
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#include <windows.h>
#include <Dbghelp.h>
#pragma comment(lib, "DbgHelp")
namespace cs_impl {
	std::string cxx_demangle(const char* name)
	{
		char buffer[1024];
		DWORD length = UnDecorateSymbolName(name, buffer, sizeof(buffer), 0);
		if (length > 0)
			return std::string(buffer, length);
		else
			return name;
	}
}
#elif defined __GNUC__

#include <cxxabi.h>

namespace cs_impl {
	std::string cxx_demangle(const char *name)
	{
		char buffer[1024] = {0};
		size_t size = sizeof(buffer);
		int status;
		char *ret = abi::__cxa_demangle(name, buffer, &size, &status);
		if (ret != nullptr)
			return std::string(ret);
		else
			return name;
	}
}
#endif

namespace cs_impl {
	cs::allocator_type<any::proxy, default_allocate_buffer_size*default_allocate_buffer_multiplier, default_allocator_provider> any::allocator;
}

namespace cs {
	std::atomic_size_t global_thread_counter(0);

	bool process_context::on_process_exit_default_handler(void *code)
	{
		extension::gc.collect();
		std::exit(*static_cast<int *>(code));
		return true;
	}

	process_context this_process;
	process_context *current_process = &this_process;

	void copy_no_return(var &val)
	{
		if (!val.is_rvalue()) {
			val.clone();
			val.detach();
		}
		else
			val.mark_as_rvalue(false);
	}

	var copy(var val)
	{
		if (!val.is_rvalue()) {
			val.clone();
			val.detach();
		}
		else
			val.mark_as_rvalue(false);
		return val;
	}

	var lvalue(const var &val)
	{
		val.mark_as_rvalue(false);
		return val;
	}

	var rvalue(const var &val)
	{
		val.mark_as_rvalue(true);
		return val;
	}

	var try_move(const var &val)
	{
		val.try_move();
		return val;
	}

	var make_namespace(const namespace_t &ns)
	{
		return var::make_protect<namespace_t>(ns);
	}

	garbage_collector<cov::dll> extension::gc;
}