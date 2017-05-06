cmake_minimum_required(VERSION 3.0)
include(ExternalProject)

#
# Skia
#

# case-insensitive match TODO: is debug, Debug, DEBUG all valid?
if (CMAKE_BUILD_TYPE MATCHES "[dD][eE][bB][uU][gG]")
	set(IS_DEBUG_BUILD ON)
else ()
	set(IS_DEBUG_BUILD OFF)
endif ()

option(EXT_SKIA_ALWAYS_BUILD "" OFF)
option(EXT_SKIA_DEBUG "" ${IS_DEBUG_BUILD})
option(EXT_SKIA_SHARED "build a shared lbrary (ON) or a static library (OFF)" OFF)
option(EXT_SKIA_OFFICIAL_BUILD "Ignored when EXT_SKIA_DEBUG is ON." ON)
option(EXT_SKIA_ENABLE_GPU "" ON)
option(EXT_SKIA_USE_EGL "When EXT_SKIA_ENABGLE_GPU is ON: ON: use EGL, OFF: use GL+GLU" OFF)
option(EXT_SKIA_USE_ICU "" ON)
option(EXT_SKIA_USE_LIBPNG "" ON)
option(EXT_SKIA_USE_EXPAT "" ON)
option(EXT_SKIA_USE_SYSTEM_EXPAT "When EXT_SKIA_USE_EXPAT is ON: use libexpat from the system" ON)
option(EXT_SKIA_USE_LIBJPEG_TURBO "" OFF)
option(EXT_SKIA_USE_LIBWEBP "" OFF)
option(EXT_SKIA_USE_ZLIB "" ON)
option(EXT_SKIA_ENABLE_PDF "" OFF)
option(EXT_SKIA_USE_FONTCONFIG "" ON)
option(EXT_SKIA_USE_FREETYPE "" ON)

set(SKIA_SRC "${CMAKE_CURRENT_SOURCE_DIR}/skia")

if (NOT EXISTS ${SKIA_SRC})
	message(FATAL_ERROR "Can't find Skia sources. Please run download-dependencies.sh.")
endif ()

