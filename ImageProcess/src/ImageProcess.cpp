// ImageProcess.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ImageProcess.h"

#define SIZE_ELEMENT 5
#define MIN_AREA 5
#define MIN_ROTATE_DEGREE 1.5

#define RADIUS_PIXELS 5

ImageProcess::ImageProcess()
	:m_pUnitsPixels(nullptr)
{

}

ImageProcess::~ImageProcess()
{
	destroyAllWindows();
}

void ImageProcess::ProcessingImage(string& strImage, string& strSave, unique_ptr<float[]>& upUnitsPixels)
{
	if (strImage.empty() || strSave.empty()) return;
	m_strSavePath = strSave;
	m_pUnitsPixels = &upUnitsPixels;

	if (!LoadInputImage(strImage)) return;

	destroyAllWindows();

	Mat imPreProc = m_imInput.clone();
	Mat imPostProc;

	if (!PreProcessing(imPreProc, imPostProc)) return;

	vector<Point2f> vecCenter;

	float fAngle = GetRotation(imPostProc, vecCenter);

	///Rotate
	if (abs(fAngle) < 45 && abs(fAngle) > MIN_ROTATE_DEGREE || abs(fAngle) >= 45 && abs(fAngle + 90) > MIN_ROTATE_DEGREE)
	{
		float fAngleToRot = (abs(fAngle) < 45) ? -abs(fAngle) : abs(90 + fAngle);

		Mat imRot;
		RotateImage(fAngleToRot, imRot);

		imshow("RotatedImage", m_imInput);

		imPreProc = m_imInput.clone();

		if (!PreProcessing(imPreProc, imPostProc)) return;

		float fAngle = GetRotation(imPostProc, vecCenter);
	}

	Point2f ptUL, ptBR;
	GetROI(vecCenter, ptUL, ptBR);

	DivideChip(ptUL, ptBR);

}

bool ImageProcess::LoadInputImage(const string &strImage)
{
	if (strImage.empty()) return false;

	m_imInput = imread(strImage, IMREAD_UNCHANGED);
	if (m_imInput.empty()) return false;

	int nChannels = m_imInput.channels();
	int nType = m_imInput.type();
	switch (m_imInput.step[1])
	{
	case 1:
	{
		m_ratioWB = (1.1 / 1.3);
		m_ratioHB = (1.3 / 1.1);
		break;
	}
	case 2:
	{
		m_ratioWB = (153.0 / 166.0);
		m_ratioHB = (142.0 / 175.0);
		break;
	}
	default:
		return false;
		break;
	}

	return true;
}

bool ImageProcess::PreProcessing(const Mat& image, Mat& imOut)
{
	if (image.empty()) return false;

	Mat imSingle(image.rows,image.cols,CV_8U);
	Mat imThresh;

	switch (image.step[1])
	{
	case 1:
	{
		bitwise_not(image, imSingle);
		threshold(imSingle, imThresh, 10, 255, THRESH_BINARY);
		break;
	}
	case 2:
	{
		convertScaleAbs(image, imSingle, 0.00389105058365758754863813229572);

		threshold(imSingle, imThresh, 8, 255, THRESH_BINARY);
		break;
	}
	default:
		return false;
		break;
	}

	Mat element = getStructuringElement(MORPH_ELLIPSE, Size(SIZE_ELEMENT, SIZE_ELEMENT));
	Mat imOpen;
	morphologyEx(imThresh, imOpen, MORPH_OPEN, element);

	imOut = imOpen.clone();

	return true;
}

float ImageProcess::GetRotation(Mat& image, vector<Point2f>& vecCenter)
{
	if (image.empty()) return 99;

	vector<vector<Point> > squares;
	vector<vector<Point> > contours;

	findContours(image, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

	vector<Point> approx;
	for (size_t i = 0; i < contours.size(); i++)
	{
		approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true);

		if (fabs(contourArea(Mat(approx))) > MIN_AREA)
		{
			squares.push_back(approx);
		}
	}

	vecCenter.resize(squares.size());
	for (size_t i = 0; i < squares.size(); i++)
	{
		vector<Moments> mu(squares.size());
		for (size_t i = 0; i < squares.size(); i++)
		{
			mu[i] = moments(squares[i], false);

			vecCenter[i] = Point2f(static_cast<float>(mu[i].m10 / mu[i].m00), static_cast<float>(mu[i].m01 / mu[i].m00));
		}
	}

	RotatedRect box = minAreaRect(Mat(vecCenter));

	Point2f center, vtx[4];
	float fAngle = box.angle;
	box.points(vtx);

	return fAngle;
}

