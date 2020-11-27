#pragma once

#include <fs/network/download.hpp>

#include <boost/date_time/posix_time/posix_time_duration.hpp>

namespace fs::gui {

class networking
{
public:
	void draw_interface();

	const auto& download_settings() const { return _download_settings; }
	      auto& download_settings()       { return _download_settings; }

	boost::posix_time::time_duration max_market_data_age() const
	{
		return boost::posix_time::minutes(_max_market_data_age_min);
	}

private:
	network::download_settings _download_settings;
	int _max_market_data_age_min = 120;
};

}
