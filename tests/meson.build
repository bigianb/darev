project('darevtest', 'cpp')
incdir = include_directories('./host_mocks')
src = ['dlist_test.cpp', 'frameFunctions_test.cpp', 'runtests.cpp', 'runtests.h', '../src/ps2/dlist.cpp', '../src/ps2/dlist.h', '../src/ps2/state.cpp', '../src/ps2/trace.cpp']
exe = executable('darevtest', src, include_directories : incdir)
test('simple test', exe)
