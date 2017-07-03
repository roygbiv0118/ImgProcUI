// ImageProcess.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "RgbProcess.h"
#include <iostream>
#include <string>
#include <strstream>
using namespace std;

#define MIN_ROTATE_DEGREE 0.5
#define RADIUS_PIXELS 5

#define SIZE_ELEMENT 5

#define WIN_NAME "R_thresh"

void on_Trackbar(int nValue, void* pArg)
{
	RgbProcess* pThis = (RgbProcess*)pArg;

	threshold(pThis->m_imTrans, pThis->m_imThresh, nValue, 255, THRESH_BINARY);

	imshow(WIN_NAME, pThis->m_imThresh);
}

RgbProcess::RgbProcess()
	:m_arrDotsPixels(NULL)
	, m_nRows(0)
	, m_nCols(0)
	, m_bShowHistgram(false)
	, m_bShowThreshold(false)
	, m_bShowTriDots(false)
	, m_bShowGrid(false)
	, m_bShowRGB(false)
	, m_eValueType(VT_AVERAGE)
	, m_rng(0xffffffff)
{
}

RgbProcess::~RgbProcess()
{
	destroyAllWindows();
}

void RgbProcess::ProcessingImage(string& strImage, ColorRGB* arrDotsPixels, ConfigData* pConfig)
{
	if (!pConfig) return;
	if (!arrDotsPixels) return;

	m_arrDotsPixels = arrDotsPixels;
	m_nRows = pConfig->nRows;
	m_nCols = pConfig->nCols;
	m_bShowHistgram = pConfig->bShowHistgram;
	m_bShowThreshold = pConfig->bShowThreshold;
	m_bShowTriDots = pConfig->bShowTriDots;
	m_bShowGrid = pConfig->bShowGrid;
	m_bShowRGB = pConfig->bShowRGB;
	m_eValueType = pConfig->eValueType;

	if (!LoadInputImage(strImage)) return;

	destroyAllWindows();

	Mat imPostProc;
	if (!PreProcessing(m_imInput, imPostProc, m_bShowRGB)) return;

	vector<Point2f> vecCenter;
	float fAngle = 0;
	if (!GetRotation(imPostProc, vecCenter, fAngle, 500, m_bShowTriDots)) return;

	///Rotate
	if (abs(fAngle) > MIN_ROTATE_DEGREE)
	{
		float fAngleToRot = fAngle;

		RotateImage(m_imInput, fAngleToRot);

		PreProcessing(m_imInput, imPostProc);

		GetRotation(imPostProc, vecCenter, fAngle, 500);
	}

	Point2f ptUL, ptBR;
	GetROI(vecCenter, ptUL, ptBR);

	ProcessingUnit(ptUL, ptBR);

}

bool RgbProcess::LoadInputImage(const string &strImage)
{
	if (strImage.empty()) return false;

	m_imInput = imread(strImage, IMREAD_UNCHANGED);
	if (m_imInput.empty()) return false;

	int nChannels = m_imInput.channels();
	int nType = m_imInput.type();

	bool bRes = false;
	switch (m_imInput.step[1])
	{
	case 3:
	case 4:
		bRes = true;
		break;
	default:
		bRes = false;
		break;
	}

	return bRes;
}

bool RgbProcess::PreProcessing(const Mat& image, Mat& imOut, bool bShowRGB)
{
	if (image.empty()) return false;

	vector<Mat> vecMat;
	split(image, vecMat);
	if (bShowRGB)
	{
		imshow("Red", vecMat[2]);
		imshow("Green", vecMat[1]);
		imshow("Blue", vecMat[0]);
	}

	Mat matColorB, matColorG, matColorR, matColorAdd;
	bitwise_not(vecMat[0], matColorB);
	bitwise_not(vecMat[1], matColorG);
	bitwise_not(vecMat[2], matColorR);

	//addWeighted(matColorR, 1, matColorB, -1,0,matColorAdd);
	m_imTrans = matColorG.clone();
	int nThresh = FindThreshold(m_imTrans, m_bShowHistgram);


	if (m_bShowThreshold)
	{
		Mat imThresh;
		namedWindow(WIN_NAME);

		int nValue = nThresh;
		createTrackbar("Threshold", WIN_NAME, &nValue, 255, on_Trackbar, (void*)this);
		on_Trackbar(nValue, (void*)this);
	}
	else
	{
		threshold(m_imTrans, m_imThresh, nThresh, 255, THRESH_BINARY);
	}

	Mat element = getStructuringElement(MORPH_ELLIPSE, Size(SIZE_ELEMENT, SIZE_ELEMENT));

	morphologyEx(m_imThresh, imOut, MORPH_OPEN, element);

	return true;
}