void ImageProcess::RotateImage(float fAngleToRot, Mat& imOutRot)
{
	Point ptCenter = Point(m_imInput.cols / 2, m_imInput.rows / 2);

	Mat maRot = getRotationMatrix2D(ptCenter, fAngleToRot, 1);

	Mat matImRot;
	warpAffine(m_imInput, imOutRot, maRot, m_imInput.size());

	m_imInput = imOutRot.clone();
}

void ImageProcess::GetROI(vector<Point2f>& vecCenter, Point2f& ptOutUL, Point2f& ptOutBR)
{
	if (0 == vecCenter.size()) return;

	Rect rectBoundingBox = boundingRect(Mat(vecCenter));
	Point2f ptUL, ptBR;
	ptUL = Point2f(rectBoundingBox.x, rectBoundingBox.y);
	ptBR = Point2f(rectBoundingBox.x + rectBoundingBox.width, rectBoundingBox.y + rectBoundingBox.height);

	float fWidthUnit, fWidthBlank, fHeightUnit, fHeightBlank;

	fWidthUnit = rectBoundingBox.width / (8 + 7 / m_ratioWB);
	fWidthBlank = fWidthUnit / m_ratioWB;
	fHeightUnit = rectBoundingBox.height / (6 + 5 / m_ratioHB);
	fHeightBlank = fHeightUnit / m_ratioHB;

	ptOutUL = Point2f(rectBoundingBox.x - fWidthBlank / 2,
		rectBoundingBox.y - fHeightBlank / 2);
	ptOutBR = Point2f(rectBoundingBox.x + rectBoundingBox.width + fWidthBlank / 2,
		rectBoundingBox.y + rectBoundingBox.height + fHeightBlank / 2);

}

void ImageProcess::DivideChip(Point2f& ptUL, Point2f& ptBR)
{
	float fStepWidth = (ptBR.x - ptUL.x) / CHIP_COLS;
	float fStepHeight = (ptBR.y - ptUL.y) / CHIP_ROWS;

	Point2f ptUnits[CHIP_ROWS][CHIP_COLS] = { 0 };
	for (int r = 0; r < CHIP_ROWS; r++)
	{
		for (int c = 0; c < CHIP_COLS; c++)
		{
			ptUnits[r][c] = Point2f(ptUL.x + c*fStepWidth, ptUL.y + r*fStepHeight);
		}
	}

	int nWidth = m_imInput.size().width;
	int nHeight = m_imInput.size().height;

	for (int r_chip = 0; r_chip < CHIP_ROWS; r_chip++)
	{
		for (int c_chip = 0; c_chip < CHIP_COLS; c_chip++)
		{
			Point2f ptUL = Point2f(ptUnits[r_chip][c_chip].x > 0 ? ptUnits[r_chip][c_chip].x : 0,
				ptUnits[r_chip][c_chip].y > 0 ? ptUnits[r_chip][c_chip].y : 0);
			Point2f ptBR = Point2f((ptUnits[r_chip][c_chip].x + fStepWidth) > nWidth ? nWidth : (ptUnits[r_chip][c_chip].x + fStepWidth),
				(ptUnits[r_chip][c_chip].y + fStepHeight) > nHeight ? nHeight : (ptUnits[r_chip][c_chip].y + fStepHeight));

			Rect rectROI = Rect(ptUL, ptBR);

			Mat imUnitROI = m_imInput(rectROI);

			stringstream ss;
			char chCol = 'A';
			ss << "ROI-" << char(chCol + c_chip) << (r_chip + 1);
			string strFile = m_strSavePath + ss.str() + ".tif";
			bool bRes = imwrite(strFile, imUnitROI);

			bool bShow = false;
			if (char(chCol + c_chip) == 'G' && r_chip+1 == 6)
			{
				///bShow = true;
			}
			ProcessingUnits(imUnitROI, r_chip, c_chip, bShow);
		}
	}

}

