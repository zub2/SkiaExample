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
