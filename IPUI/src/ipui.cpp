#include "ipui.h"
#include <string>
using namespace std;

#define HEIGHT_SHOW 400
#define DELETE_SAVE(p) if (p){delete p; p = nullptr;}

#include "ImageProcess.h"
#include "RgbProcess.h"
#include "rgblabel.h"
#include <ActiveQt/QAxObject>

#define SAFE_DELETE(p) do{ if(p) {delete (p); (p)=NULL; }} while(false) 

IPUI::IPUI(QWidget *parent)
	: QDialog(parent)
	, m_pImage(NULL)
	, m_pImScaled(NULL)
	, m_nPID(0)
	, m_pConfig(NULL)
	, m_arrDotsPixels(NULL)
	, m_nGrayOrRGB(0)
	, m_rgbSelected(0)
	, m_pCbGrayOrRGB(NULL)
	, m_pThSave(NULL)
{
	ui.setupUi(this);

	setWindowFlags(Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);

	m_strCurDir.clear();

	m_pImageProc = new ImageProcess();
	m_pProcRGB = new RgbProcess();

	ui.pbTestSingle->hide();

	ui.lbShow->setFixedHeight(HEIGHT_SHOW);
	
	LoadConfig();
	m_nRows = m_pConfig->nRows;
	m_nCols = m_pConfig->nCols;

	ui.progressBar->reset();

}

IPUI::~IPUI()
{
#if !B_SWITCH_SAVE
	if (!m_strCurDir.isEmpty())
	{
		killAndDel();
	}
#endif

	DELETE_SAVE(m_pImageProc);
	DELETE_SAVE(m_pProcRGB);

	DELETE_SAVE(m_arrDotsPixels);

	DELETE_SAVE(m_pImage);
	DELETE_SAVE(m_pImScaled);

	DELETE_SAVE(m_pConfig);

	DELETE_SAVE(m_pThSave);

}

void IPUI::LoadConfig()
{
	if (m_pConfig)
	{
		delete m_pConfig;
		m_pConfig = NULL;
	}

	m_pConfig = new ConfigData;

	QSettings setting("../Config/config.ini", QSettings::IniFormat);
	m_pConfig->nRows = setting.value("Grid/Rows").toInt();
	m_pConfig->nCols = setting.value("Grid/Columns").toInt();

	m_pConfig->bShowHistgram = setting.value("Visible/ShowHistgram").toBool();
	m_pConfig->bShowThreshold = setting.value("Visible/ShowThreshold").toBool();
	m_pConfig->bShowTriDots = setting.value("Visible/ShowTriDots").toBool();
	m_pConfig->bShowGrid = setting.value("Visible/ShowGrid").toBool();
	m_pConfig->bShowRGB = setting.value("Visible/ShowRGB").toBool();

}

void IPUI::showImage(const QString& strFile)
{
	m_strImage = QString(strFile);

	if (m_pImage)
	{
		delete m_pImage;
		m_pImage = NULL;
	}

	m_pImage = new QImage(m_strImage);
	float fRatio = m_pImage->width()*1.0 / m_pImage->height();

	if (m_pImScaled)
	{
		delete m_pImScaled;
		m_pImScaled = NULL;
	}
	m_pImScaled = new QImage(QSize(fRatio*ui.lbShow->height(), ui.lbShow->height()), m_pImage->format());
	*m_pImScaled = m_pImage->scaled(QSize(fRatio*ui.lbShow->height(), ui.lbShow->height()));

	ui.lbShow->setPixmap(QPixmap::fromImage(*m_pImScaled));
}

void IPUI::on_tabWidget_currentChanged(int index)
{
	switch (index)
	{
	case 0:
		ui.lbShow->SetManual(false);
		break;
	case 1:
		ui.lbShow->SetManual(true);
		break;
	default:
		ui.lbShow->SetManual(false);
		break;
	}
}

