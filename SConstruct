env = Environment(
	CCFLAGS=['-std=c++14', '-Wall', '-g', '-O0'])

env.ParseConfig('pkg-config --cflags --libs libvlc')


env.Program('rplay', ['main.cpp'])
