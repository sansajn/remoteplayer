# dependencies:
#    libgstreamer1.0-dev
env = Environment(
	CCFLAGS=['-std=c++14', '-Wall', '-g', '-O0'],
	LIBS=['boost_filesystem', 'boost_system'])

env.ParseConfig('pkg-config --cflags --libs gstreamer-1.0')

env.Program('rplay', [
	'main.cpp',
	'player.cpp',
	'library.cpp'
])
