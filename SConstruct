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

zmqu_objs = env.Object(Glob('libs/zmqu/*.cpp'))

# static libzmqu library
zmqu_lib = env.StaticLibrary('zmqu', [zmqu_objs])

env.Program('rplay', [
	'rplay.cpp',
	'player.cpp',
	'library.cpp',
	zmqu_lib
])

# client
env.Program('rplayc', ['rplayc.cpp', zmqu_lib])
