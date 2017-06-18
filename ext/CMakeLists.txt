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

# I wanted to expose (almost) all Skia options as CMake options but sadly
# GN is a really bad tool - It produces non-overridable configure errors like:
#
# The variable "skia_use_system_libjpeg_turbo" was set as a build argument but never appeared in a declare_args() block in any buildfile.
#
# ... even though the option exists (this one in skia/third_party/libjpeg-turbo/BUILD.gb)
# and it's listed in gn args --list.
# (It seems to be caused by the fact that some options are only actually used when other
# options are set appropriately. But it's not always obvious - one would have to dig into
# Skia configuration. And replicating the dependencies here + propagating it correctly into
# the ExternalProject setup is beyond reasonable.
option(EXT_SKIA_ALWAYS_BUILD "" OFF)
option(EXT_SKIA_DEBUG "" ${IS_DEBUG_BUILD})
option(EXT_SKIA_SHARED "build a shared lbrary (ON) or a static library (OFF)" OFF)

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

SET_SKIA_CONFIG_OPTION(SK_CONF_DEBUG ${EXT_SKIA_DEBUG})
SET_SKIA_CONFIG_OPTION(SK_CONF_SHARED ${EXT_SKIA_SHARED})

# it's not possible to enable both OFFICIAL and DEBUG
if (EXT_SKIA_OFFICIAL_BUILD AND NOT EXT_SKIA_DEBUG)
	set(SK_CONF_IS_OFFICIAL_BUILD "true")
else ()
	set(SK_CONF_IS_OFFICIAL_BUILD "false")
endif ()

ExternalProject_Add(SKIA_build
	# This is needed for list expansion of C(XX)_FLAGS.
	LIST_SEPARATOR ","
	SOURCE_DIR ${SKIA_SRC}
	CONFIGURE_COMMAND ${SKIA_SRC}/bin/gn gen --root=${SKIA_SRC} ${SKIA_BUILD_DIR}
		# getting right the wonky cmake string handling (with the extra level of ExternalProject)
		# and the undocumented args handling of GN is a special treat
		# at least Cmake 3.0 has the line continuation feature wich is cool
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
skia_enable_gpu=true \
skia_use_egl=false \
skia_use_vulkan=false \
skia_enable_vulkan_debug_layers=false \
skia_enable_spirv_validation=false \
skia_use_icu=true \
skia_use_system_icu=true \
skia_use_expat=true \
skia_use_system_expat=true \
skia_use_libjpeg_turbo=false \
skia_use_libpng=true \
skia_use_system_libpng=true \
skia_use_zlib=true \
skia_use_system_zlib=true \
skia_use_libwebp=false \
skia_enable_pdf=true \
skia_use_sfntly=true \
skia_use_fontconfig=true \
skia_use_freetype=true \
skia_use_system_freetype2=true \
skia_enable_tools=false \
skia_use_lua=false \
skia_use_mesa=false \
skia_use_piex=false \
skia_use_dng_sdk=false \
"
	BUILD_COMMAND ninja -C ${SKIA_BUILD_DIR} -j1
	# there is no install step provided
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
	"SK_SAMPLES_FOR_X" # always set for linux, even if there's no X used
	"SK_SUPPORT_GPU=1" # skia_enable_gpu
	#"SK_VULKAN" # skia_use_vulkan
	#"SK_ENABLE_VK_LAYERS" # skia_enable_vulkan_debug_layers
	#"SK_ENABLE_SPIRV_VALIDATION" # skia_enable_spirv_validation
	#"SK_HAS_JPEG_LIBRARY" # skia_use_libjpeg_turbo
	"SK_HAS_PNG_LIBRARY" # skia_use_libpng
	"SK_SUPPORT_PDF" # skia_enable_pdf
	"SK_PDF_USE_SFNTLY" # skia_use_sfntly
	#"SK_HAS_WEBP_LIBRARY" # skia_use_libwebp
	"SK_XML" # skia_use_expat
)