void IPUI::on_pbTestSingle_clicked()
{
	QString strFile = QFileDialog::getOpenFileName(this, "Open", "../Res/", "Images(*.tif *.jpg *.bmp *.png)");
	if (strFile.isEmpty()) return;

	showImage(strFile);

	QString strFileName;
	int nBeg = strFile.lastIndexOf('/');
	if (-1 != nBeg)
	{
		strFileName = strFile.mid(nBeg + 1);
	}

	QString strSave = QString("../Res/Save_%1_%2_%3/").arg(QDate::currentDate().toString("yyyyMMdd")).arg(QTime::currentTime().toString("hhmmss")).arg(strFileName);

	bool bRes = QDir::current().mkdir(strSave);

	unique_ptr<float[]> upUnitsPixels(new float[CHIP_ROWS*CHIP_COLS*UNIT_ROWS*UNIT_COLS]{0});

	m_pImageProc->ProcessingImage(m_strImage.toLocal8Bit().toStdString(), strSave.toStdString(), upUnitsPixels);

	QString strFileCSV = strSave + "save.csv";

#if B_SWITCH_SAVE
	saveCSV(strFileCSV, upUnitsPixels);

#else	
	if (!m_strCurDir.isEmpty())
	{
		killAndDel();
	}
	saveCSV(strFileCSV, upUnitsPixels);

	QProcess* pProcess = new QProcess(this);
	QStringList args;
	args << strFileCSV;
	//pProcess->start("C:/Program Files (x86)/Microsoft Office 2007/Office12/EXCEL.EXE", args);
	pProcess->start("D:\\Program Files\\Microsoft Office\\Office15\\excel.exe", args);
	m_nPID = pProcess->processId();

	m_strCurDir = strSave;
#endif

}

void IPUI::on_pbTestRGB_clicked()
{
	QString strFile = QFileDialog::getOpenFileName(this, "Open", "../Res/", "Images(*.tif *.jpg *.bmp *.png)");
	if (strFile.isEmpty()) return;

	showImage(strFile);

	if (m_arrDotsPixels)
	{
		delete[] m_arrDotsPixels;
		m_arrDotsPixels = NULL;
	}
	m_arrDotsPixels = new ColorRGB[m_nRows*m_nCols];

	m_pProcRGB->ProcessingImage(m_strImage.toLocal8Bit().toStdString(), m_arrDotsPixels, m_pConfig);

	bool bRes = checkResults();
	if (bRes)
	{
		ui.teShowError->setHtml(QString("<font color=\"black\" size=\"5\">%1</font>").arg(tr("No Error")));
	}
	else
	{
		ui.teShowError->setHtml(QString("<font color=\"red\" size=\"5\">%1</font>").arg(tr("Error")));
	}

	m_pCbGrayOrRGB = new QComboBox(this);
	connect(m_pCbGrayOrRGB, SIGNAL(currentIndexChanged(int)), this, SLOT(on_GrayOrRGB_index(int)));
	m_pCbGrayOrRGB->addItem(tr("Gray"));
	m_pCbGrayOrRGB->addItem(tr("RGB"));
	ui.gridLayout->addWidget(m_pCbGrayOrRGB, 0, m_nCols);

	ui.lbShow->Reset();
	m_rgbSelected = 0;
	ui.lbManualPixel->setText("");

	///Save Excel
	DELETE_SAVE(m_pThSave);
	m_pThSave = new std::thread(&IPUI::saveExcel, this);
	m_pThSave->detach();

}

void IPUI::saveCSV(const QString& strFile, unique_ptr<float[]>& upUnitsPixels)
{
	QFile file(strFile);
	bool bRes = file.open(QIODevice::Truncate | QIODevice::WriteOnly);
	QTextStream stream(&file);

	char chCols = 'A';
	for (int chip_row = 0; chip_row < CHIP_ROWS; chip_row++)
	{
		for (int chip_col = 0; chip_col < CHIP_COLS; chip_col++)
		{

			stream << char(chCols + chip_col) << (chip_row + 1) << endl;
			for (int unit_row = 0; unit_row < UNIT_ROWS; unit_row++)
			{
				stream << ' ' << ',';
				for (int unit_col = 0; unit_col < UNIT_COLS; unit_col++)
				{
					stream << qRound(upUnitsPixels[(chip_row*CHIP_COLS + chip_col)*UNIT_ROWS*UNIT_COLS + (unit_row*UNIT_COLS + unit_col)]) << ',';
				}
				stream << endl;
			}
			stream << endl;

		}
	}
	file.close();

}

