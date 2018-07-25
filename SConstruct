# dependencies:
#    libgstreamer1.0-dev
#    libzmq3-dev

env = Environment(
	CCFLAGS=['-std=c++14', '-Wall', '-g', '-O0'],
	LIBS=[
		'pthread',
		'boost_filesystem',
		'boost_system',
		'boost_thread',
		'boost_log'],
	CPPDEFINES=['BOOST_SPIRIT_THREADSAFE', 'BOOST_LOG_DYN_LINK'],  # json support
	CPPPATH=['libs/']
)

env.ParseConfig('pkg-config --cflags --libs gstreamer-1.0 libzmq')

# static libzmqu library
zmqu_lib = env.StaticLibrary('zmqu', Glob('libs/zmqu/*.cpp'))

objs = env.Object(['helpers.cpp']);

env.Program('rplay', [
	'rplay.cpp',
	'player.cpp',
	'library.cpp',
	zmqu_lib
])

# console client
env.Program('rplayc', ['rplayc.cpp', objs, zmqu_lib])

# graphics client
genv = env.Clone()
genv.ParseConfig('pkg-config gtkmm-3.0 --cflags --libs')

genv.Program('grplay', [
	'grplay.cpp',
	'player_client.cpp',
	objs,
	zmqu_lib
])