if (SK_CONF_SHARED)
	liast(APPEND SKIA_DEFINES "SKIA_DLL")
endif ()

# GN doesn't seem to do any fancy stuff with library dependencies - it just
# asks the compiler to link to them w/o any fany search strategies. This
# also means that whatever is found here is not guaranteed to be the same as
# what GN finds. :-(
# Anyway, using plain find_library() seems to be the next closest thing,
# so even if some libraries provide pkg_config or special packages for CMake,
# only find_library() is used here.
# This is needed only for static library build where the dependencies have
# to be added explicitly.
if (NOT EXT_SKIA_SHARED)
	function(ADD_SKIA_LIBRARY_DEPENDENCY LIB_NAME_LIST)
		foreach(LIB_NAME ${LIB_NAME_LIST})
			message(STATUS "Searching for ${LIB_NAME}...")
			find_library(LIB${LIB_NAME} ${LIB_NAME})
			if (NOT LIB${LIB_NAME})
				message(FATAL_ERROR "Can't find required library ${LIB_NAME}.")
			endif ()
			set(SKIA_DEPENDENCIES "${SKIA_DEPENDENCIES};${LIB${LIB_NAME}}" PARENT_SCOPE)
		endforeach()
	endfunction()

	# seem to be always required...
	ADD_SKIA_LIBRARY_DEPENDENCY("dl")
	ADD_SKIA_LIBRARY_DEPENDENCY("pthread")

	# when skia_enable_gpu:
	#ADD_SKIA_LIBRARY_DEPENDENCY("EGL") # skia_use_egl
	ADD_SKIA_LIBRARY_DEPENDENCY("GL") # !skia_use_egl # TODO: GLU?

	ADD_SKIA_LIBRARY_DEPENDENCY("icuuc") # skia_use_system_icu
	ADD_SKIA_LIBRARY_DEPENDENCY("expat") #skia_use_system_expat
	#ADD_SKIA_LIBRARY_DEPENDENCY("jpeg") # skia_use_system_libjpeg_turbo
	ADD_SKIA_LIBRARY_DEPENDENCY("png") # skia_use_system_libpng
	ADD_SKIA_LIBRARY_DEPENDENCY(${EXT_SKIA_USE_SYSTEM_ZLIB} "z") # skia_use_system_zlib

	# webp integration doesn't expose the system option...
	#ADD_SKIA_LIBRARY_DEPENDENCY("webp") # SK_CONF_IS_OFFICIAL_BUILD && skia_use_libwebp

	ADD_SKIA_LIBRARY_DEPENDENCY(${EXT_SKIA_USE_FONTCONFIG} "fontconfig") # skia_use_fontconfig
	ADD_SKIA_LIBRARY_DEPENDENCY(${EXT_SKIA_USE_FREETYPE2} "freetype") # skia_use_system_freetype2
endif (NOT EXT_SKIA_SHARED)

if (EXT_SKIA_SHARED)
	add_library(SKIA SHARED IMPORTED GLOBAL)
	set(SKIA_LIBRARY "${SKIA_BUILD_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}skia${CMAKE_SHARED_LIBRARY_SUFFIX}")
else (EXT_SKIA_SHARED)
	add_library(SKIA STATIC IMPORTED GLOBAL)
	set(SKIA_LIBRARY "${SKIA_BUILD_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}skia${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif (EXT_SKIA_SHARED)

set_target_properties(SKIA PROPERTIES
	IMPORTED_LOCATION "${SKIA_LIBRARY}"
	INTERFACE_INCLUDE_DIRECTORIES "${SKIA_HEADERS}"
	INTERFACE_COMPILE_DEFINITIONS "${SKIA_DEFINES}"
	IMPORTED_LINK_INTERFACE_LIBRARIES "${SKIA_DEPENDENCIES}"
)
add_dependencies(SKIA SKIA_build)