bool RgbProcess::GetRotation(const Mat& image, vector<Point2f>& vecCenter, float& angle, double dMinArea /*= 500*/, bool bShowTriDots /*= false*/)
{
	if (image.empty()) return false;

	vector<vector<Point> > squares;
	vector<vector<Point> > contours;

	findContours(image, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

	vector<Point> approx;
	for (size_t i = 0; i < contours.size(); i++)
	{
		approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true);

		if (fabs(contourArea(Mat(approx))) > dMinArea)
		{
			squares.push_back(approx);
		}
	}

	if (squares.size() < 3) return false;

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

	Point2f ptXmin = vecCenter[0], ptXmax = vecCenter[0], ptYmax = vecCenter[0];
	for (Point2f pt : vecCenter)
	{
		if (pt.y > ptYmax.y) ptYmax = pt;
		if ((pt.x + pt.y) > (ptXmax.x + ptXmax.y)) ptXmax = pt;
		if (pt.x < ptXmin.x) ptXmin = pt;
	}

	Point2f dirLeft = ptXmin - ptYmax;
	float cosLeft = 0;
	if (dirLeft != Point2f(0, 0))
	{
		cosLeft = dirLeft.dot(Point2f(-1, 0)) / sqrt(dirLeft.x*dirLeft.x + dirLeft.y*dirLeft.y);
	}
	Point2f dirRight = ptXmax - ptYmax;
	float cosRight = 0;
	if (dirRight != Point2f(0, 0))
	{
		cosRight = dirRight.dot(Point2f(1, 0)) / sqrt(dirRight.x*dirRight.x + dirRight.y*dirRight.y);
	}

	float fClockwise = -acos(cosRight) / CV_PI * 180;
	float fCounterClockwise = acos(cosLeft) / CV_PI * 180;

	angle = cosRight > cosLeft ? fClockwise : fCounterClockwise;

	if (bShowTriDots)
	{
		Mat imShow = m_imInput.clone();

		circle(imShow, ptXmin, 2, Scalar(255, 0, 0), 5);
		circle(imShow, ptYmax, 4, Scalar(0, 255, 0), 5);
		circle(imShow, ptXmax, 8, Scalar(0, 0, 255), 5);

		unsigned int nRand = m_rng.next();
		stringstream ss;
		ss << nRand;

		imshow(String("TriDots:") + ss.str(), imShow);
	}

	return true;
}

void RgbProcess::RotateImage(Mat& image, float fAngleToRot)
{
	Point ptCenter = Point(image.cols / 2, image.rows / 2);

	Mat maRot = getRotationMatrix2D(ptCenter, fAngleToRot, 1);

	Mat matImRot;
	warpAffine(image, matImRot, maRot, image.size(), 1, 0, Scalar(255, 255, 255));

	image = matImRot.clone();
}

void RgbProcess::GetROI(vector<Point2f>& vecCenter, Point2f& ptOutUL, Point2f& ptOutBR)
{
	if (vecCenter.size() < 3) return;

	Rect rectBoundingBox = boundingRect(Mat(vecCenter));

	ptOutUL = Point2f(rectBoundingBox.x, rectBoundingBox.y);
	ptOutBR = Point2f(rectBoundingBox.x + rectBoundingBox.width, rectBoundingBox.y + rectBoundingBox.height);

}

ColorRGB RgbProcess::GetDotPixels(const Mat& imUnit, const Point2f& ptDot, int nRadius, ValueType eType)
{
	if (imUnit.empty()) return ColorRGB();
	nRadius *= 0.4;

	if (imUnit.step[1] != 3) return ColorRGB();

	multiset<ColorRGB, less<ColorRGB>> setPixels;

	switch (eType)
	{
	case VT_AVERAGE:
		nRadius = 5;
		break;
	case VT_MAXMIN:
		nRadius *= 0.4;
		break;
	default:
		break;
	}

	for (int y = ptDot.y - nRadius; y <= ptDot.y + nRadius; y++)
	{
		for (int x = ptDot.x - nRadius; x <= ptDot.x + nRadius; x++)
		{
			Vec3b v3Color = imUnit.at<Vec3b>(Point2f(x, y));
			ColorRGB rgb(v3Color[2], v3Color[1], v3Color[0]);

			setPixels.insert(rgb);
		}
	}

	unsigned int sumR = 0;
	unsigned int sumG = 0;
	unsigned int sumB = 0;
	int nSize = 0;

	switch (eType)
	{
	case VT_AVERAGE:
	{
		for (ColorRGB rgb : setPixels)
		{
			sumR += rgb.R;
			sumG += rgb.G;
			sumB += rgb.B;
		}
		nSize = setPixels.size();
	}
		break;
	case VT_MAXMIN:
	{
		const int SIZE_PIXS = 4;
		int cnt = 0;
		for (ColorRGB rgb : setPixels)
		{
			if (++cnt > SIZE_PIXS) break;
			sumR += rgb.R;
			sumG += rgb.G;
			sumB += rgb.B;
		}
		nSize = SIZE_PIXS;
	}
		break;
	default:
		break;
	}

	return ColorRGB(sumR / nSize, sumG / nSize, sumB / nSize);

}

