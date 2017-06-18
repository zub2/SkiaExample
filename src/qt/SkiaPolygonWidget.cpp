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
#include "SkiaPolygonWidget.h"

#include <stdexcept>
#include <algorithm>
#include <cmath>

#include <QResizeEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QImage>

#include <SkImage.h>
#include <SkSurface.h>
#include <SkCanvas.h>
#include <SkPixmap.h>
#include <SkPath.h>

/**
 * Skia and Qt use different style of naming pixel formats.
 *
 * Qt's Format_ARGB32 means a pixel loaded into a 32bit variable has the structure
 * 0xAARRGGBB. When stored in memory this becomes either BB, GG, RR, AA (little endian)
 * or AA, RR, GG, BB (big endian).
 * Then there is Format_RGBA8888 which is defined to be stored as RR, GG, BB, AA. In a
 * 32bit variable this becomes either 0xAABBGGRR (little endian) or 0xRRGGBBAA (big endian).
 *
 * Skia seems to label formats based on how the components are stored in memory:
 * kRGBA_8888_SkColorType means a pixel is stored in memory like this:
 * RR, GG, BB, AA. When loaded into a register this becomes either 0xAABBGGRR (little endian)
 * or 0xRRGGBBAA (big endian).
 *
 * This means that:
 * * for little endian:
 *   kBGRA_8888_SkColorType (Skia) is the same as Format_ARGB32 (Qt)
 *   kRGBA_8888_SkColorType (Skia) is the same as Format_RGBA8888 (Qt)
 *
 * * for big endian:
 *   kBGRA_8888_SkColorType (Skia) would be Format_BGRA32 or Format_BGRA888 in Qt (but there's no such thing)
 *   kRGBA_8888_SkColorType (Skia) is Format_RGBA888 (Qt)
 *
 * And, of course, Skia probably would not work on big endian even though there is some
 * half-baked support for big endian.
 */

#ifdef SK_CPU_BENDIAN
	// It seems that big endian is not even properly supported by Skia.
	// The mapping of Skia to Qt pixel formats would need to be extended here.
	#error Big endian not supported.
#endif

namespace
{
	/**
	 * Map a Skia color type a QImage premultiplied format.
	 */
	template<SkColorType skColorType>
	struct SkColorTypeToQtFormatPreMultiplied;

	template<>
	struct SkColorTypeToQtFormatPreMultiplied<kBGRA_8888_SkColorType>
	{
		static constexpr QImage::Format value = QImage::Format_ARGB32_Premultiplied;
	};

	template<>
	struct SkColorTypeToQtFormatPreMultiplied<kRGBA_8888_SkColorType>
	{
		static constexpr QImage::Format value = QImage::Format_RGBA8888_Premultiplied;
	};

	SkColor skColorFromQColor(const QColor & qColor)
	{
		return SkColorSetARGB(qColor.alpha(), qColor.red(), qColor.green(), qColor.blue());
	}

	constexpr QSize SIZE_HINT(400, 400);
	constexpr QSize MINIMUM_SIZE_HINT(200, 200);
	constexpr float DEFAULT_ANGLE = 0;
	constexpr unsigned DEFAULT_VERTEX_COUNT = 5;
	const QColor DEFAULT_BACKGROUND_COLOR(255, 255, 255, 255);
	const QColor DEFAULT_STROKE_COLOR(0, 0, 0, 255);
	const QColor DEFAULT_FILL_COLOR(0, 255, 0, 255);
	const QColor DEFAULT_TEXT_COLOR(0, 0, 0, 128);
}

