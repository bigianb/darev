project('darevtest', 'cpp', default_options : ['cpp_std=c++17', 'buildtype=debug'])
incdir = include_directories('./host_mocks')
src = ['dlist_test.cpp', 'frameFunctions_test.cpp', 'gsalloc_test.cpp', 'runtests.cpp', 'runtests.h',
'../src/ps2/dlist.cpp', '../src/ps2/dlist.h', 
'../src/ps2/gsAllocator.cpp', '../src/ps2/state.cpp', '../src/ps2/trace.cpp']

incdirlist = [incdir, '/usr/local/ps2dev/ps2sdk/ee/include']
exe = executable('darevtest.elf', src, include_directories :incdirlist )
test('simple test', exe)
