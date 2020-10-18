/*! \file youtube_dl.hpp
youtube-dl wrapper and helpers */

#pragma once
#include <string>
#include <Python.h>
#include "event.hpp"

using progress_update_event = event<std::string, size_t, size_t>;

/*! youtube-dl wrapper implementation
\note all work is delegated to `download.py` script
\note not thread safe
\note one instance can exist at time */
class youtube_dl
{
public:
	/*! \note to work relative imports we need to provide argv[0] */
	youtube_dl(progress_update_event & progress_updater, int argc, char * argv[],
		std::string const & outtmpl = "", std::string const & python_home = std::string{});

	~youtube_dl();

	/*! download media as audio
	\param [in] url
	\param [out] filename The final downloaded file name.
	\return returns true on successfull download with the final name of downloaded file as `filename` */
	bool download_audio(std::string const & url, std::string & filename);

	void abort();  // TODO: not working

	std::string const & outtmpl() const {return _outtmpl;}

	/*! \param[in] media_id Media identifier (file name without an extension) */
	void notify_progress_update(std::string const & media_id, size_t downloaded_bytes,
		size_t total_bytes);

private:
	void load_module(std::string const & module);
	void download(std::string const & function_name, std::string const & url);
	bool download_with(std::string const download_function, std::string const & url);

	PyObject * _module,
		* _env;  // __dict__
	std::string const _python_home,  //!< \note needs to be stored in a static storage (according doc)
		_outtmpl;
	progress_update_event & _progress_updater;
};
