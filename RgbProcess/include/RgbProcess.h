#pragma once

#ifdef DLL_EXPORTS
#define DLL_API _declspec(dllexport)
#else
#define DLL_API _declspec(dllimport)
#endif

#include <opencv2/opencv.hpp>
using namespace cv;

#include <sstream>
#include <functional>
#include <vector>
#include <string>
#include <set>
#include <memory>
using namespace std;

enum ValueType
{
	VT_AVERAGE,
	VT_MAXMIN
};

struct ConfigData
{
	unsigned short nRows;
	unsigned short nCols;
	bool bShowHistgram;
	bool bShowThreshold;
	bool bShowTriDots;
	bool bShowGrid;
	bool bShowRGB;
	ValueType eValueType;

	ConfigData()
		:nRows(0)
		, nCols(0)
		, bShowHistgram(false)
		, bShowThreshold(false)
		, bShowTriDots(false)
		, bShowGrid(false)
		, bShowRGB(false)
		, eValueType(VT_AVERAGE)
	{
	}
};

struct ColorRGB
{
	unsigned char R;
	unsigned char G;
	unsigned char B;

	ColorRGB()
	{
		R = 0;
		G = 0;
		B = 0;
	}
	ColorRGB(unsigned char r, unsigned char g, unsigned char b)
	{
		R = r;
		G = g;
		B = b;
	}

	bool operator>(ColorRGB rgb) const
	{
		bool bRes = false;
		if (R>rgb.R)
		{
			bRes = true;
		}
		else if (R == rgb.R)
		{
			if (G>rgb.G)
			{
				bRes = true;
			}
			else if (G == rgb.G)
			{
				if (B > rgb.B)
				{
					bRes = true;
				}
				else
				{
					bRes = false;
				}
			}
			else
			{
				bRes = false;
			}
		}
		else
		{
			bRes = false;
		}
		return bRes;
	}

	bool operator<(ColorRGB rgb) const
	{
		bool bRes = false;
		if (R < rgb.R)
		{
			bRes = true;
		}
		else if (R == rgb.R)
		{
			if (G < rgb.G)
			{
				bRes = true;
			}
			else if (G == rgb.G)
			{
				if (B < rgb.B)
				{
					bRes = true;
				}
				else
				{
					bRes = false;
				}
			}
			else
			{
				bRes = false;
			}
		}
		else
		{
			bRes = false;
		}
		return bRes;
	}

};

class DLL_API RgbProcess
{
public:

	RgbProcess();
	~RgbProcess();
	
	void ProcessingImage(string& strImage, ColorRGB* arrDotsPixels, ConfigData* pConfig);

private:
	bool LoadInputImage(const string &strImage);

	bool PreProcessing(const Mat& image, Mat& imOut, bool bShowRGB = false);
	int FindThreshold(const Mat &image, bool bShow = false);

	bool GetRotation(const Mat& image, vector<Point2f>& vecCenter, float& angle, double dMinArea = 500, bool bShowTriDots = false);
	void RotateImage(Mat& image, float fAngleToRot);
	void GetROI(vector<Point2f>& vecCenter,Point2f& ptOutUL,Point2f& ptOutBR);

	bool ProcessingUnit(const Point2f& ptUL,const Point2f& ptBR);
	ColorRGB GetDotPixels(const Mat& imUnit, const Point2f& ptDot, int nRadius, ValueType eType);

public:
	Mat m_imThresh,m_imTrans;

protected:
	Mat m_imInput;

	int m_nRows;
	int m_nCols;
	bool m_bShowHistgram;
	bool m_bShowThreshold;
	bool m_bShowTriDots;
	bool m_bShowGrid;
	bool m_bShowRGB;
	ValueType m_eValueType;

	ColorRGB* m_arrDotsPixels;

	RNG	m_rng;
};