set(SKIA_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/skia")

# Skia comes with -Werror on by default. That's a cool feature for release...
set(SKIA_C_FLAGS -Wno-error ${CMAKE_C_FLAGS})
set(SKIA_CXX_FLAGS -Wno-error ${CMAKE_CXS_FLAGS})

function(SET_SKIA_CONFIG_OPTION OPT_NAME OPT_VALUE)
	if (OPT_VALUE)
		set(${OPT_NAME} "true" PARENT_SCOPE)
	else ()
		set(${OPT_NAME} "false" PARENT_SCOPE)
	endif ()
endfunction()

# it's not possible to enable both OFFICIAL and DEBUG
if (EXT_SKIA_OFFICIAL_BUILD AND NOT EXT_SKIA_DEBUG)
	set(SK_CONF_IS_OFFICIAL_BUILD "true")
else ()
	set(SK_CONF_IS_OFFICIAL_BUILD "false")
endif ()

SET_SKIA_CONFIG_OPTION(SK_CONF_DEBUG ${EXT_SKIA_DEBUG})
SET_SKIA_CONFIG_OPTION(SK_CONF_SHARED ${EXT_SKIA_SHARED})
SET_SKIA_CONFIG_OPTION(SK_CONF_ENABLE_GPU ${EXT_SKIA_ENABLE_GPU})
SET_SKIA_CONFIG_OPTION(SK_CONF_USE_EGL ${EXT_SKIA_USE_EGL})
SET_SKIA_CONFIG_OPTION(SK_CONF_USE_ICU ${EXT_SKIA_USE_ICU})
SET_SKIA_CONFIG_OPTION(SK_CONF_USE_LIBPNG ${EXT_SKIA_USE_LIBPNG})
SET_SKIA_CONFIG_OPTION(SK_CONF_USE_EXPAT ${EXT_SKIA_USE_EXPAT})
SET_SKIA_CONFIG_OPTION(SK_CONF_USE_SYSTEM_EXPAT ${EXT_SKIA_USE_SYSTEM_EXPAT})
SET_SKIA_CONFIG_OPTION(SK_CONF_USE_LIBJPEG_TURBO ${EXT_SKIA_USE_LIBJPEG_TURBO})
SET_SKIA_CONFIG_OPTION(SK_CONF_USE_LIBWEBP ${EXT_SKIA_USE_LIBWEBP})
SET_SKIA_CONFIG_OPTION(SK_CONF_USE_ZLIB ${EXT_SKIA_USE_ZLIB})
SET_SKIA_CONFIG_OPTION(SK_CONF_ENABLE_PDF ${EXT_SKIA_ENABLE_PDF})
SET_SKIA_CONFIG_OPTION(SK_CONF_USE_FONTCONFIG ${EXT_SKIA_USE_FONTCONFIG})
SET_SKIA_CONFIG_OPTION(SK_CONF_USE_FREETYPE ${EXT_SKIA_USE_FREETYPE})

ExternalProject_Add(SKIA_build
	# This is needed for list expansion of C(XX)_FLAGS.
	LIST_SEPARATOR ","
	SOURCE_DIR ${SKIA_SRC}
	CONFIGURE_COMMAND ${SKIA_SRC}/bin/gn gen --root=${SKIA_SRC} ${SKIA_BUILD_DIR}
		# getting right the wonky cmake string handling (with the extra level of ExternalProject)
		# and the undocumented args handling of GN is a special treat
		# at least Cmake 3.0 gives us the line continuation feature wich is cool
		# but then the args can't be indented because GN is written by somone with overly strong opinions:
		# "You got a tab character in here. Tabs are evil. Convert to spaces."
		"--args=\
ar=\"${CMAKE_AR}\" \
cc=\"${CMAKE_C_COMPILER}\" \
cxx=\"${CMAKE_CXX_COMPILER}\" \
extra_cflags_c=[\"${SKIA_C_FLAGS}\"] \
extra_cflags_cc=[\"${SKIA_CXX_FLAGS}\"] \
is_official_build=${SK_CONF_IS_OFFICIAL_BUILD} \
is_component_build=${SK_CONF_SHARED} \
is_debug=${SK_CONF_DEBUG} \
skia_enable_gpu=${SK_CONF_ENABLE_GPU} \
skia_enable_pdf=${SK_CONF_ENABLE_PDF} \
skia_use_egl=${SK_CONF_USE_EGL} \
skia_use_expat=${SK_CONF_USE_EXPAT} \
skia_use_system_expat=${SK_CONF_USE_SYSTEM_EXPAT} \
skia_use_fontconfig=${SK_CONF_USE_FONTCONFIG} \
skia_use_freetype=${SK_CONF_USE_FREETYPE} \
skia_use_icu=${SK_CONF_USE_ICU} \
skia_use_libjpeg_turbo=${SK_CONF_USE_LIBJPEG_TURBO} \
skia_use_libpng=${SK_CONF_USE_LIBPNG} \
skia_use_zlib=${SK_CONF_USE_ZLIB} \
skia_use_libwebp=${SK_CONF_USE_LIBWEBP} \
skia_enable_tools=false \
skia_use_lua=false \
skia_use_mesa=false \
skia_use_piex=false \
skia_use_vulkan=false \
skia_enable_spirv_validation=false \
skia_use_dng_sdk=false \
skia_use_sfntly=false \
"
	BUILD_COMMAND ninja -C ${SKIA_BUILD_DIR} -j1
	# AFAIK there is not install step provided
	INSTALL_COMMAND true
)

if (EXT_SKIA_ALWAYS_BUILD)
	# Make sure the target is always rebuilt.
	# Without this changing Skia sources doesn't trigger a ninja build. With this
	# ninja build is always triggered. This is not needed if you never touch the
	# Skia sources. Please note that enabling this ends up with re-building of
	# targets that depend on the library. :(
	ExternalProject_Add_Step(SKIA_build ForceBuild COMMAND true DEPENDERS build ALWAYS 1)
endif ()

# taken from BUILD.gn (skia_public_includes, minus things that are obviously useless for us)
set(SKIA_HEADERS
	${SKIA_SRC}/include/c
	${SKIA_SRC}/include/codec
	${SKIA_SRC}/include/config
	${SKIA_SRC}/include/core
	${SKIA_SRC}/include/effects
	${SKIA_SRC}/include/gpu
	${SKIA_SRC}/include/gpu/gl
	${SKIA_SRC}/include/pathops
	${SKIA_SRC}/include/ports
	${SKIA_SRC}/include/svg
	${SKIA_SRC}/include/utils
)

# this must match what BUILD.gn sets
# if it's wrong it can result in all kinds of funny behavior
set(SKIA_DEFINES
	SK_SAMPLES_FOR_X # always set for linux, even if there's no X used
)

function(ADD_SKIA_DEFINES IS_ENABLED IF_ENABLED_DEFINE_LIST)
	if (IS_ENABLED)
		set(SKIA_DEFINES "${SKIA_DEFINES};${IF_ENABLED_DEFINE_LIST}" PARENT_SCOPE)
	else (IS_ENABLED)
		# optionaly define for the not-enabled case can be specified
		if (ARGN EQUAL 1)
			set(SKIA_DEFINES "${SKIA_DEFINES};${ARGV1}" PARENT_SCOPE)
		endif ()
		if (ARGN GREATER 1)
			message(FATAL_ERROR "ADD_SKIA_DEFINES: wrong number of arguments!")
		endif ()
	endif (IS_ENABLED)
endfunction()

ADD_SKIA_DEFINES(${EXT_SKIA_ENABLE_GPU} "SK_SUPPORT_GPU=1" "SK_SUPPORT_GPU=0")
ADD_SKIA_DEFINES(${EXT_SKIA_USE_LIBPNG} "SK_HAS_PNG_LIBRARY")
ADD_SKIA_DEFINES(${EXT_SKIA_USE_LIBJPEG_TURBO} "SK_HAS_JPEG_LIBRARY")
ADD_SKIA_DEFINES(${EXT_SKIA_ENABLE_PDF} "SK_SUPPORT_PDF")
ADD_SKIA_DEFINES(${EXT_SKIA_USE_LIBWEBP} "SK_HAS_WEBP_LIBRARY")
ADD_SKIA_DEFINES(${EXT_SKIA_USE_EXPAT} "SK_XML")

# GN doesn't seem to do any fancy stuff with library dependencies - it just
# asks the compiler to link to them w/o any fany search strategies. This
# also means that whatever we find here is not guaranteed to be the same as
# what GN finds. :-(
# Anyway, using plain find_library() seems to be the next closest thing,
# so even if some libraries provide pkg_config or special packages for CMake,
# only find_library() is used here.
# This is needed only for static library build where we need to the dependencies
# explicitly.
if (NOT EXT_SKIA_SHARED)
	function(ADD_SKIA_LIBRARY_DEPENDENCY IS_ENABLED LIB_NAME_LIST)
		if (IS_ENABLED)
			foreach(LIB_NAME ${LIB_NAME_LIST})
				message(STATUS "Searching for ${LIB_NAME}...")
				find_library(LIB ${LIB_NAME})
				if (NOT LIB)
					message(FATAL_ERROR "Can't find required library ${LIB_NAME}.")
				endif ()
				set(SKIA_DEPENDENCIES "${SKIA_DEPENDENCIES};${LIB}" PARENT_SCOPE)
			endforeach()
		endif (IS_ENABLED)
	endfunction()

	# seem to be always required...
	ADD_SKIA_LIBRARY_DEPENDENCY(ON "dl")
	ADD_SKIA_LIBRARY_DEPENDENCY(ON "pthread")

	ADD_SKIA_LIBRARY_DEPENDENCY(${EXT_SKIA_USE_SYSTEM_EXPAT} "expat")
	ADD_SKIA_LIBRARY_DEPENDENCY(${EXT_SKIA_USE_SYSTEM_EXPAT} "png")

	if (EXT_SKIA_ENABLE_GPU)
		if (EXT_SKIA_USE_EGL)
			ADD_SKIA_LIBRARY_DEPENDENCY(ON "EGL")
		else ()
			ADD_SKIA_LIBRARY_DEPENDENCY(ON "GL;GLU")
		endif ()
	endif (EXT_SKIA_ENABLE_GPU)

	ADD_SKIA_LIBRARY_DEPENDENCY(${EXT_SKIA_USE_FONTCONFIG} "fontconfig")
	ADD_SKIA_LIBRARY_DEPENDENCY(${EXT_SKIA_USE_FREETYPE} "freetype")
endif (NOT EXT_SKIA_SHARED)

add_library(SKIA STATIC IMPORTED GLOBAL)
set_target_properties(SKIA PROPERTIES
	IMPORTED_LOCATION "${SKIA_BUILD_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}skia${CMAKE_STATIC_LIBRARY_SUFFIX}"
	INTERFACE_INCLUDE_DIRECTORIES "${SKIA_HEADERS}"
	INTERFACE_COMPILE_DEFINITIONS "${SKIA_DEFINES}"
	IMPORTED_LINK_INTERFACE_LIBRARIES "${SKIA_DEPENDENCIES}"
)
add_dependencies(SKIA SKIA_build)
