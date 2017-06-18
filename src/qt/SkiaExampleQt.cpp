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
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QSpinBox>
#include <QSlider>

#include "SkiaPolygonWidget.h"

class SkiaExampleQt: public QWidget
{
	Q_OBJECT

public:
	explicit SkiaExampleQt(QWidget *parent = nullptr):
		QWidget(parent)
	{
		setWindowTitle(QString::fromUtf8(u8"Qt Skia Example"));

		QVBoxLayout * vboxLayout(new QVBoxLayout);

		m_skiaPolygonWidget = new SkiaPolygonWidget(this);
		vboxLayout->addWidget(m_skiaPolygonWidget, 1);
		m_skiaPolygonWidget->show();

		QFormLayout * formLayout(new QFormLayout);

		QSpinBox * vertexCountSpinBox(new QSpinBox);
		vertexCountSpinBox->setRange(3, 20);
		vertexCountSpinBox->setSingleStep(1);
		vertexCountSpinBox->setValue(m_skiaPolygonWidget->vertexCount());

		formLayout->addRow(u8"&Vertices", vertexCountSpinBox);

		QSlider * angleSlider(new QSlider);
		angleSlider->setOrientation(Qt::Horizontal);
		angleSlider->setRange(0, 359);
		formLayout->addRow(u8"&Rotation", angleSlider);

		vboxLayout->addLayout(formLayout);

		setLayout(vboxLayout);

		// there is valueChanget(int) and valueChanged(const QString&), use static_cast to pick the int one
		connect(vertexCountSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
				m_skiaPolygonWidget, &SkiaPolygonWidget::setVertexCount);
		connect(angleSlider, &QSlider::valueChanged, [this](int angleDegrees)
		{
			m_skiaPolygonWidget->setAngle(angleDegrees * M_PI/180);
		});
	}

private:
	SkiaPolygonWidget * m_skiaPolygonWidget;
};

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	SkiaExampleQt w;
	w.show();

	return a.exec();
}

#include "SkiaExampleQt.moc"
