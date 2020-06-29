#pragma once

#include <fs/log/logger.hpp>

#include <string>
#include <vector>

namespace fs::log
{

/**
 * @class buffer logger - stores output in a buffer
 *
 * @details Useful when there is a need to temporarily stop immediate
 * logging - for example when awaiting a thread-safe logger mutex
 * or outputting only in case of problems (as in tests).
 */
class buffer_logger : public logger
{
public:
	struct message
	{
		severity s;
		std::string text;
	};

	void begin_message(severity s) override
	{
		_current_message.s = s;
	}

	void end_message() override
	{
		_buffer.push_back(std::move(_current_message));
		_current_message.text.clear();
	}

	void add(std::string_view text) override
	{
		_current_message.text.append(text);
	}

	void add(char character) override
	{
		_current_message.text.push_back(character);
	}

	void add(std::int64_t number) override
	{
		_current_message.text.append(std::to_string(number));
	}

	void add(std::uint64_t number) override
	{
		_current_message.text.append(std::to_string(number));
	}

	bool has_warnings_or_errors() const
	{
		for (const message& m : _buffer) {
			if (m.s == severity::warning || m.s == severity::error)
				return true;
		}

		return false;
	}

	bool has_errors() const
	{
		for (const message& m : _buffer) {
			if (m.s == severity::error)
				return true;
		}

		return false;
	}

	void dump_to(logger& l) const
	{
		for (const message& m : _buffer)
			l.message(m.s) << m.text;
	}

	      auto& messages()       { return _buffer; }
	const auto& messages() const { return _buffer; }

	void clear()
	{
		_buffer.clear();
	}

private:
	message _current_message;
	std::vector<message> _buffer;
};

}
