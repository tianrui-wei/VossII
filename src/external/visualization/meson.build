
visualization = ['list_util.c', 'place_bounds.c', 'place.c', 'sch_util.c', 'schematic_draw_module.c']

visualization_lib = shared_library('visualization', 
  visualization, 
  dependencies: [x11_dep, tk_dep], 
  link_with: vossii_lib,
  include_directories : fl_inc)


