#include <memory>
#include <filesystem>
#include <vector>
#include <cassert>
#include <iostream>
#include "rplib/log.hpp"
#include "youtube_dl.hpp"

using std::vector;
using std::unique_ptr;
using std::filesystem::path,
	std::filesystem::directory_iterator;
using std::string;
using std::clog, std::endl;

// python
template <typename R>
R dict_get(PyObject * obj, char const * key);

template <>
long dict_get<long>(PyObject * obj, char const * key)
{
	return PyLong_AsLong(PyDict_GetItemString(obj, key));
}

template <>
long long dict_get<long long>(PyObject * obj, char const * key)
{
	return PyLong_AsLongLong(PyDict_GetItemString(obj, key));
}

// TODO: throw key_not_found exception (out_of_range) in case key not found
template <>
string dict_get<string>(PyObject * obj, char const * key)
{
	PyObject * utf8_str = PyDict_GetItemString(obj, key);
	assert(utf8_str);
	assert(PyUnicode_Check(utf8_str));

	PyObject * str = PyUnicode_AsUTF8String(utf8_str);
	assert(str && PyString_Check(str));

	char const * buf = PyString_AsString(str);
	assert(buf);

	string result = buf;

	Py_DECREF(str);

	return result;
}

static bool locate_downloaded_media(string const & filename_hint, string & filename);

/*! Find similar files in directory.
\param[in] directory Directory to search.
\param[in] filename Filename without an extension. */
static vector<path> find_similar_files(path const & directory, path const & filename);

static int injected_quit(void *);  // TODO: what is that? remove if not used.
static void python_decref(PyObject * p) {Py_DECREF(p);}


struct download_progress
{
	string status,
		filename,
		tmpfilename;
	long long downloaded_bytes,
		total_bytes;
};

static youtube_dl * __ydl = nullptr;  // current ydl instance TODO: get rid of this
static download_progress __current_download;

/*! \param[in] args dictionary with the entries
	* status: One of "downloading", "error", or "finished".
		Check this first and ignore unknown values.

	If status is one of "downloading", or "finished", the
	following properties may also be present:
	* filename: The final filename (always present)
	* tmpfilename: The filename we're currently writing to
	* downloaded_bytes: Bytes on disk
	* total_bytes: Size of the whole file, None if unknown
	* total_bytes_estimate: Guess of the eventual file size,
		None if unavailable.
	* elapsed: The number of seconds since download started.
	* eta: The estimated time in seconds, None if unknown
	* speed: The download speed in bytes/second, None if
		unknown
	* fragment_index: The counter of the currently
		downloaded video fragment.
	* fragment_count: The number of fragments (= individual
		files that will be merged) */
static PyObject * native_progress_hooks(PyObject * self, PyObject * args);

static PyObject * native_ffmpeg_location(PyObject * self, PyObject *);
static PyObject * native_filesystem_output(PyObject * self, PyObject *);

// TODO: rename filesystem_output to outtmpl (youtube-dl)
PyMethodDef native_methods[] = {
	{"progress_hooks", native_progress_hooks, METH_VARARGS, "Implements youtube-dl progress_hooks method."},
	{"ffmpeg_location", native_ffmpeg_location, METH_NOARGS, "Return ffmpeg location (defined in config file)"},
	{"filesystem_output", native_filesystem_output, METH_NOARGS, "Return default output path"},
	{NULL, NULL, 0, NULL}
};

bool youtube_dl::download_audio(std::string const & url, std::string & filename)
{
	if (download_with("download_audio", url))
		return locate_downloaded_media(__current_download.filename, filename);
	else
		return false;
}

void youtube_dl::notify_progress_update(std::string const & media_id, size_t downloaded_bytes,
	size_t total_bytes)
{
	_progress_updater.call(media_id, downloaded_bytes, total_bytes);
}

bool youtube_dl::download_with(string const download_function, string const & url)
{
	PyObject * funct = PyDict_GetItemString(_env, download_function.c_str());  // borrowed reference, do not decref
	assert(funct && "function doesn't exist");

	// TODO: define type with deleter
	unique_ptr<PyObject, decltype(&python_decref)> args{
		PyTuple_Pack(1, PyString_FromString(url.c_str())), &python_decref};  // prepare function's arguments
	assert(args.get());

	if (PyCallable_Check(funct))
	{
		// TODO: define type with deleter
		unique_ptr<PyObject, decltype(&python_decref)> result{
			PyObject_CallObject(funct, args.get()), &python_decref};  // call python function

		if (result)
			return true;
		else
		{
			PyErr_Print();  // TODO: print to exception message
			return false;
		}
	}
	else
	{
		PyErr_Print();  // TODO: print to exception message
		return false;
	}
}

