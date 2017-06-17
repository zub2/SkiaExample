# Skia example with Qt GUI
# inspired by http://doc.qt.io/qt-5/cmake-manual.html

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the QtWidgets library
find_package(Qt5Widgets REQUIRED)

add_executable(skia_example_qt
	SkiaPolygonWidget.h
	SkiaPolygonWidget.cpp
	SkiaExampleQt.cpp
)
target_link_libraries(skia_example_qt SKIA Qt5::Widgets)