SkiaPolygonWidget::SkiaPolygonWidget(QWidget *parent):
	QWidget(parent),
	m_angle(DEFAULT_ANGLE),
	m_vertexCount(DEFAULT_VERTEX_COUNT),
	m_backgroundColor(DEFAULT_BACKGROUND_COLOR),
	m_strokeColor(DEFAULT_STROKE_COLOR),
	m_textColor(DEFAULT_TEXT_COLOR),
	m_fillColor(DEFAULT_FILL_COLOR)
{
	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

QSize SkiaPolygonWidget::sizeHint() const
{
	return SIZE_HINT;
}

QSize SkiaPolygonWidget::minimumSizeHint() const
{
	return MINIMUM_SIZE_HINT;
}

void SkiaPolygonWidget::resizeEvent(QResizeEvent *event)
{
	int size = std::min(width(), height());

	if (size > 0)
	{
		/*
		 * It would be ideal to use kBGRA_8888_SkColorType pixel format for the Skia
		 * surface and QImage::Format_ARGB32_Premultiplied for the QImage format.
		 * But Skia's 32bit backend always suppors exactly one of kBGRA_8888_SkColorType
		 * and kRGBA_8888_SkColorType. There is kN32_SkColorType which then equals
		 * to the selected one. So asking for kBGRA_8888_SkColorType here would
		 * mean on some platforms (Windows?) the creation of the raster image would fail.
		 *
		 * For Linux kN32_SkColorType is forced (via SK_SAMPLES_FOR_X in SkUserConfig.h)
		 * to kBGRA_8888_SkColorType. On Windows it seems to be equal to kRGBA_8888_SkColorType.
		 *
		 * If needed this could be changed in Skia configuration.
		 */
		m_skiaSurface = SkSurface::MakeRasterN32Premul(size, size);
		if (!m_skiaSurface)
			throw std::runtime_error("can't create Skia surface");
	}
	else
		m_skiaSurface.reset();

	QWidget::resizeEvent(event);
}

void SkiaPolygonWidget::paintEvent(QPaintEvent *)
{
	QPainter painter(this);

	bool succeeded = false;
	if (m_skiaSurface)
	{
		SkCanvas* skiaCanvas = m_skiaSurface->getCanvas();
		if (!skiaCanvas)
			throw std::runtime_error("cant' get Skia canvas");

		render(*skiaCanvas);

		SkPixmap skiaPixmap;
		if (m_skiaSurface->peekPixels(&skiaPixmap))
		{
			QImage qtImage(reinterpret_cast<const uchar*>(skiaPixmap.addr()),
						skiaPixmap.width(),
						skiaPixmap.height(),
						skiaPixmap.rowBytes(),
						SkColorTypeToQtFormatPreMultiplied<kN32_SkColorType>::value);

			// draw image centered, avoid skewing it
			const int imageWidth = skiaCanvas->getBaseLayerSize().width();
			const int imageHeight = skiaCanvas->getBaseLayerSize().height();
			const QRect content = contentsRect();

			const int midX = content.x() + content.width()/2;
			const int midY = content.y() + content.height()/2;
			const int x = midX - imageWidth/2;
			const int y = midY - imageHeight/2;

			QRect drawRect(x, y, imageWidth, imageHeight);
			painter.drawImage(drawRect, qtImage);

			succeeded = true;
		}
	}

	if (!succeeded)
		painter.fillRect(contentsRect(), QColor(255, 0, 0));
}

SkPath SkiaPolygonWidget::createPolygonPath(SkScalar size, SkScalar rotationAngle, unsigned count)
{
	// draw the polygon as if it's in [-1,1]²
	SkPath path;
	path.moveTo(1, 0);
	const SkScalar angleStep = 2 * M_PI / count;
	for (int i = 0; i < count; i++)
	{
		const SkScalar a = angleStep * i;
		path.lineTo(std::cos(a), std::sin(a));
	}

	// close the path
	path.lineTo(path.getPoint(0));

	// rotate, shift (0,0) to the center and scale
	SkMatrix t;
	t.setRotate(rotationAngle * 180/M_PI); // radians to degrees
	t.postTranslate(1, 1);
	t.postScale(size/2, size/2);
	path.transform(t);
	return path;
}

void SkiaPolygonWidget::render(SkCanvas &canvas)
{
	canvas.clear(skColorFromQColor(m_backgroundColor));

	SkPath path(createPolygonPath(canvas.getBaseLayerSize().width(), m_angle, m_vertexCount));

	SkPaint paint;
	paint.setAntiAlias(true);

	// fill
	paint.setStyle(SkPaint::kFill_Style);
	paint.setColor(skColorFromQColor(m_fillColor));
	canvas.drawPath(path, paint);

	// stroke
	paint.setStyle(SkPaint::kStroke_Style);
	paint.setStrokeWidth(2.0f);
	paint.setColor(skColorFromQColor(m_strokeColor));
	canvas.drawPath(path, paint);

	paint.setTextSize(32.0f);
	paint.setColor(skColorFromQColor(m_textColor));
	paint.setStyle(SkPaint::kFill_Style);
	canvas.drawString(std::to_string(m_vertexCount).c_str(), 0, canvas.getBaseLayerSize().height(), paint);
}

#include "SkiaPolygonWidget.moc"
