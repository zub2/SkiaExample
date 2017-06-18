/*
 * Copyright 2017 David Kozub <zub at linux.fjfi.cvut.cz>
 *
 * This file is part of SkiaExample.
 *
 * SkiaExample is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SkiaExample is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SkiaExample.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <iostream>
#include <stdexcept>

#include <SkData.h>
#include <SkImage.h>
#include <SkStream.h>
#include <SkSurface.h>
#include <SkCanvas.h>

void raster(int width, int height, void (*draw)(SkCanvas&), const char* path)
{
	sk_sp<SkSurface> rasterSurface = SkSurface::MakeRasterN32Premul(width, height);
	if (!rasterSurface)
		throw std::runtime_error("can't create surface");

	SkCanvas* rasterCanvas = rasterSurface->getCanvas();
	if (!rasterCanvas)
		throw std::runtime_error("can't get canvas");

	draw(*rasterCanvas);

	sk_sp<SkImage> img(rasterSurface->makeImageSnapshot());
	if (!img)
		throw std::runtime_error("can't make image snapshot");

	sk_sp<SkData> png(img->encode());
	if (!png)
		throw std::runtime_error("can't encode png");

	SkFILEWStream out(path);
	out.write(png->data(), png->size());
}

void drawTest(SkCanvas &canvas)
{
	canvas.clear(SK_ColorWHITE);

	SkPaint paint;
	paint.setTextSize(32.0f);
	paint.setAntiAlias(true);
	paint.setColor(SK_ColorBLACK);
	paint.setStyle(SkPaint::kFill_Style);
	canvas.drawString("Skia Test", 20, 32, paint);
}

int main()
{
	const char *testFileName = "test.png";
	const int WIDTH = 100;
	const int HEIGHT = 100;

	std::cout << "Writing test output to " << testFileName << std::endl;
	raster(WIDTH, HEIGHT, drawTest, testFileName);
}