void IPUI::killAndDel()
{
	QString strCmd;
	int nRes = 0;
	strCmd = QString("@taskkill  /f /t /pid %1 1>nul 2>nul").arg(m_nPID);
	nRes = system(strCmd.toLocal8Bit());

	strCmd = QString("@rd /s/q ") + m_strCurDir.replace('/', "\\") + " 1>nul 2>nul";
	nRes = system(strCmd.toLocal8Bit());

}

void IPUI::on_GrayOrRGB_index(int index)
{
	switch (index)
	{
	case 0:
		for (QLabel* pLabel : m_listLayout)
		{
			pLabel->setParent(NULL);
			ui.gridLayout->removeWidget(pLabel);
			SAFE_DELETE(pLabel);
		}
		m_listLayout.clear();

		for (int r = 0; r < m_pConfig->nRows; r++)
		{
			for (int c = 0; c < m_pConfig->nCols; c++)
			{
				ColorRGB v3 = m_arrDotsPixels[r*m_pConfig->nCols + c];

				QLabel* pLabel = new QLabel(this);
				pLabel->setFrameShape(QFrame::StyledPanel);
				pLabel->setFixedHeight(20);
				pLabel->setText(QString("%1").arg(v3.R * 76 + v3.G * 150 + v3.B * 29));
				pLabel->setAttribute(Qt::WA_DeleteOnClose);
				ui.gridLayout->addWidget(pLabel, r, c);

				m_listLayout << pLabel;
			}
		}
		break;
	case 1:
		for (QLabel* pLabel : m_listLayout)
		{
			pLabel->setParent(NULL);
			ui.gridLayout->removeWidget(pLabel);
			SAFE_DELETE(pLabel);
		}
		m_listLayout.clear();

		for (int r = 0; r < m_pConfig->nRows; r++)
		{
			for (int c = 0; c < m_pConfig->nCols; c++)
			{
				ColorRGB v3 = m_arrDotsPixels[r*m_pConfig->nCols + c];

				QLabel* pLabel = new QLabel(this);
				pLabel->setFrameShape(QFrame::StyledPanel);
				pLabel->setFixedHeight(20);
				pLabel->setText(QString("R:%1 G:%2 B:%3").arg(v3.R).arg(v3.G).arg(v3.B));
				pLabel->setAttribute(Qt::WA_DeleteOnClose);
				ui.gridLayout->addWidget(pLabel, r, c);

				m_listLayout << pLabel;
			}
		}
		break;
	default:
		break;
	}
}

void IPUI::on_cbManualType_currentIndexChanged(int index)
{
	m_nGrayOrRGB = index;

	int nRed = qRed(m_rgbSelected);
	int nGreen = qGreen(m_rgbSelected);
	int nBlue = qBlue(m_rgbSelected);
	int nGray = nRed * 76 + nGreen * 150 + nBlue * 29;

	switch (m_nGrayOrRGB)
	{
	case 0:
		ui.lbManualPixel->setText(QString("%1").arg(nGray));
		break;
	case 1:
		ui.lbManualPixel->setText(QString("R:%1 G:%2 B:%3").arg(nRed).arg(nGreen).arg(nBlue));
		break;
	default:
		ui.lbManualPixel->setText(QString("%1").arg(nGray));
		break;
	}

}

void IPUI::GetManualPixel(const QPoint &pt)
{
	if (!m_pImScaled) return;

	m_rgbSelected = m_pImScaled->pixel(pt);
	int nRed = qRed(m_rgbSelected);
	int nGreen = qGreen(m_rgbSelected);
	int nBlue = qBlue(m_rgbSelected);
	int nGray = nRed * 76 + nGreen * 150 + nBlue * 29;

	switch (m_nGrayOrRGB)
	{
	case 0:
		ui.lbManualPixel->setText(QString("%1").arg(nGray));
		break;
	case 1:
		ui.lbManualPixel->setText(QString("R:%1 G:%2 B:%3").arg(nRed).arg(nGreen).arg(nBlue));
		break;
	default:
		ui.lbManualPixel->setText(QString("%1").arg(nGray));
		break;
	}
}

