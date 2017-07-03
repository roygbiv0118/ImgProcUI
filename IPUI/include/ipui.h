#ifndef IPUI_H
#define IPUI_H

#include <QtWidgets/QtWidgets>
#include "ui_ipui.h"
#include <memory>
#include <map>
#include <thread>
using namespace std;

class ImageProcess;
class RgbProcess;
struct ConfigData;
struct ColorRGB;

class QRGBLabel;

class IPUI : public QDialog
{
	Q_OBJECT

public:
	IPUI(QWidget *parent = 0);
	virtual ~IPUI();

	void GetManualPixel(const QPoint &pt);

private:
	void LoadConfig();
	void showImage(const QString& strFile);
	void saveCSV(const QString& strFile, unique_ptr<float[]>& upUnitsPixels);
	void saveExcel();
	void saveExcels(map<QString, ColorRGB*> &mapColorRGB);

	bool checkResults();

	void killAndDel();

private slots:
	void on_pbTestSingle_clicked();
	void on_pbTestRGB_clicked();
	void on_pbTestDir_clicked();
	void on_GrayOrRGB_index(int index);
	void on_cbManualType_currentIndexChanged(int index);
	void on_tabWidget_currentChanged(int index);
	void on_cbValueType_currentIndexChanged(int index);

private:
	Ui::IPUIClass ui;
	ImageProcess* m_pImageProc;
	RgbProcess* m_pProcRGB;

	ColorRGB* m_arrDotsPixels;
	unsigned short m_nRows;
	unsigned short m_nCols;

	ConfigData* m_pConfig;

	QImage* m_pImage;
	QImage* m_pImScaled;
	QString m_strImage;
	QString m_strCurDir;

	QPoint m_StartPos;

	qint64 m_nPID;

	QList<QLabel *> m_listLayout;
	QComboBox* m_pCbGrayOrRGB;

	int m_nGrayOrRGB;
	QRgb m_rgbSelected;

	std::thread* m_pThSave;

};

#endif // IPUI_H
