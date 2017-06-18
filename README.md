# Skia Example Project

This is an example CMake-based project that integrates Skia. A headless and a Qt Widgets example are provided. The headless example `skia_example_headless` just creates a `test.png` in the current working directory using Skia. The Qt example `skia_example_qt` creates a window with a Skia-based widget drawing a polygon.

There is probably quite some space to optimize the Skia config and the Qt wrapping. But I hope this can serve as a starting point.

After checking out this repository run `ext/download-dependencies.sh` to get Skia sources. Only then run CMake to configure the project.
