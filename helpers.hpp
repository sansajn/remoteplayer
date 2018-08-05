#pragma once
#include <string>
#include "json.hpp"

std::string read_file(std::string const & fname);
void save_file(std::string const & fname, std::string const & data);

jtree read_json_file(std::string const & fname);
void save_json_file(std::string const & fname, jtree const & root);

std::string pwd();

/*! it can be used to notify that certain seconds passed
\code
void update(long position) {
	static watch_alert every_3s{3};
	if (every_3s.update(position))
		;  // do something
}
\endcode */
class watch_alert
{
public:
	watch_alert(int trigger);  //! \param trigger in s
	bool update(long position);  //!< \param dur in ns

private:
	long const _trigger;  //!< in ns
	long _t0;  //! in ns
};
