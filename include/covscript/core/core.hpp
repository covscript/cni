#pragma once
/*
* Covariant Script Programming Language
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
*
* Namespaces:
* cs: Main Namespace
* cs_impl: Implement Namespace
*/
// LibDLL
#include <covscript/import/libdll/dll.hpp>
// STL
#include <unordered_map>
#include <unordered_set>
#include <type_traits>
#include <functional>
#include <typeindex>
#include <exception>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <istream>
#include <ostream>
#include <utility>
#include <cstring>
#include <atomic>
#include <cctype>
#include <string>
#include <vector>
#include <memory>
#include <deque>
#include <list>
// CovScript ABI Version
// Must be different to SDK
#define COVSCRIPT_ABI_VERSION 990307
// CovScript Headers
#include <covscript/core/components.hpp>
#include <covscript/core/definition.hpp>
#include <covscript/core/variable.hpp>

namespace cs {
// Process Context
	class process_context final {
		std::atomic<bool> is_sigint_raised{};
	public:
// Exit code
		int exit_code = 0;
// Event Handling
		static bool on_process_exit_default_handler(void *);

		event_type on_process_exit;

// DO NOT TOUCH THIS EVENT DIRECTLY!!
		event_type on_process_sigint;

		inline void poll_event()
		{
			if (is_sigint_raised) {
				is_sigint_raised = false;
				on_process_sigint.touch(nullptr);
			}
		}

		inline void raise_sigint()
		{
			is_sigint_raised = true;
		}

// Exception Handling
		static void cs_defalt_exception_handler(const lang_error &e)
		{
			throw e;
		}

		static void std_defalt_exception_handler(const std::exception &e)
		{
			throw forward_exception(e.what());
		}

		std_exception_handler std_eh_callback = &std_defalt_exception_handler;
		cs_exception_handler cs_eh_callback = &cs_defalt_exception_handler;

		process_context() : on_process_exit(&on_process_exit_default_handler), on_process_sigint(&on_process_exit_default_handler)
		{
			is_sigint_raised = false;
		}
	};

	extern process_context this_process;
	extern process_context *current_process;

	// Callable and Function
	class callable final {
	public:
		using function_type = std::function<var(vector &)>;
		enum class types {
			normal, request_fold, member_fn, member_visitor, force_regular
		};
	private:
		function_type mFunc;
		types mType = types::normal;
	public:
		callable() = delete;

		callable(const callable &) = default;

		explicit callable(function_type func, types type = types::normal) : mFunc(std::move(func)), mType(type) {}

		bool is_request_fold() const
		{
			return mType == types::request_fold;
		}

		bool is_member_fn() const
		{
			return mType == types::member_fn;
		}

		types type() const
		{
			return mType;
		}

		var call(vector &args) const
		{
			return mFunc(args);
		}

		const function_type &get_raw_data() const
		{
			return mFunc;
		}
	};

// Copy
	void copy_no_return(var &);

	var copy(var);

// Move Semantics
	var lvalue(const var &);

	var rvalue(const var &);

	var try_move(const var &);

	template<typename... ArgsT>
	static var invoke(const var &func, ArgsT &&... _args)
	{
		if (func.type() == typeid(callable)) {
			vector args{std::forward<ArgsT>(_args)...};
			return func.const_val<callable>().call(args);
		}
		else
			throw runtime_error("Invoke non-callable object.");
	}

	struct pointer final {
		var data;

		pointer() = default;

		explicit pointer(var v) : data(std::move(v)) {}

		bool operator==(const pointer &ptr) const
		{
			return data.is_same(ptr.data);
		}
	};

	static const pointer null_pointer = {};

	struct domain_ref final {
		domain_type *domain = nullptr;

		domain_ref(domain_type *ptr) : domain(ptr) {}
	};

	class var_id final {
		friend class domain_type;

		friend class domain_manager;

		mutable std::size_t m_domain_id = 0, m_slot_id = 0;
		mutable std::shared_ptr<domain_ref> m_ref;
		std::string m_id;
	public:
		var_id() = delete;

		var_id(std::string name) : m_id(std::move(name)) {}

		var_id(const var_id &) = default;

		var_id(var_id &&) noexcept = default;

		var_id &operator=(const var_id &) = default;

		var_id &operator=(var_id &&) = default;

		inline void set_id(const std::string &id)
		{
			m_id = id;
		}

		inline const std::string &get_id() const noexcept
		{
			return m_id;
		}

		inline operator std::string &() noexcept
		{
			return m_id;
		}

		inline operator const std::string &() const noexcept
		{
			return m_id;
		}
	};

	class domain_type final {
		map_t<std::string, std::size_t> m_reflect;
		std::shared_ptr<domain_ref> m_ref;
		std::vector<var> m_slot;
		bool optimize = false;

