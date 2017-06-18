cmake_minimum_required(VERSION 3.0)
include(ExternalProject)

#
# Skia
#

set(SKIA_SRC "${CMAKE_CURRENT_SOURCE_DIR}/skia")

if (NOT EXISTS ${SKIA_SRC})
	message(FATAL_ERROR "Can't find Skia sources. Please run download-dependencies.sh.")
endif ()

set(SKIA_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/gb_build")
set(SKIA_INSTALL_DIR "TODO")

# Skia comes with -Werror on by default. That's a cool feature for release...
set(SKIA_C_FLAGS -Wno-error ${CMAKE_C_FLAGS})
set(SKIA_CXX_FLAGS -Wno-error ${CMAKE_CXS_FLAGS})

ExternalProject_Add(SKIA_build
	# This is needed for list expansion of C(XX)_FLAGS.
	LIST_SEPARATOR ","
	SOURCE_DIR ${SKIA_SRC}
	CONFIGURE_COMMAND ${SKIA_SRC}/bin/gn gen --root=${SKIA_SRC} ${SKIA_BUILD_DIR}
		# getting right the wonky cmake string handling (with the extra level of ExternalProject)
		# and the undocumented args handling of GN is a special treat
		# at least Cmake 3.0 goves us the line continuation feature wich is cool
		# but then the args can't be indented because GN is written by somone with overly strong opinions:
		# "You got a tab character in here. Tabs are evil. Convert to spaces."
		"--args=\
ar=\"${CMAKE_AR}\" \
cc=\"${CMAKE_C_COMPILER}\" \
cxx=\"${CMAKE_CXX_COMPILER}\" \
extra_cflags_c=[\"${SKIA_C_FLAGS}\"] \
extra_cflags_cc=[\"${SKIA_CXX_FLAGS}\"] \
is_official_build=true \
is_component_build=false \
skia_enable_pdf=false \
skia_use_egl=false \
skia_enable_spirv_validation=false \
skia_enable_tools=false \
skia_use_expat=false \
skia_use_fontconfig=true \
skia_use_freetype=true \
skia_use_icu=true \
skia_use_libjpeg_turbo=false \
skia_use_libpng=true \
skia_use_libwebp=false \
skia_use_lua=false \
skia_use_mesa=false \
skia_use_piex=false \
skia_use_vulkan=false \
skia_use_zlib=true \
"
	BUILD_COMMAND ninja -C ${SKIA_BUILD_DIR} -j1
	# AFAIK there is not install step provided
	INSTALL_COMMAND true
)

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

find_library(LIBDL dl)
# TODO: this is just temporary
find_library(LIBGL GL)
if (NOT LIBGL)
	message(FATAL_ERROR "Can't find libGL")
endif()
#find_package(Freetype REQUIRED)
pkg_check_modules(FONTCONFIG REQUIRED fontconfig)

add_library(SKIA STATIC IMPORTED GLOBAL)
set_target_properties(SKIA PROPERTIES
	IMPORTED_LOCATION "${SKIA_BUILD_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}skia${CMAKE_STATIC_LIBRARY_SUFFIX}"
	INTERFACE_INCLUDE_DIRECTORIES "${SKIA_HEADERS}"
	IMPORTED_LINK_INTERFACE_LIBRARIES "${LIBDL};${FONTCONFIG_LIBRARIES};${PNG_LIBRARIES};${LIBPTHREAD};${LIBGL}"
)
add_dependencies(SKIA SKIA_build)
