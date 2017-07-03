#include "rgblabel.h"
#include "ipui.h"

#define CIRCLE_DIAMETER 50

QRGBLabel::QRGBLabel(QWidget *parent)
	: QLabel(parent)
	, m_pIPUI(qobject_cast<IPUI*>(parent))
	, m_ptSelected(QPoint(-100,-100))
	, m_bManual(false)
{
	m_pixCircle.load("../Res/circle.png");
	m_pixScaled = m_pixCircle.scaled(QSize(CIRCLE_DIAMETER, CIRCLE_DIAMETER));
}

QRGBLabel::~QRGBLabel()
{}

void QRGBLabel::SetManual(bool bManual)
{
	m_bManual = bManual;
}

void QRGBLabel::Reset()
{
	m_ptSelected = QPoint(-100,-100);
}

void QRGBLabel::mousePressEvent(QMouseEvent * evt)
{
	if (!m_bManual) return;

	m_ptSelected = evt->pos();
	m_pIPUI->GetManualPixel(m_ptSelected);
}

void QRGBLabel::paintEvent(QPaintEvent *evt)
{
	QLabel::paintEvent(evt);

	if (m_bManual)
	{
		QPainter painter(this);

		painter.drawPixmap(m_ptSelected - QPoint(CIRCLE_DIAMETER / 2, CIRCLE_DIAMETER / 2), m_pixScaled);
	}	

	update();

}
