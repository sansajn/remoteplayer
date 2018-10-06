#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>
#include "log.hpp"

namespace rpl {

namespace logging = boost::log;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

void log_to_console()
{
	logging::add_common_attributes();

	logging::add_console_log(std::clog,
		keywords::format = (
			expr::stream
				<< "[" << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f") << "] "
				<<	"[" << logging::trivial::severity << "] "
				<< expr::smessage
		)  // [2017-12-15 15:48:46.989610] [info]
	);
}

void log_to_file(std::string const & log_file)
{
	logging::add_common_attributes();

	logging::add_file_log(
		keywords::file_name = log_file,
		keywords::auto_flush = true,
//		keywords::open_mode = (std::ios::out|std::ios::app),
		keywords::format = (
			expr::stream
				<< "[" << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f") << "] "
				<< "[" << logging::trivial::severity << "] "
				<< expr::smessage
		)  // [2017-12-15 15:48:46.989610] [info]
	);
}

}  // rpl
