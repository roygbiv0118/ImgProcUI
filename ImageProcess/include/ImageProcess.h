#pragma once

#ifdef DLL_EXPORTS
#define DLL_API _declspec(dllexport)
#else
#define DLL_API _declspec(dllimport)
#endif

#define B_SWITCH_SAVE 1;

#define CHIP_ROWS 6
#define CHIP_COLS 8

#define UNIT_ROWS 5
#define UNIT_COLS 6

#include <opencv2/opencv.hpp>
using namespace cv;

#include <sstream>
#include <functional>
#include <vector>
#include <string>
#include <set>
#include <memory>
using namespace std;

class DLL_API ImageProcess
{
public:
	ImageProcess();
	~ImageProcess();
	
	void ProcessingImage(string& strImage, string& strSave, unique_ptr<float[]>& upUnitsPixels);

private:
	bool LoadInputImage(const string &strImage);

	bool PreProcessing(const Mat& image,Mat& imOut);
	float GetRotation(Mat& image, vector<Point2f>& vecCenter);
	void RotateImage(float fAngleToRot, Mat& imOutRot);
	void GetROI(vector<Point2f>& vecCenter,Point2f& ptOutUL,Point2f& ptOutBR);
	void DivideChip(Point2f& ptUL, Point2f& ptBR);

	bool ProcessingUnits(const Mat& matUnit, int nRowChip, int nColChip, bool bShow = false);
	bool DivideUnits(vector<Point2f>& vecDots, unique_ptr<Point2f[]>& uptrDots, Size& minSize);
	float GetUnitPixels(const Mat& imUnit, const Point2f& ptDot, int nRadius);
	float WriteCSV();

protected:
	Mat m_imInput;
	string m_strSavePath;

	float m_ratioWB;//width to blank
	float m_ratioHB;//height to blank

	unique_ptr<float[]>* m_pUnitsPixels;
};

