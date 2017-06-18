#include <iostream>

#include <SkData.h>
#include <SkImage.h>
#include <SkStream.h>
#include <SkSurface.h>
#include <SkCanvas.h>

void raster(int width, int height, void (*draw)(SkCanvas&), const char* path)
{
	sk_sp<SkSurface> rasterSurface = SkSurface::MakeRasterN32Premul(width, height);
	if (!rasterSurface)
		return;

	SkCanvas* rasterCanvas = rasterSurface->getCanvas();
	if (!rasterCanvas)
		return;

	draw(*rasterCanvas);

	sk_sp<SkImage> img(rasterSurface->makeImageSnapshot());
	if (!img)
		return;

	sk_sp<SkData> png(img->encode());
	if (!png)
		return;

	SkFILEWStream out(path);
	out.write(png->data(), png->size());
}

void drawTest(SkCanvas &canvas)
{
	SkPaint paint;
	paint.setColor(SK_ColorRED);
	canvas.drawPaint(paint);
}

int main()
{
	raster(100, 100, drawTest, "test.png");
}