void IPUI::saveExcel()
{
	QAxObject excel("Excel.Application");
	excel.setProperty("Visible", false);
	excel.setProperty("DisplayAlerts", false);
	QAxObject *work_books = excel.querySubObject("WorkBooks");
	if (!work_books) return;

	QString strPath = QDir::currentPath();
	QString strFileName = strPath+"/../Res/"+"test00.xlsx";
	strFileName.replace("/","\\");
	QFile file(strFileName);

	if (file.exists())
	{
		///system("TASKKILL /F /IM EXCEL.exe /T");
		bool bRes = file.remove();
	}

	if (!file.exists())
	{
		work_books->dynamicCall("Add");
		QAxObject *work_book = excel.querySubObject("ActiveWorkBook");

		if (work_book)
		{
			QAxObject *work_sheets = work_book->querySubObject("Sheets");

			int sheet_count = work_sheets->property("Count").toInt();
			if (sheet_count > 0)
			{
				QAxObject *work_sheet = work_book->querySubObject("Sheets(int)", 1);

				for (int i = 1; i <= m_nRows; i++)
				{
					for (int j = 1; j <= m_nCols; j++)
					{
						QAxObject *cell = work_sheet->querySubObject("Cells(int,int)", i, j);
						ColorRGB v3 = m_arrDotsPixels[(i-1)*m_nCols+(j-1)];
						
						cell->setProperty("Value", QString("%1").arg(v3.R * 76 + v3.G * 150 + v3.B * 29));

						cell->setProperty("ColumnWidth", 30);
						cell->setProperty("RowHeight", 50);
						cell->setProperty("HorizontalAlignment", -4108); //xlLeft:-4131 xlCenter:-4108 xlRight:-4152
					}
				}
			}

		}

		work_book->dynamicCall("SaveAs(const QString&)", strFileName);
		work_book->dynamicCall("Close(Boolean)", false);
	}

	excel.dynamicCall("Quit(void)");
}

void IPUI::saveExcels(map<QString, ColorRGB*> &mapColorRGB)
{
	ui.teShowError->setText(tr("Saving excel..."));
	ui.progressBar->reset();
	ui.progressBar->setRange(0,mapColorRGB.size());

	QAxObject excel("Excel.Application");
	excel.setProperty("Visible", false);
	excel.setProperty("DisplayAlerts", false);
	QAxObject *work_books = excel.querySubObject("WorkBooks");
	if (!work_books) return;

	QString strPath = QDir::currentPath();
	QString strFileName = strPath + "/../Res/" + "results.xlsx";
	strFileName.replace("/", "\\");
	QFile file(strFileName);

	if (file.exists())
	{
		///system("TASKKILL /F /IM EXCEL.exe /T");
		bool bRes = file.remove();
	}

	if (!file.exists())
	{
		work_books->dynamicCall("Add");
		QAxObject *work_book = excel.querySubObject("ActiveWorkBook");

		if (work_book)
		{
			QAxObject *work_sheets = work_book->querySubObject("Sheets");

			int sheet_count = work_sheets->property("Count").toInt();
			if (sheet_count > 0)
			{
				QAxObject *work_sheet = work_book->querySubObject("Sheets(int)", 1);

				//////////////////////////////////////////////////////////////////////////
				int rowExcel = 0, nUnit = 0;

				int nCnt = 0;
				auto it = mapColorRGB.begin();
				for (; it != mapColorRGB.end(); it++,nUnit++)
				{
					QString strName = it->first;
					ColorRGB* colorRGB = it->second;

					rowExcel =1 + nUnit*(m_nRows + 2);
					QAxObject *cell = work_sheet->querySubObject("Cells(int,int)", rowExcel, 1);
					cell->setProperty("Value", '\''+strName);
					/*cell->setProperty("ColumnWidth", 30);
					cell->setProperty("RowHeight", 50);*/
					cell->setProperty("HorizontalAlignment", -4108); //xlLeft:-4131 xlCenter:-4108 xlRight:-4152

					for (int i = 1; i <= m_nRows; i++)
					{
						for (int j = 1; j <= m_nCols; j++)
						{
							QAxObject *cell = work_sheet->querySubObject("Cells(int,int)", rowExcel+i, j);
							ColorRGB v3 = colorRGB[(i - 1)*m_nCols + (j - 1)];

							cell->setProperty("Value", QString("%1").arg(v3.R * 76 + v3.G * 150 + v3.B * 29));

							//cell->setProperty("ColumnWidth", 20);
							//cell->setProperty("RowHeight", 20);
							//cell->setProperty("HorizontalAlignment", -4108); //xlLeft:-4131 xlCenter:-4108 xlRight:-4152
						}
					}

					delete[] colorRGB;

					ui.progressBar->setValue(++nCnt);
				}
				mapColorRGB.clear();
				//////////////////////////////////////////////////////////////////////////

			}

		}

		work_book->dynamicCall("SaveAs(const QString&)", strFileName);
		work_book->dynamicCall("Close(Boolean)", false);

		ui.teShowError->setText(tr("Finish!"));

	}
	else
	{
		ui.teShowError->setText(tr("Error: Please close Excel!"));
	}

	excel.dynamicCall("Quit(void)");

}

