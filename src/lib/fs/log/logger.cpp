#include <fs/log/logger.hpp>

#include <utility>

namespace fs::log
{

message_stream::message_stream(severity s, logger& log)
: log(&log)
{
	log.begin_message(s);
}

message_stream::~message_stream()
{
	if (log)
		log->end_message();
}

void swap(message_stream& lhs, message_stream& rhs) noexcept
{
	using std::swap;
	swap(lhs.log, rhs.log);
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

void message_stream::print_underlined_code(std::string_view all_code, std::string_view code_to_underline)
{
	utility::code_underliner cu(all_code, code_to_underline);

	for (auto underlined_code = cu.next(); underlined_code; underlined_code = cu.next()) {
		print_underlined_code(underlined_code);
	}
}

void message_stream::print_pointed_code(std::string_view all_code, const char* character_to_underline)
{
	print_underlined_code(utility::code_underliner(all_code, character_to_underline).next(), '^');
}

}