void youtube_dl::load_module(string const & module)
{
	_module = PyImport_Import(PyString_FromString(module.c_str()));
	assert(_module && "unable to import/open python module");
	// TODO: show error message, NULL with an exception set on failure

	_env = PyModule_GetDict(_module);  // borrowed reference
	assert(_env);
}

youtube_dl::youtube_dl(progress_update_event & progress_updater, int argc,
	char * argv[], string const & outtmpl, string const & python_home)
		: _module{nullptr}
		, _env{nullptr}
		, _python_home{python_home}
		, _outtmpl{outtmpl}
		, _progress_updater{progress_updater}
{
	if (!_python_home.empty())
		Py_SetPythonHome(const_cast<char *>(_python_home.c_str()));

	Py_SetProgramName(argv[0]);
	Py_Initialize();
	PySys_SetArgv(argc, argv);

	PyObject * native = Py_InitModule("native", native_methods);  // extend by native module
	clog << "native=" << (void *)native << endl;

	// TODO: store native with this pointer for later use in native methods

	load_module("download");  // loads ./download.py

	assert(!__ydl && "instance already used");
	__ydl = this;
}

youtube_dl::~youtube_dl()
{
	__ydl = nullptr;
	Py_DECREF(_module);
	Py_Finalize();
}

int injected_quit(void *)
{
	clog << "hello from injected_quit" << endl;
	PyErr_SetInterrupt();  // abort python script execution
	return -1;
}

void youtube_dl::abort()
{
	clog << "abort()" << endl;
	PyGILState_STATE gil = PyGILState_Ensure();
	Py_AddPendingCall(injected_quit, NULL);
	PyGILState_Release(gil);
	clog << "abort(), done" << endl;
}

PyObject * native_progress_hooks(PyObject * self, PyObject * args)
{
	LOG(trace) << "progress_hooks(self=" << (void *)self << ", args="
		<< (void *)args << ")" << endl;

	assert(PyTuple_Check(args) && PyTuple_Size(args) > 0);

	PyObject * progress = PyTuple_GetItem(args, 0);  // borrowed reference
	assert(PyDict_Check(progress) && "dictionary expected");

	// extract data
	string status = dict_get<string>(progress, "status");
	long long downloaded_bytes = dict_get<long long>(progress, "downloaded_bytes");
	long long total_bytes = dict_get<long long>(progress, "total_bytes");
	string filename = dict_get<string>(progress, "filename");
	string tmpfilename;

/*
	clog << "\n\n"
		<< "status=" << status << "\n"
		<< "filename=" << filename << "\n"
		<< "downloaded=" << downloaded_bytes << "bytes\n"
		<< "total=" << total_bytes << "bytes" << endl;
*/

	if (status != "finished")
		tmpfilename = dict_get<string>(progress, "tmpfilename");
	// else (status == "downloading") {}

	// update current download stats
	__current_download = {
		status,
		filename,
		tmpfilename,
		downloaded_bytes,
		total_bytes
	};

	// in case we are done, lets wait for "finished" event to notify we are done
	if (downloaded_bytes < total_bytes || status != "downloading")
		__ydl->notify_progress_update(path{filename}.stem().string(), (size_t)downloaded_bytes,
			(size_t)total_bytes);

	Py_RETURN_NONE;
}

PyObject * native_ffmpeg_location(PyObject * self, PyObject *)
{
//	config_file const & conf = default_config();

//	if (!conf.ffmpeg_home.empty())
//		return Py_BuildValue("s", conf.ffmpeg_home.c_str());
//	else
//		Py_RETURN_NONE;

	Py_RETURN_NONE;
}

PyObject * native_filesystem_output(PyObject * self, PyObject *)
{
//	config_file const & conf = default_config();

//	if (!conf.filesystem_output.empty())
//		return Py_BuildValue("s", conf.filesystem_output.c_str());
//	else
//		return Py_BuildValue("s", "%(title)s-%(id)s.%(ext)s");

	LOG(trace) << "filesystem_output(self=" << (void *)self << ")" << endl;

	assert(__ydl);
	return PyUnicode_FromString(__ydl->outtmpl().c_str());
}

bool locate_downloaded_media(string const & filename_hint, string & filename)
{
	path p = filename_hint;
	vector<path> candidates = find_similar_files(p.parent_path(), p.stem());

	if (size(candidates) == 1)
	{
		filename = candidates[0];
		return true;
	}
	else if (size(candidates) > 1) // select the first one not with '.webm' extension
	{
		for (path const & p : candidates)
		{
			if (p.extension() != ".webm")
			{
				filename = p.string();
				return true;
			}
		}
		return false;
	}
	else
		return false;  // nothing found
}

vector<path> find_similar_files(path const & directory, path const & filename)
{
	vector<path> result;
	for (auto const & e : directory_iterator{directory})
	{
		if (e.path().stem() == filename)
			result.push_back(e.path());
	}
	return result;
}
