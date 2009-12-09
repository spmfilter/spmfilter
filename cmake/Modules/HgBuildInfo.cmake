# HgBuildInfo.cmake
#
# a simple CMake Module which collects all build required build info.
# The generated variables will be used in config.h later for printing
# out build information of an installed program.

# collect build info
execute_process(
	COMMAND hg tip -q 
	OUTPUT_VARIABLE HGREV 
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
	COMMAND hostname
	OUTPUT_VARIABLE BUILD_HOST
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
	COMMAND date
	OUTPUT_VARIABLE BUILD_DATE
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
	COMMAND pwd
	OUTPUT_VARIABLE BUILD_DIR
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
	COMMAND whoami
	OUTPUT_VARIABLE BUILD_USER
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

