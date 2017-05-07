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
	raster(100, 100, drawTest, "test.png");
}
