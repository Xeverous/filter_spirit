#include "fs/log/logger.hpp"

namespace fs
{

logger_wrapper::~logger_wrapper()
{
	logger.end_message();
}

logger_wrapper logger::info()
{
	begin_info_message();
	return logger_wrapper(*this);
}

logger_wrapper logger::warning()
{
	begin_warning_message();
	return logger_wrapper(*this);
}

logger_wrapper logger::error()
{
	begin_error_message();
	return logger_wrapper(*this);
}

}