bool RgbProcess::ProcessingUnit(const Point2f& ptUL, const Point2f& ptBR)
{
	float fStepRow = (ptBR.y - ptUL.y) / (m_nRows - 1);
	float fStepCol = (ptBR.x - ptUL.x) / (m_nCols - 1);

	Mat imShow = m_imInput.clone();

	for (int r = 0; r < m_nRows; r++)
	{
		for (int c = 0; c < m_nCols; c++)
		{
			Point2f ptDot = Point2f(ptUL.x + c*fStepCol, ptUL.y + r*fStepRow);

			ColorRGB v3Color = GetDotPixels(m_imInput, ptDot, (ptBR.x-ptUL.x)/(m_nCols-1), m_eValueType);

			m_arrDotsPixels[r*m_nCols + c] = v3Color;

			if (m_bShowGrid)///
			{
				circle(imShow, ptDot, RADIUS_PIXELS, Scalar(0, 255, 255), 2);
			}///

		}
	}

	if (m_bShowGrid)///
	{
		for (int r = 0; r < m_nRows; r++)
		{
			Point2f ptLeft = ptUL + Point2f(0, r*fStepRow);
			Point2f ptRight = Point2f(ptBR.x, ptLeft.y);
			line(imShow, ptLeft, ptRight, Scalar(255, 0, 0));
		}
		for (int c = 0; c < m_nCols; c++)
		{
			Point2f ptUp = ptUL + Point2f(c*fStepCol, 0);
			Point2f ptDown = Point2f(ptUp.x, ptBR.y);
			line(imShow, ptUp, ptDown, Scalar(255, 0, 0));
		}

		imshow("Grid", imShow);
	}///

	return true;
}

int RgbProcess::FindThreshold(const Mat &image, bool bShow)
{
	int bins = 256;
	int hist_size[] = { bins };
	float range[] = { 0, 256 };
	const float* ranges[] = { range };
	MatND imHist;
	int channels[] = { 0 };

	calcHist(&image, 1, channels, Mat(), imHist, 1, hist_size, ranges, true, false);

	double minValue, maxValue;
	Point ptMin, ptMax;
	minMaxLoc(imHist, 0, &maxValue, 0, &ptMax);

	int nThresh = ptMax.y;
	float fPreValue = maxValue;
	for (; nThresh < bins; nThresh++)
	{
		float fBinValue = imHist.at<float>(nThresh);

		if (fBinValue<500 && fBinValue>fPreValue)
		{
			nThresh--;
			break;
		}
		fPreValue = fBinValue;
	}

	if (bShow)
	{
		int scale = 2;
		int histHeight = 256 * scale;
		Mat histImage = Mat::zeros(histHeight, bins*scale, CV_8UC3);

		for (int i = 0; i < bins; i++)
		{
			float binValue = imHist.at<float>(i);

			cout << i << "\t\t" << binValue << endl;

			int intensity = cvRound(binValue*histHeight / maxValue);  //要绘制的高度

			Scalar slrColor;
			switch (i / 10 % 3)
			{
			case 0:
				slrColor = Scalar(255, 0, 0);
				break;
			case 1:
				slrColor = Scalar(0, 255, 0);
				break;
			case 2:
				slrColor = Scalar(0, 0, 255);
				break;
			default:
				slrColor = Scalar(255, 255, 255);
				break;
			}

			rectangle(histImage, Point(i*scale, histHeight - 1),
				Point((i + 1)*scale - 1, histHeight - intensity),
				slrColor);
		}

		imshow("Histgram", histImage);
	}

	return nThresh;
}