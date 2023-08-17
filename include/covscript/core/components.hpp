#pragma once
/*
* Covariant Script Basic Components
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
#include <covscript/import/mozart/base.hpp>
#include <atomic>

namespace cs {
	extern std::atomic_size_t global_thread_counter;

	struct thread_guard final {
		thread_guard()
		{
			++global_thread_counter;
		}
		thread_guard(const thread_guard &) = delete;
		thread_guard(thread_guard &&) noexcept = delete;
		~thread_guard()
		{
			--global_thread_counter;
		}
	};

// Exceptions
	class exception final : public std::exception {
		std::size_t mLine = 0;
		std::string mFile, mCode, mWhat, mStr;

		static std::string compose_what(const std::string &file, std::size_t line, const std::string &code, const std::string &what)
		{
			return "File \"" + file + "\", line " + std::to_string(line) + ": " + what + "\n>\t" + code + "\n";
		}

		static std::string compose_what_internal(const std::string &file, const std::string &what)
		{
			return "File \"" + file + "\", line <INTERNAL>: " + what + "\n";
		}

	public:
		exception() = delete;

		exception(std::size_t line, std::string file, std::string code, std::string what) noexcept:
			mLine(line), mFile(std::move(file)), mCode(std::move(code)), mWhat(std::move(what))
		{
			mStr = compose_what(mFile, mLine, mCode, mWhat);
		}

		exception(const exception &) = default;

		exception(exception &&) noexcept = default;

		~exception() override = default;

		exception &operator=(const exception &) = default;

		exception &operator=(exception &&) = default;

		const std::string &file() const noexcept
		{
			return mFile;
		}

		const char *what() const noexcept override
		{
			return this->mStr.c_str();
		}
	};

	class compile_error final : public std::exception {
		std::string mWhat = "Compile Error";
	public:
		compile_error() = default;

		explicit compile_error(const std::string &str) noexcept:
			mWhat("Compile Error: " + str) {}

		compile_error(const compile_error &) = default;

		compile_error(compile_error &&) noexcept = default;

		~compile_error() override = default;

		compile_error &operator=(const compile_error &) = default;

		compile_error &operator=(compile_error &&) = default;

		const char *what() const noexcept override
		{
			return this->mWhat.c_str();
		}
	};

	class runtime_error final : public std::exception {
		std::string mWhat = "Runtime Error";
	public:
		runtime_error() = default;

		explicit runtime_error(const std::string &str) noexcept:
			mWhat("Runtime Error: " + str) {}

		runtime_error(const runtime_error &) = default;

		runtime_error(runtime_error &&) noexcept = default;

		~runtime_error() override = default;

		runtime_error &operator=(const runtime_error &) = default;

		runtime_error &operator=(runtime_error &&) = default;

		const char *what() const noexcept override
		{
			return this->mWhat.c_str();
		}
	};

	class internal_error final : public std::exception {
		std::string mWhat = "Internal Error";
	public:
		internal_error() = default;

		explicit internal_error(const std::string &str) noexcept:
			mWhat("Internal Error: " + str) {}

		internal_error(const internal_error &) = default;

		internal_error(internal_error &&) noexcept = default;

		~internal_error() override = default;

		internal_error &operator=(const internal_error &) = default;

		internal_error &operator=(internal_error &&) = default;

		const char *what() const noexcept override
		{
			return this->mWhat.c_str();
		}
	};

	class lang_error final {
		std::string mWhat;
	public:
		lang_error() = default;

		explicit lang_error(std::string str) noexcept:
			mWhat(std::move(str)) {}

		lang_error(const lang_error &) = default;

		lang_error(lang_error &&) noexcept = default;

		~lang_error() = default;

		lang_error &operator=(const lang_error &) = default;

		lang_error &operator=(lang_error &&) = default;

		const char *what() const noexcept
		{
			return this->mWhat.c_str();
		}
	};

	class fatal_error final : public std::exception {
		std::string mWhat = "Fatal Error";
	public:
		fatal_error() = default;

		explicit fatal_error(const std::string &str) noexcept:
			mWhat("Fatal Error: " + str) {}

		fatal_error(const fatal_error &) = default;

		fatal_error(fatal_error &&) noexcept = default;

		~fatal_error() override = default;

		fatal_error &operator=(const fatal_error &) = default;

		fatal_error &operator=(fatal_error &&) = default;

		const char *what() const noexcept override
		{
			return this->mWhat.c_str();
		}
	};

	class forward_exception final : public std::exception {
		std::string mWhat;
	public:
		forward_exception() = delete;

		explicit forward_exception(const char *str) noexcept: mWhat(str) {}

		forward_exception(const forward_exception &) = default;

		forward_exception(forward_exception &&) noexcept = default;

		~forward_exception() override = default;

		forward_exception &operator=(const forward_exception &) = default;

		forward_exception &operator=(forward_exception &&) = default;

		const char *what() const noexcept override
		{
			return this->mWhat.c_str();
		}
	};

// Numeric
	using numeric_float = long double;
	using numeric_integer = long long int;

	class numeric final {
		union {
			numeric_float _num;
			numeric_integer _int;
		} data;
		bool type = 1;

		inline static std::uint8_t get_composite_type(bool lhs, bool rhs) noexcept
		{
			return lhs << 1 | rhs;
		}

	public:
		numeric()
		{
			data._int = 0;
		}

		template<typename T>
		numeric(const T &dat)
		{
			if (std::is_integral<T>::value) {
				type = 1;
				data._int = dat;
			}
			else {
				type = 0;
				data._num = dat;
			}
		}

		numeric(const numeric &rhs) : data(rhs.data), type(rhs.type) {}

		numeric(numeric &&rhs) noexcept: data(rhs.data), type(rhs.type) {}

		~numeric() = default;

		numeric operator+(const numeric &rhs) const noexcept
		{
			switch (get_composite_type(type, rhs.type)) {
			default:
			case 0b00:
				return data._num + rhs.data._num;
			case 0b01:
				return data._num + rhs.data._int;
			case 0b10:
				return data._int + rhs.data._num;
			case 0b11:
				return data._int + rhs.data._int;
			}
		}

		template<typename T>
		numeric operator+(const T &rhs) const noexcept
		{
			if (type)
				return data._int + rhs;
			else
				return data._num + rhs;
		}

		numeric operator-(const numeric &rhs) const noexcept
		{
			switch (get_composite_type(type, rhs.type)) {
			default:
			case 0b00:
				return data._num - rhs.data._num;
			case 0b01:
				return data._num - rhs.data._int;
			case 0b10:
				return data._int - rhs.data._num;
			case 0b11:
				return data._int - rhs.data._int;
			}
		}

		template<typename T>
		numeric operator-(const T &rhs) const noexcept
		{
			if (type)
				return data._int - rhs;
			else
				return data._num - rhs;
		}

		numeric operator*(const numeric &rhs) const noexcept
		{
			switch (get_composite_type(type, rhs.type)) {
			default:
			case 0b00:
				return data._num * rhs.data._num;
			case 0b01:
				return data._num * rhs.data._int;
			case 0b10:
				return data._int * rhs.data._num;
			case 0b11:
				return data._int * rhs.data._int;
			}
		}

		template<typename T>
		numeric operator*(const T &rhs) const noexcept
		{
			if (type)
				return data._int * rhs;
			else
				return data._num * rhs;
		}

		numeric operator/(const numeric &rhs) const noexcept
		{
			switch (get_composite_type(type, rhs.type)) {
			default:
			case 0b00:
				return data._num / rhs.data._num;
			case 0b01:
				return data._num / rhs.data._int;
			case 0b10:
				return data._int / rhs.data._num;
			case 0b11:
				if (data._int % rhs.data._int != 0)
					return static_cast<numeric_float>(data._int) / rhs.data._int;
				else
					return data._int / rhs.data._int;
			}
		}

		template<typename T>
		numeric operator/(const T &rhs) const noexcept
		{
			if (type)
				return data._int / rhs;
			else
				return data._num / rhs;
		}

		numeric &operator=(const numeric &num)
		{
			if (this != &num) {
				data = num.data;
				type = num.type;
			}
			return *this;
		}

		template<typename T>
		numeric &operator=(const T &dat)
		{
			if (std::is_integral<T>::value) {
				type = 1;
				data._int = dat;
			}
			else {
				type = 0;
				data._num = dat;
			}
			return *this;
		}

		bool operator<(const numeric &rhs) const noexcept
		{
			switch (get_composite_type(type, rhs.type)) {
			default:
			case 0b00:
				return data._num < rhs.data._num;
			case 0b01:
				return data._num < rhs.data._int;
			case 0b10:
				return data._int < rhs.data._num;
			case 0b11:
				return data._int < rhs.data._int;
			}
		}

		template<typename T>
		bool operator<(const T &rhs) const noexcept
		{
			if (type)
				return data._int < rhs;
			else
				return data._num < rhs;
		}

		bool operator<=(const numeric &rhs) const noexcept
		{
			switch (get_composite_type(type, rhs.type)) {
			default:
			case 0b00:
				return data._num <= rhs.data._num;
			case 0b01:
				return data._num <= rhs.data._int;
			case 0b10:
				return data._int <= rhs.data._num;
			case 0b11:
				return data._int <= rhs.data._int;
			}
		}

		template<typename T>
		bool operator<=(const T &rhs) const noexcept
		{
			if (type)
				return data._int <= rhs;
			else
				return data._num <= rhs;
		}

		bool operator>(const numeric &rhs) const noexcept
		{
			switch (get_composite_type(type, rhs.type)) {
			default:
			case 0b00:
				return data._num > rhs.data._num;
			case 0b01:
				return data._num > rhs.data._int;
			case 0b10:
				return data._int > rhs.data._num;
			case 0b11:
				return data._int > rhs.data._int;
			}
		}

		template<typename T>
		bool operator>(const T &rhs) const noexcept
		{
			if (type)
				return data._int > rhs;
			else
				return data._num > rhs;
		}

		bool operator>=(const numeric &rhs) const noexcept
		{
			switch (get_composite_type(type, rhs.type)) {
			default:
			case 0b00:
				return data._num >= rhs.data._num;
			case 0b01:
				return data._num >= rhs.data._int;
			case 0b10:
				return data._int >= rhs.data._num;
			case 0b11:
				return data._int >= rhs.data._int;
			}
		}

		template<typename T>
		bool operator>=(const T &rhs) const noexcept
		{
			if (type)
				return data._int >= rhs;
			else
				return data._num >= rhs;
		}

		bool operator==(const numeric &rhs) const noexcept
		{
			switch (get_composite_type(type, rhs.type)) {
			default:
			case 0b00:
				return data._num == rhs.data._num;
			case 0b01:
				return data._num == rhs.data._int;
			case 0b10:
				return data._int == rhs.data._num;
			case 0b11:
				return data._int == rhs.data._int;
			}
		}

		template<typename T>
		bool operator==(const T &rhs) const noexcept
		{
			if (type)
				return data._int == rhs;
			else
				return data._num == rhs;
		}

		bool operator!=(const numeric &rhs) const noexcept
		{
			switch (get_composite_type(type, rhs.type)) {
			default:
			case 0b00:
				return data._num != rhs.data._num;
			case 0b01:
				return data._num != rhs.data._int;
			case 0b10:
				return data._int != rhs.data._num;
			case 0b11:
				return data._int != rhs.data._int;
			}
		}

		template<typename T>
		bool operator!=(const T &rhs) const noexcept
		{
			if (type)
				return data._int != rhs;
			else
				return data._num != rhs;
		}

		numeric &operator++() noexcept
		{
			if (type)
				++data._int;
			else
				++data._num;
			return *this;
		}

		numeric &operator--() noexcept
		{
			if (type)
				--data._int;
			else
				--data._num;
			return *this;
		}

		numeric operator++(int) noexcept
		{
			if (type)
				return data._int++;
			else
				return data._num++;
		}

		numeric operator--(int) noexcept
		{
			if (type)
				return data._int--;
			else
				return data._num--;
		}

		numeric operator-() const noexcept
		{
			if (type)
				return -data._int;
			else
				return -data._num;
		}

		bool is_integer() const noexcept
		{
			return type;
		}

		bool is_float() const noexcept
		{
			return !type;
		}

		numeric_integer as_integer() const noexcept
		{
			if (type)
				return data._int;
			else
				return data._num;
		}

		numeric_float as_float() const noexcept
		{
			if (type)
				return data._int;
			else
				return data._num;
		}
	};

// Buffer Pool
	template<typename T, std::size_t blck_size, template<typename> class allocator_t=std::allocator>
	class allocator_type final {
		T *mPool[blck_size];
		allocator_t<T> mAlloc;
		std::size_t mOffset = 0;
	public:
		allocator_type()
		{
			while (mOffset < 0.5 * blck_size)
				mPool[mOffset++] = mAlloc.allocate(1);
		}

		allocator_type(const allocator_type &) = delete;

		~allocator_type()
		{
			while (mOffset > 0)
				mAlloc.deallocate(mPool[--mOffset], 1);
		}

		template<typename...ArgsT>
		inline T *alloc(ArgsT &&...args)
		{
			T *ptr = nullptr;
			if (mOffset > 0 && global_thread_counter == 0)
				ptr = mPool[--mOffset];
			else
				ptr = mAlloc.allocate(1);
			mAlloc.construct(ptr, std::forward<ArgsT>(args)...);
			return ptr;
		}

		inline void free(T *ptr)
		{
			mAlloc.destroy(ptr);
			if (mOffset < blck_size && global_thread_counter == 0)
				mPool[mOffset++] = ptr;
			else
				mAlloc.deallocate(ptr, 1);
		}
	};

	class event_type final {
	public:
		using listener_type = std::function<bool(void *)>;
	private:
		std::deque<listener_type> m_listener;
	public:
		event_type() = delete;

		event_type(const event_type &) = delete;

		explicit event_type(listener_type default_listener)
		{
			m_listener.push_front(std::move(default_listener));
		}

		void add_listener(listener_type listener)
		{
			m_listener.push_front(std::move(listener));
		}

		bool touch(void *arg)
		{
			for (auto &listener: m_listener)
				if (listener(arg))
					return true;
			return false;
		}
	};
}
