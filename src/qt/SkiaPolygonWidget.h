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
#ifndef SKIA_POLYGON_WIDGET_H
#define SKIA_POLYGON_WIDGET_H

#include <stdexcept>

#include <QWidget>
#include "SkSurface.h"

// this is inspired by skia/debugger/SkRasterWidget.cpp
class SkiaPolygonWidget: public QWidget
{
	Q_OBJECT
	Q_PROPERTY(float angle READ angle WRITE setAngle NOTIFY angleChanged)
	Q_PROPERTY(unsigned vertexCount READ vertexCount WRITE setVertexCount NOTIFY vertexCountChanged)
	Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
	Q_PROPERTY(QColor strokeColor READ strokeColor WRITE setStrokeColor NOTIFY strokeColorChanged)
	Q_PROPERTY(QColor fillColor READ fillColor WRITE setFillColor NOTIFY fillColorChanged)
	Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)

public:
	SkiaPolygonWidget(QWidget *parent = nullptr);

	virtual QSize sizeHint() const override;
	virtual QSize minimumSizeHint() const override;

	void setAngle(float a)
	{
		m_angle = a;
		update();
		emit angleChanged(a);
	}

	float angle() const
	{
		return m_angle;
	}

	void setVertexCount(unsigned n)
	{
		m_vertexCount = n;
		update();
		emit vertexCountChanged(n);
	}

	unsigned vertexCount() const
	{
		return m_vertexCount;
	}

	void setBackgroundColor(const QColor &c)
	{
		m_backgroundColor = c;
		update();
		emit backgroundColorChanged(c);
	}

	const QColor& backgroundColor() const
	{
		return m_backgroundColor;
	}

	void setStrokeColor(const QColor &c)
	{
		m_strokeColor = c;
		update();
		emit strokeColorChanged(c);
	}

	const QColor& strokeColor() const
	{
		return m_strokeColor;
	}

	void setFillColor(const QColor &c)
	{
		m_fillColor = c;
		update();
		emit fillColorChanged(c);
	}

	const QColor& fillColor() const
	{
		return m_fillColor;
	}

	void setTextColor(const QColor &c)
	{
		m_textColor = c;
		update();
		emit textColorChanged(c);
	}

	const QColor& textColor() const
	{
		return m_textColor;
	}

signals:
	void angleChanged(float);
	void vertexCountChanged(unsigned);
	void backgroundColorChanged(QColor);
	void strokeColorChanged(QColor);
	void fillColorChanged(QColor);
	void textColorChanged(QColor);

protected:
	virtual void resizeEvent(QResizeEvent *event) override;
	virtual void paintEvent(QPaintEvent *event) override;

private:
	static SkPath createPolygonPath(SkScalar size, SkScalar rotationAngle, unsigned count);
	void render(SkCanvas &canvas);

	float m_angle;
	unsigned m_vertexCount;
	QColor m_backgroundColor;
	QColor m_strokeColor;
	QColor m_fillColor;
	QColor m_textColor;
	sk_sp<SkSurface> m_skiaSurface;
};

#endif // SKIA_POLYGON_WIDGET_H
