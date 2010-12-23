# SCOOM_SEARCH_LIBS(FUNCTION, SEARCH-LIBS,
#                   [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND],
#                   [OTHER-LIBRARIES])
# --------------------------------------------------------
# Search for a library defining FUNC, if it's not already available.
# Do not update the LIBS variable.
AC_DEFUN([SCROOM_SEARCH_LIBS],
[AS_VAR_PUSHDEF([scroom_Libs], [scroom_m4_$1])dnl
AS_VAR_COPY([scroom_Libs], [LIBS])
AC_SEARCH_LIBS([$1], [$2], [$3], [$4], [$5])
AS_VAR_COPY([LIBS], [scroom_Libs])
AS_VAR_POPDEF([scroom_Libs])dnl
])
