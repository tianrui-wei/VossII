project('vossii', 'c', 'cpp')

if get_option('buildtype') == 'debug'
  add_project_arguments('-DDEBUG', language : ['c', 'cpp'])
endif

x11_dep = dependency('x11')
tk_dep = dependency('tcl')
edit_dep = dependency('libedit')

vossii_dep = [x11_dep, tk_dep, edit_dep]

fl_inc = include_directories('include')

subdir('lib')
subdir('external')

# wrap minisat in cmake project
cmake_opt_var = cmake.subproject_options()

minisat_proj = cmake.subproject('minisat', options: cmake_opt_var)
minisat_dep = minisat_proj.dependency('minisat-lib-shared')
# minisatlib = shared_library('minisat', minisat_dep)
# lib_subproj = subproject('lib')

# wrapped whereami library

subproject('whereami')


subdir('bin/fl')
