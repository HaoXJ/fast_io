#pragma once

#include<locale>

namespace fast_io
{

template<stream stm>
requires (std::same_as<typename stm::char_type,char>||std::same_as<typename stm::char_type,wchar_t>)
class cpp_locale_wrapper
{
public:
	using char_type = typename stm::char_type;
	using native_handle_type = stm;
	using status_type = std::locale;
	fast_io_basic_streambuf<native_handle_type> buf;
	template<typename ...Args>
	requires std::constructible_from<fast_io_basic_streambuf<native_handle_type>,Args...>
	cpp_locale_wrapper(Args&& ...args):buf(std::forward<Args>(args)...){}
	auto* rdbuf() noexcept
	{
		return std::addressof(buf);
	}
	auto* rdbuf() const noexcept
	{
		return std::addressof(buf);
	}

	auto getloc() const
	{
		return buf.getloc();
	}

	auto imbue(std::locale& loc) const
	{
		return buf.imbue(loc);
	}
/*
	auto& get_ios_base() noexcept
	{
		return iob;
	}
	auto& get_ios_base() const noexcept
	{
		return iob;
	}
*/
	auto& native_handle() const noexcept
	{
		return buf.native_handle();
	}
	auto& native_handle() noexcept
	{
		return buf.native_handle();
	}
};
/*
template<stream stm>
requires (std::same_as<typename stm::char_type,char>||std::same_as<typename stm::char_type,wchar_t>)&&requires(stm& sm)
{
	{sm.getloc()}->std::convertible_to<std::locale>;
}
class cpp_locale_wrapper<stm>
{
public:
	using char_type = typename stm::char_type;
	using native_handle_type = stm;
	using status_type = std::locale;
	native_handle_type handle;
	decltype(auto) getloc()
	{
		return handle.getloc();
	}
	decltype(auto) getloc() const
	{
		return handle.getloc();
	}
	auto& native_handle() const noexcept
	{
		return handle;
	}
	auto& native_handle() noexcept
	{
		return handle;
	}
};
*/

template<output_stream stm,std::contiguous_iterator Iter>
requires (std::same_as<std::iter_value_t<Iter>,char>)||
(std::same_as<typename stm::char_type,std::iter_value_t<Iter>>)
inline void write(cpp_locale_wrapper<stm>& oum,Iter begin,Iter end)
{
	if constexpr(std::same_as<std::iter_value_t<Iter>,typename stm::char_type>)
		return write(oum.native_handle(),begin,end);
	else
	{
		reserve_write(oum.native_handle(),end-begin,[&](wchar_t* out_iter)
		{
			return std::use_facet<std::ctype<typename stm::char_type>>
				(oum.getloc()).
				widen(std::to_address(begin),std::to_address(end),
				out_iter)-std::to_address(begin)+out_iter;
		});
	}
}

template<input_stream stm,std::contiguous_iterator Iter>
requires (std::same_as<std::iter_value_t<Iter>,char>)||
(std::same_as<typename stm::char_type,std::iter_value_t<Iter>>)
inline Iter read(cpp_locale_wrapper<stm>& inm,Iter begin,Iter end)
{
	if constexpr(std::same_as<std::iter_value_t<Iter>,typename stm::char_type>)
		return read(inm.native_handle(),begin,end);
	else if constexpr(std::same_as<typename stm::char_type,char>)
	{
		reserve_read(inm.native_handle(),end-begin,[&](char* inb,char* eb)
		{
			begin+=inm.getloc().widen(inb,eb,std::to_address(begin))-std::to_address(begin);
		});
		return begin;
	}
}

template<output_stream stm,typename T>
inline void cpp_locale_print_define(cpp_locale_wrapper<stm>& oum,
std::basic_ostream<typename stm::char_type> &sbf,
std::locale& loc,T&& ref)
{
	if constexpr(std::integral<T>||std::floating_point<T>)
	{
		if constexpr(std::same_as<std::remove_cvref_t<T>,float>)
			sbf.precision(8);
		else if constexpr(std::same_as<std::remove_cvref_t<T>,double>)
			sbf.precision(17);
		else if constexpr (std::same_as<std::remove_cvref_t<T>,long double>)
			sbf.precision(35);
		std::use_facet<std::num_put<char>>(loc).
		put(oum.rdbuf(), sbf, '0', 2.71);
//		f.put(std::ostreambuf_iterator<char>(sbf),
//			static_cast<std::ios_base&>(sbf),
//			' ',5);
//		f.put(iter,sbf,0,ref);
//f.put(std::ostreambuf_iterator<char>(sbf), static_cast<std::ios_base&>(sbf), ' ', 5);
//			sbf,static_cast<typename stm::char_type>(0),
//			ref);
	}
/*	else if constexpr(requires(std::basic_ostream<typename stm::char_type>& out)
	{
		out<<std::forward<T>(ref);
	})
	{
		std::basic_ostream<typename stm::char_type> out{oum.rdbuf()};
		out<<std::forward<T>(ref);
	}*/
	else
	{
		print(oum.native_handle(),std::forward<T>(ref));
	}
}

template<output_stream stm,typename... Args>
inline void print_status_define(cpp_locale_wrapper<stm>& oum,Args&& ...args)
{
	std::basic_ostream<typename stm::char_type> sbf(oum.rdbuf());
	auto loc{oum.getloc()};
	(cpp_locale_print_define(oum,sbf,loc,std::forward<Args>(args)),...);
}

template<output_stream stm,typename... Args>
inline void println_status_define(cpp_locale_wrapper<stm>& oum,Args&& ...args)
{
	std::basic_ostream<typename stm::char_type> sbf(oum.rdbuf());
	auto loc{oum.getloc()};
	(cpp_locale_print_define(oum,sbf,loc,std::forward<Args>(args)),...);
	oum.buf.sputc(u8'\n');
}

}