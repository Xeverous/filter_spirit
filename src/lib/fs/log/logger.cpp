#include <fs/log/logger.hpp>

#include <utility>

namespace fs::log
{

message_stream::message_stream(severity s, logger& l)
: _logger(&l)
{
	_logger->begin_logging();
	_logger->begin_message(s);
}

message_stream::~message_stream()
{
	if (_logger) {
		_logger->end_message();
		_logger->end_logging();
	}
}

void swap(message_stream& lhs, message_stream& rhs) noexcept
{
	using std::swap;
	swap(lhs._logger, rhs._logger);
}

message_stream& operator<<(message_stream& stream, severity s)
{
	if (stream._logger) {
		stream._logger->end_message();
		stream._logger->begin_message(s);
	}

	return stream;
}

void message_stream::print_line_number(int line_number)
{
	*this << "line " << line_number << ": ";
}

void message_stream::print_underlined_code(utility::underlined_code uc, char underline_char)
{
	auto& self = *this;
	self << uc.code_line << '\n' << uc.indent_view();

	for (auto i = 0u; i < uc.spaces; ++i)
		self << ' ';

	for (auto i = 0u; i < uc.underlines; ++i)
		self << underline_char;

	self << '\n';
}

void message_stream::print_underlined_code(std::string_view surrounding_lines, std::string_view code_to_underline)
{
	utility::code_underliner cu(surrounding_lines, code_to_underline);

	for (auto underlined_code = cu.next(); underlined_code; underlined_code = cu.next()) {
		print_underlined_code(underlined_code);
	}
}

void message_stream::print_pointed_code(std::string_view surrounding_lines, const char* character_to_underline)
{
	print_underlined_code(utility::code_underliner(surrounding_lines, character_to_underline).next(), '^');
}

}