		inline std::size_t get_slot_id(const std::string &name) const
		{
			if (m_reflect.count(name) > 0)
				return m_reflect.at(name);
			else
				throw runtime_error("Use of undefined variable \"" + name + "\".");
		}

	public:
		domain_type() : m_ref(std::make_shared<domain_ref>(this)) {}

		domain_type(const domain_type &domain) : m_reflect(domain.m_reflect), m_ref(std::make_shared<domain_ref>(this)),
			m_slot(domain.m_slot) {}

		domain_type(domain_type &&domain) noexcept: m_ref(std::make_shared<domain_ref>(this))
		{
			std::swap(m_reflect, domain.m_reflect);
			std::swap(m_slot, domain.m_slot);
		}

		~domain_type()
		{
			m_ref->domain = nullptr;
		}

		void clear()
		{
			m_reflect.clear();
			m_slot.clear();
			optimize = false;
		}

		inline void next() noexcept
		{
			optimize = true;
		}

		inline bool consistence(const var_id &id) const noexcept
		{
			return id.m_ref == m_ref;
		}

		inline bool exist(const std::string &name) const noexcept
		{
			return m_reflect.count(name) > 0;
		}

		inline bool exist(const var_id &id) const noexcept
		{
			return m_reflect.count(id.m_id) > 0;
		}

		domain_type &add_var(const std::string &name, const var &val)
		{
			if (m_reflect.count(name) == 0) {
				m_slot.push_back(val);
				m_reflect.emplace(name, m_slot.size() - 1);
			}
			else
				m_slot[m_reflect[name]] = val;
			return *this;
		}

		domain_type &add_var(const var_id &id, const var &val)
		{
			if (m_reflect.count(id.m_id) == 0) {
				m_slot.push_back(val);
				m_reflect.emplace(id.m_id, m_slot.size() - 1);
				id.m_slot_id = m_slot.size() - 1;
				id.m_ref = m_ref;
			}
			else {
				if (id.m_ref != m_ref) {
					id.m_slot_id = m_reflect[id.m_id];
					id.m_ref = m_ref;
				}
				m_slot[id.m_slot_id] = val;
			}
			return *this;
		}

		bool add_var_optimal(const std::string &name, const var &val, bool override = false)
		{
			if (m_reflect.count(name) > 0) {
				if (optimize) {
					m_slot[m_reflect.at(name)] = val;
					return true;
				}
				else if (override) {
					add_var(name, val);
					return true;
				}
				else
					return false;
			}
			else {
				add_var(name, val);
				return true;
			}
		}

		bool add_var_optimal(const var_id &id, const var &val, bool override = false)
		{
			if (id.m_ref == m_ref) {
				if (optimize) {
					m_slot[id.m_slot_id] = val;
					return true;
				}
				else if (override) {
					add_var(id, val);
					return true;
				}
				else
					return false;
			}
			else {
				add_var(id, val);
				return true;
			}
		}

		var &get_var(const var_id &id)
		{
			if (id.m_ref != m_ref) {
				id.m_slot_id = get_slot_id(id.m_id);
				id.m_ref = m_ref;
			}
			return m_slot[id.m_slot_id];
		}

		const var &get_var(const var_id &id) const
		{
			if (id.m_ref != m_ref) {
				id.m_slot_id = get_slot_id(id.m_id);
				id.m_ref = m_ref;
			}
			return m_slot[id.m_slot_id];
		}

		var &get_var(const std::string &name)
		{
			if (m_reflect.count(name) > 0)
				return m_slot[m_reflect.at(name)];
			else
				throw runtime_error("Use of undefined variable \"" + name + "\".");
		}

		const var &get_var(const std::string &name) const
		{
			if (m_reflect.count(name) > 0)
				return m_slot[m_reflect.at(name)];
			else
				throw runtime_error("Use of undefined variable \"" + name + "\".");
		}

		var &get_var_no_check(const var_id &id) noexcept
		{
			if (id.m_ref != m_ref) {
				id.m_slot_id = m_reflect.at(id.m_id);
				id.m_ref = m_ref;
			}
			return m_slot[id.m_slot_id];
		}

		const var &get_var_no_check(const var_id &id) const noexcept
		{
			if (id.m_ref != m_ref) {
				id.m_slot_id = m_reflect.at(id.m_id);
				id.m_ref = m_ref;
			}
			return m_slot[id.m_slot_id];
		}

		var &get_var_no_check(const var_id &id, std::size_t domain_id) noexcept
		{
			id.m_domain_id = domain_id;
			if (id.m_ref != m_ref) {
				id.m_slot_id = m_reflect.at(id.m_id);
				id.m_ref = m_ref;
			}
			return m_slot[id.m_slot_id];
		}

