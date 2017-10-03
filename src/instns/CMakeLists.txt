include_directories(.)

add_library(instns STATIC
    instn.c instn_defs.c instn_impl_misc.c
    instn_impl_arith.c instn_impl_bitw.c instn_impl_rel.c
    instn_impl_logc.c instn_impl_ctrl.c
    instn_impl_sys.c
)
