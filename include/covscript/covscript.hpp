#pragma once
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
#include <covscript/core/cni.hpp>

namespace cs_function_invoker_impl {
	template<typename T>
	struct convert_helper {
		static inline const T &get_val(const cs::var &val)
		{
			return val.const_val<T>();
		}
	};

	template<typename T>
	struct convert_helper<const T &> {
		static inline const T &get_val(const cs::var &val)
		{
			return val.const_val<T>();
		}
	};

	template<typename T>
	struct convert_helper<T &> {
		static inline T &get_val(const cs::var &val)
		{
			return val.val<T>();
		}
	};

	template<>
	struct convert_helper<void> {
		static inline void get_val(const cs::var &) {}
	};

	template<typename>
	class function_invoker;

	template<typename RetT, typename...ArgsT>
	class function_invoker<RetT(ArgsT...)> {
		cs::var m_func;
	public:
		function_invoker() = default;

		function_invoker(const function_invoker &) = default;

		function_invoker &operator=(const function_invoker &) = default;

		explicit function_invoker(cs::var func) : m_func(std::move(func)) {}

		void assign(const cs::var &func)
		{
			m_func = func;
		}

		cs::var target() const
		{
			return m_func;
		}

		template<typename...ElementT>
		RetT operator()(ElementT &&...args) const
		{
			return convert_helper<RetT>::get_val(cs::invoke(m_func, cs_impl::type_convertor<ElementT, ArgsT>::convert(
			        std::forward<ElementT>(args))...));
		}
	};
}

namespace cs {
    using cs_function_invoker_impl::function_invoker;
}