		inline var &get_var_no_check(const std::string &name) noexcept
		{
			return m_slot[m_reflect.at(name)];
		}

		inline const var &get_var_no_check(const std::string &name) const noexcept
		{
			return m_slot[m_reflect.at(name)];
		}

		inline auto begin() const
		{
			return m_reflect.cbegin();
		}

		inline auto end() const
		{
			return m_reflect.cend();
		}

		// Caution! Only use for traverse!
		inline var &get_var_by_id(std::size_t id)
		{
			return m_slot[id];
		}

		inline const var &get_var_by_id(std::size_t id) const
		{
			return m_slot[id];
		}
	};

// Namespace and extensions
	class name_space {
		domain_type *m_data = nullptr;
		bool is_ref = false;
	public:
		name_space() : m_data(new domain_type) {}

		name_space(const name_space &ns) : m_data(new domain_type)
		{
			copy_namespace(ns);
		}

		explicit name_space(domain_type dat) : m_data(new domain_type(std::move(dat))) {}

		explicit name_space(domain_type *dat) : m_data(dat), is_ref(true) {}

		virtual ~name_space()
		{
			if (!is_ref)
				delete m_data;
		}

		name_space &add_var(const std::string &name, const var &var)
		{
			m_data->add_var(name, var);
			return *this;
		}

		name_space &add_var(const var_id &id, const var &var)
		{
			m_data->add_var(id, var);
			return *this;
		}

		var &get_var(const std::string &name)
		{
			return m_data->get_var(name);
		}

		const var &get_var(const std::string &name) const
		{
			return m_data->get_var(name);
		}

		var &get_var(const var_id &id)
		{
			return m_data->get_var(id);
		}

		const var &get_var(const var_id &id) const
		{
			return m_data->get_var(id);
		}

		domain_type &get_domain() const
		{
			return *m_data;
		}

		inline void copy_namespace(const name_space &ns)
		{
			copy_domain(*ns.m_data);
		}

		void copy_domain(const domain_type &domain)
		{
			for (auto &it: domain)
				m_data->add_var(it.first, domain.get_var_by_id(it.second));
		}

		name_space &operator=(const name_space &ns)
		{
			if (&ns != this) {
				m_data->clear();
				copy_namespace(ns);
			}
			return *this;
		}
	};

// Internal Garbage Collection
	template<typename T>
	class garbage_collector final {
		set_t<T *> table;
	public:
		garbage_collector() = default;

		garbage_collector(const garbage_collector &) = delete;

		~garbage_collector()
		{
			collect();
		}

		void collect()
		{
			for (auto &ptr: table)
				::operator delete(ptr);
			table.clear();
		}

		void add(void *ptr)
		{
			table.emplace(static_cast<T *>(ptr));
		}

		void remove(void *ptr)
		{
			table.erase(static_cast<T *>(ptr));
		}
	};

	namespace dll_resources {
		constexpr char dll_compatible_check[] = "__CS_ABI_COMPATIBLE__";
		constexpr char dll_main_entrance[] = "__CS_EXTENSION_MAIN__";

		typedef int(*dll_compatible_check_t)();

		typedef void(*dll_main_entrance_t)(name_space *, process_context *);
	}

	class extension final : public name_space {
	public:
		static garbage_collector<cov::dll> gc;

		extension() = delete;

		extension(const extension &) = delete;

		inline static int truncate(int n, int m)
		{
			return n == 0 ? 0 : n / int(std::pow(10, (std::max)(int(std::log10(std::abs(n))) - (std::max)(m, 0) + 1, 0)));
		}

		explicit extension(const std::string &path)
		{
			using namespace dll_resources;
			cov::dll *dll = new cov::dll(path);
			gc.add(dll);
			dll_compatible_check_t dll_check = reinterpret_cast<dll_compatible_check_t>(dll->get_address(
			                                       dll_compatible_check));
			if (dll_check == nullptr || truncate(dll_check(), 4) != truncate(COVSCRIPT_ABI_VERSION, 4))
				throw runtime_error("Incompatible Covariant Script Extension.(Target: " + std::to_string(dll_check()) +
				                    ", Current: " + std::to_string(COVSCRIPT_ABI_VERSION) + ")");
			dll_main_entrance_t dll_main = reinterpret_cast<dll_main_entrance_t>(dll->get_address(dll_main_entrance));
			if (dll_main != nullptr) {
				dll_main(this, current_process);
			}
			else
				throw runtime_error("Broken Covariant Script Extension.");
		}
	};

	var make_namespace(const namespace_t &);

	template<typename T, typename...ArgsT>
	static namespace_t make_shared_namespace(ArgsT &&...args)
	{
		return std::make_shared<T>(std::forward<ArgsT>(args)...);
	}
}
