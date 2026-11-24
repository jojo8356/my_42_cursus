#ifndef EXCEPTION_HPP
# define EXCEPTION_HPP

# include <iostream>
# include <exception>

namespace ft
{
	//////////////////////////////
	// Exception base class
	//////////////////////////////

	class exception {
	public:
				exception						(void)					throw()		{};
				exception						(const exception &)		throw()		{};
		virtual ~exception						(void)					throw()		{};

				exception &			operator=	(const exception &)		throw()		{ return (*this); };
		virtual const char*	what		(void) const			throw()		{ return "ft::exception"; };
	};

	//////////////////////////////
	// Out of range
	//////////////////////////////

	class out_of_range : public ft::exception, public std::out_of_range {
	public:
			explicit out_of_range		(const std::string & what_arg) : ft::exception(), std::out_of_range(what_arg) {};
		virtual ~out_of_range			(void)			throw()		{};
		virtual const char*	what(void) const throw()		{ return std::out_of_range::what(); };
	};

	//////////////////////////////
	// Length error
	//////////////////////////////

	class length_error : public ft::exception, public std::length_error {
	public:
			explicit length_error		(const std::string & what_arg) : ft::exception(), std::length_error(what_arg) {};
		virtual ~length_error			(void)			throw()		{};
		virtual const char*	what(void) const throw()		{ return std::length_error::what(); };
	};
}

#endif