bool ImageProcess::DivideUnits(vector<Point2f>& vecDots, unique_ptr<Point2f[]>& uptrDots, Size& minSize)
{
	if (0 == vecDots.size()) return false;

	Rect rectBoundingBox = boundingRect(Mat(vecDots));

	if (rectBoundingBox.width < minSize.width / 3 || rectBoundingBox.height < minSize.height / 3)
		return false;

	Point2f ptUL, ptBR;
	ptUL = Point2f(rectBoundingBox.x, rectBoundingBox.y);
	ptBR = Point2f(rectBoundingBox.x + rectBoundingBox.width, rectBoundingBox.y + rectBoundingBox.height);

	float fUnitStepWidth = (ptBR.x - ptUL.x) / (UNIT_COLS - 1);
	float fUnitStepHeight = (ptBR.y - ptUL.y) / (UNIT_ROWS - 1);

	for (int r = 0; r < UNIT_ROWS; r++)
	{
		for (int c = 0; c < UNIT_COLS; c++)
		{
			uptrDots[r*UNIT_COLS + c] = Point2f(ptUL.x + c*fUnitStepWidth, ptUL.y + r*fUnitStepHeight);
		}
	}

	return true;
}

float ImageProcess::GetUnitPixels(const Mat& imUnit, const Point2f& ptDot, int nRadius)
{
	if (imUnit.empty()) return 0;

	switch (imUnit.step[1])
	{
	case 1:
	{
		Mat imNot;
		bitwise_not(imUnit, imNot);

		multiset<uchar, greater<uchar>> setPixels;
		for (int y = ptDot.y - nRadius; y <= ptDot.y + nRadius; y++)
		{
			for (int x = ptDot.x - nRadius; x <= ptDot.x + nRadius; x++)
			{
				setPixels.insert(imNot.at<uchar>(x, y));
			}
		}

		float fSum = 0;
		auto it = setPixels.begin();
		for (int i = 0; i < 4; i++)
		{
			fSum += *it;
			it++;
		}
		float fAverage4Pixels = (fSum) / 4.0;

		return fAverage4Pixels*255.0;
		break;
	}
	case 2:
	{
		multiset<unsigned short, greater<unsigned short>> setPixels;
		for (int y = ptDot.y - nRadius; y <= ptDot.y + nRadius; y++)
		{
			for (int x = ptDot.x - nRadius; x <= ptDot.x + nRadius; x++)
			{
				setPixels.insert(imUnit.at<unsigned short>(x,y));
			}
		}

		float fSum = 0;
		auto it = setPixels.begin();
		for (int i = 0; i < 4; i++)
		{
			fSum += *it;
			it++;
		}
		float fAverage4Pixels = (fSum) / 4.0;

		return fAverage4Pixels;
		break;
	}
	default:
		return 0;
		break;
	}
}

bool ImageProcess::ProcessingUnits(const Mat& matUnit, int nRowChip, int nColChip, bool bShow /*= false*/)
{
	Mat imUnitPost;

	if (!PreProcessing(matUnit, imUnitPost)) return false;
	if (bShow)
	{
		imshow("Pre", matUnit);
		imshow("UnitImage", imUnitPost);
	}

	vector<Point2f> vecDots;
	float fAngle = GetRotation(imUnitPost, vecDots);

	unique_ptr<Point2f[]> uptrDots(new Point2f[UNIT_ROWS*UNIT_COLS]);

	if (!DivideUnits(vecDots, uptrDots, Size(imUnitPost.cols, imUnitPost.rows))) return false;

	for (int r_unit = 0; r_unit < UNIT_ROWS; r_unit++)
	{
		for (int c_unit = 0; c_unit < UNIT_COLS; c_unit++)
		{
			float fAveragePixelsValue = GetUnitPixels(matUnit, uptrDots[r_unit*UNIT_COLS + c_unit], RADIUS_PIXELS);

			(*m_pUnitsPixels)[(nRowChip*CHIP_COLS + nColChip)*UNIT_ROWS*UNIT_COLS + (r_unit*UNIT_COLS + c_unit)] = fAveragePixelsValue;
		}
	}

	return true;
}