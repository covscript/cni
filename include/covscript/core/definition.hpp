#pragma once
/*
* Covariant Script Definition
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

// Platform detection

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__WINDOWS__)

#include <Windows.h>

#define COVSCRIPT_PLATFORM_WIN32
#if defined(_WIN64)
#define COVSCRIPT_PLATFORM_NAME "Win64"
#else
#define COVSCRIPT_PLATFORM_NAME "Win32"
#endif
#define COVSCRIPT_PLATFORM_HOME "C:\\Program Files (x86)\\CovScript"
#endif

#if defined(linux) || defined(__linux) || defined(__linux__) || defined(__gnu_linux__)
#define COVSCRIPT_PLATFORM_LINUX
#define COVSCRIPT_PLATFORM_NAME "Linux"
#define COVSCRIPT_PLATFORM_HOME "/usr/share/covscript"
#endif

#if defined(macintosh) || defined(Macintosh) || defined(__APPLE__) || defined(__MACH__)
#define COVSCRIPT_PLATFORM_DARWIN
#define COVSCRIPT_PLATFORM_NAME "Darwin"
#define COVSCRIPT_PLATFORM_HOME "/Applications/CovScript.app/Contents/MacOS/covscript"
#endif

#if defined(COVSCRIPT_PLATFORM_LINUX) || defined(COVSCRIPT_PLATFORM_DARWIN)
#define COVSCRIPT_PLATFORM_UNIX
#endif

#if defined(__clang__)
#define COVSCRIPT_COMPILER_CLANG
#define COVSCRIPT_COMPILER_SIMPILE_NAME "Clang"
#endif

#if defined(__llvm__)
#define COVSCRIPT_COMPILER_LLVM
#if defined(COVSCRIPT_COMPILER_CLANG)
#undef  COVSCRIPT_COMPILER_SIMPILE_NAME
#define COVSCRIPT_COMPILER_SIMPILE_NAME "LLVM/Clang"
#else
#define COVSCRIPT_COMPILER_SIMPILE_NAME "LLVM"
#endif
#endif

#if defined(__GNUC__)
#define COVSCRIPT_COMPILER_GNUC
#if defined(COVSCRIPT_COMPILER_CLANG)
#undef  COVSCRIPT_COMPILER_SIMPILE_NAME
#define COVSCRIPT_COMPILER_SIMPILE_NAME "GCC/Clang"
#else
#define COVSCRIPT_COMPILER_SIMPILE_NAME "GCC"
#endif
#endif

#if defined(_MSC_VER)
#define COVSCRIPT_COMPILER_MSVC
#if defined(COVSCRIPT_COMPILER_CLANG)
#undef  COVSCRIPT_COMPILER_SIMPILE_NAME
#define COVSCRIPT_COMPILER_SIMPILE_NAME "MSVC/Clang"
#else
#define COVSCRIPT_COMPILER_SIMPILE_NAME "MSVC"
#endif
#endif

#define COVSCRIPT_COMPILER_NAME COVSCRIPT_COMPILER_SIMPILE_NAME

#if defined(__MINGW32__)
#undef COVSCRIPT_COMPILER_NAME
#define COVSCRIPT_COMPILER_NAME COVSCRIPT_COMPILER_SIMPILE_NAME " (MinGW)"
#define COVSCRIPT_COMPILER_MINGW
#define COVSCRIPT_COMPILER_MINGW32
#endif

#if defined(__MINGW64__)
#undef COVSCRIPT_COMPILER_NAME
#define COVSCRIPT_COMPILER_NAME COVSCRIPT_COMPILER_SIMPILE_NAME " (MinGW-w64)"
#define COVSCRIPT_COMPILER_MINGW
#define COVSCRIPT_COMPILER_MINGW64
#endif

// Types

namespace cs_impl {
	class any;

	class cni;
}
namespace cs {
	class domain_type;

	class name_space;

	template<typename _kT, typename _vT> using map_t = std::unordered_map<_kT, _vT>;
	template<typename _Tp> using set_t = std::unordered_set<_Tp>;


	using var = cs_impl::any;
	using boolean = bool;
	using string = std::string;
	using vector = std::vector<var>;
	using callable = std::function<var(vector &)>;
	using domain_t = std::shared_ptr<domain_type>;
	using namespace_t = std::shared_ptr<name_space>;

// Compatible declaration since 210503
	using number = numeric_float;

	typedef void(*cs_exception_handler)(const lang_error &);

	typedef void(*std_exception_handler)(const std::exception &);

// Path seperator and delimiter
#ifdef COVSCRIPT_PLATFORM_WIN32
	constexpr char path_separator = '\\';
	constexpr char path_delimiter = ';';
#else
	constexpr char path_separator = '/';
	constexpr char path_delimiter = ':';
#endif
}