void IPUI::on_cbValueType_currentIndexChanged(int index)
{
	m_pConfig->eValueType = static_cast<ValueType>(index);

	for (QLabel* pLabel : m_listLayout)
	{
		pLabel->setParent(NULL);
		ui.gridLayout->removeWidget(pLabel);
		SAFE_DELETE(pLabel);
	}
	m_listLayout.clear();

	if (m_pCbGrayOrRGB)
	{
		m_pCbGrayOrRGB->setParent(NULL);
		ui.gridLayout->removeWidget(m_pCbGrayOrRGB);
		SAFE_DELETE(m_pCbGrayOrRGB);
	}

	ui.lbShow->clear();

}

bool IPUI::checkResults()
{
#define IsApprox(a,b) abs(a - b)<6000
	ColorRGB rgbTopLeft = m_arrDotsPixels[0];
	ColorRGB rgbTopRight = m_arrDotsPixels[m_nCols - 1];
	ColorRGB rgbBotLeft = m_arrDotsPixels[(m_nRows - 1)*m_nCols];
	ColorRGB rgbBotRight = m_arrDotsPixels[(m_nRows - 1)*m_nCols + (m_nCols - 1)];

	int grayTopLeft = rgbTopLeft.R * 76 + rgbTopLeft.G * 150 + rgbTopLeft.B * 29;
	int grayTopRight = rgbTopRight.R * 76 + rgbTopRight.G * 150 + rgbTopRight.B * 29;
	int grayBotLeft = rgbBotLeft.R * 76 + rgbBotLeft.G * 150 + rgbBotLeft.B * 29;
	int grayBotRight = rgbBotRight.R * 76 + rgbBotRight.G * 150 + rgbBotRight.B * 29;

	bool b1 = IsApprox(grayTopLeft, grayBotLeft);
	bool b2 = IsApprox(grayTopLeft, grayBotRight);
	bool b3 = !(IsApprox(grayTopLeft, grayTopRight));
	return (IsApprox(grayTopLeft, grayBotLeft) && IsApprox(grayTopLeft, grayBotRight) && !(IsApprox(grayTopLeft, grayTopRight)));

}

void IPUI::on_pbTestDir_clicked()
{
	QString strDir = QFileDialog::getExistingDirectory(this, "Open directory", "../Res/");
	if (strDir.isEmpty()) return;

	QDir dir(strDir);
	if (!dir.exists()) return;
	QStringList filters;
	filters << "*.jpg";
	QStringList listFileNames = dir.entryList(filters, QDir::Files);


	ui.progressBar->reset();
	ui.progressBar->setRange(0, listFileNames.size());
	ui.teShowError->setText(tr("Processing images..."));

	int nCnt = 0;
	map<QString, ColorRGB*> mapColorRGB;
	for (QString &strName:listFileNames)
	{
		ColorRGB* arrDotsPixels = new ColorRGB[m_nRows*m_nCols];

		QString strFilePath = strDir + '/' + strName;

		ConfigData config;
		config.nRows = m_nRows;
		config.nCols = m_nCols;
		config.eValueType = m_pConfig->eValueType;
		m_pProcRGB->ProcessingImage(strFilePath.toLocal8Bit().toStdString(), arrDotsPixels, &config);

		QFileInfo info(strName);
		QString strBaseName = info.baseName();
		mapColorRGB.insert(make_pair(strBaseName, arrDotsPixels));

		ui.progressBar->setValue(++nCnt);
	}

	saveExcels(mapColorRGB);
}