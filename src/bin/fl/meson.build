flex = find_program('flex', required: true)
bison = find_program('bison', required: true)

lgen = generator(flex,
output : '@PLAINNAME@.yy.c',
arguments : ['-o', '@OUTPUT@', '@INPUT@'])

lfiles = lgen.process('lexer')

pgen = generator(bison,
output : ['@BASENAME@.tab.c', '@BASENAME@.tab.h'],
arguments : ['-d', '-v', '@INPUT@', '--defines=@OUTPUT1@', '--output=@OUTPUT0@'])

pfiles = pgen.process('language.y')


fl_source = [
'arb_prec.c',
'cache.c',
'compile.c',
'emit.c',
'error.c',
'expand_cursor.c',
'file_ops.c',
'fl.c',
'graph.c',
'initialize.c',
'io.c',
'lp.c',
'new_bdd.c',
'remote_tcl.c',
'symbol_tbl.c',
'typecheck.c',
'bexpr.c',
'draw_graph.c',
'debug.c',
'symbol.c',
'strings.c',
'int_ops.c',
'list_ops.c',
'system.c',
'float.c',
'bv.c',
'bev.c',
'fsm.c',
'doc.c',
'sha256.c',
'image.c',
'minisat_ifc.cc',
'main.cc',
'plugin.c',
'plugin_loader.c',
'plugin_checker.c',
'plugin_errlog.c',
'table.c',
'iso.c',
'pexlif.c',
'serialize.c',
lfiles,
pfiles
]

loc_inc = include_directories('.')

fl_lib = executable('fl', fl_source,
    dependencies: [vossii_dep, minisat_dep],
    link_with: vossii_lib,
    include_directories: [fl_inc, loc_inc],
    override_options : ['c_std=gnu99'],
    c_args: ['-D__STDC_LIMIT_MACROS', '-D__STDC_FORMAT_MACROS', '-Wno-error', '-DDEBUG', '-DFL_BINARY=1', '-DVERSION_DATE=0000']
    )
