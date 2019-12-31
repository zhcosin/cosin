
#ifndef __COSIN_EXCEPTION_H_
#define __COSIN_EXCEPTION_H_

#include <exception>
#include <string>
#include "boost/exception/exception.hpp"
#include "boost/operators.hpp"

namespace cosin
{

template<typename _error_code_type>
class exception : public virtual std::exception,
				  public virtual boost::exception
{
template<typename _type>
class error_code : boost::equality_comparable<error_code<_type> >
{
public:
	error_code(const _type& _code, const std::string& _description)
		: code(_code), description(_description) {}
public:
	_type get_code() const throw()
	{
		return this->code;
	}
	std::string get_description() const throw()
	{
		return this->description;
	}
public:
	bool operator==(const error_code<_type>& other) const throw()
	{
		return this->code == other.code;
	}
private:
	_type			code;
	std::string		description;
};

public:
class tracor
{
public:
	tracor() : line_num(-1) {}
	tracor(const std::string& filename, const std::string& funname, int line)
		: file_name(filename), function_name(funname), line_num(line) {}
public:
	std::string get_file_name() const throw()
	{
		return this->file_name;
	}
	std::string get_function_name() const throw()
	{
		return this->function_name;
	}
	int get_line_number() const throw()
	{
		return this->line_num;
	}
private:
	std::string		file_name;
	std::string		function_name;
	int				line_num;
};

public:
typedef boost::error_info<struct tag_trace_info, tracor>					trace_info;
typedef boost::error_info<struct tag_exception_int_code, error_code<_error_code_type> >	exception_code;
typedef boost::error_info<struct tag_error_description, std::string>		exception_description;

public:
	exception(const error_code<_error_code_type>& err_code, const tracor& trace)
	{
		*this << exception_code(err_code) << trace_info(trace);
	}

	exception(const error_code<_error_code_type>& err_code, const tracor& trace, const std::string& description)
	{
		*this << exception_code(err_code) << trace_info(trace) << exception_description(description);
	}

	virtual ~exception() throw() {}

public:
	virtual std::string get_exception_name() const throw()
	{
		return "cosin::exception";
	}

	const error_code<_error_code_type>& get_error_code() const throw()
	{
		return *boost::get_error_info<exception_code>(*this);
	}

	const tracor& get_tracor() const throw()
	{
		return *boost::get_error_info<trace_info>(*this);
	}

	const std::string& get_exception_description() const throw()
	{
		return *boost::get_error_info<exception_description>(*this);
	}

	std::string to_string() const throw()
	{
		const error_code<_error_code_type>& the_err_code = this->get_error_code();
		const tracor& the_tracor = this->get_tracor();

		std::stringstream ss;
		ss << "file: " << the_tracor.get_file_name() << "\n"
			<< "line: " << the_tracor.get_line_number() << "\n"
			<< "function: " << the_tracor.get_function_name() << "\n"
			<< "error code: " << the_err_code.get_code() << "\n"
			<< "error code description: " << the_err_code.get_description() << "\n"
			<< "exception description: " << this->get_exception_description() << "\n"
			<< "Diagnostic information: " << this->diagnostic_information(*this) << "\n";

		return ss.str();
	}
};

}

#endif
