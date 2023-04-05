project('whereami', 'c', 'cpp')

incdir = include_directories('src')

libwhereami = shared_library('whereami', sources: ['src/whereami.h', 'src/whereami.c'], include_directories: incdir, install: true)

whereami_dep = declare_dependency(include_directories: incdir, link_with: libwhereami)
