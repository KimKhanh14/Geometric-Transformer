#include "GeometricTransformer.h"

void AffineTransform::Translate(float dx, float dy) {
	float value[3][3] = { {1, 0, dx}
						, {0, 1, dy}
						, {0, 0, 1} };
	Mat matTranslate = Mat(3, 3, CV_32FC1, value);
	this->_matrixTransform = matTranslate * this->_matrixTransform;
}

void AffineTransform::Rotate(float angle) {
	//Chuyển radian sang độ
	float cosAngle = cos(angle * PI / 180);
	float sinAngle = sin(angle * PI / 180);
	float value[3][3] = { {cosAngle, -sinAngle, 0}
						, {sinAngle, cosAngle, 0}
						, {0, 0, 1} };
	Mat matRotate = Mat(3, 3, CV_32FC1, value);
	this->_matrixTransform = matRotate * this->_matrixTransform;
}

void AffineTransform::Scale(float sx, float sy) {
	float value[3][3] = { {sx, 0, 0}
						, {0, sy, 0}
						, {0, 0, 1} };
	Mat matScale = Mat(3, 3, CV_32FC1, value);
	this->_matrixTransform = matScale * this->_matrixTransform;
}

void AffineTransform::TransformPoint(float& x, float& y) {
	float v[] = { x, y, 1 };
	Mat matResult = this->_matrixTransform * (Mat(3, 1, CV_32FC1, v));
	x = matResult.ptr<float>(0)[0];
	y = matResult.ptr<float>(0)[1];
}

Mat AffineTransform::getmatrixTransform()
{
	return this->_matrixTransform;
}

void AffineTransform::setmatrixTransform(Mat matrixTransform) {
	this->_matrixTransform = matrixTransform;
}

AffineTransform::AffineTransform() {
	float value[3][3] = { {1, 0, 0}
						,{0, 1, 0}
						, {0, 0, 1} };
	this->_matrixTransform = Mat(3, 3, CV_32FC1, value).clone();
}

AffineTransform::~AffineTransform() {
	this->_matrixTransform.release();
}

PixelInterpolate::PixelInterpolate() {}
PixelInterpolate::~PixelInterpolate() {}

NearestNeighborInterpolate::NearestNeighborInterpolate() {}
NearestNeighborInterpolate::~NearestNeighborInterpolate() {}

uchar NearestNeighborInterpolate::Interpolate(float tx, float ty, uchar* pSrc, int srcWidthStep, int nChannels) {
	int l = (int)round(tx);
	int k = (int)round(ty);

	uchar* p = pSrc + srcWidthStep * l + nChannels * k;
	uchar res = p[0];

	return res;
}

BilinearInterpolate::BilinearInterpolate() {}
BilinearInterpolate::~BilinearInterpolate() {}

uchar BilinearInterpolate::Interpolate(float tx, float ty, uchar* pSrc, int srcWidthStep, int nChannels) {
	int l = (int)floor(tx);
	int k = (int)floor(ty);
	float a = tx - l;
	float b = ty - k;

	uchar* p1 = pSrc + srcWidthStep * l + nChannels * k;
	uchar* p2 = pSrc + srcWidthStep * (l + 1) + nChannels * k;
	uchar* p3 = pSrc + srcWidthStep * l + nChannels * (k + 1);
	uchar* p4 = pSrc + srcWidthStep * (l + 1) + nChannels * (k + 1);

	int flk = p1[0];
	int fl1k = p2[0];
	int flk1 = p3[0];
	int fl1k1 = p4[0];

	float sum = (1 - a) * (1 - b) * flk + a * (1 - b) * fl1k + (1 - a) * b * flk1 + a * b * fl1k1;
	uchar res = round(sum);
	return res;
}

GeometricTransformer::GeometricTransformer() {}
GeometricTransformer::~GeometricTransformer() {}

int GeometricTransformer::Transform(const Mat& beforeImage, Mat& afterImage, AffineTransform* transformer, PixelInterpolate* interpolator)
{
	if (beforeImage.empty()) //Kiểm tra có ảnh không
	{
		return 0;
	}

	//Khởi tạo vector
	float B[] ={0, 0, 1.0};
	Mat	Pos = Mat(3, 1, CV_32FC1, B);

	//Nội suy màu
	uchar* pSrc = beforeImage.data;
	Mat aff = transformer->getmatrixTransform();
	Mat ReverseAffine = aff.inv();
	for (int i = 0; i < afterImage.rows; i++) {
		uchar* pDes = afterImage.ptr<uchar>(i);
		for (int j = 0; j < afterImage.cols; j++) {
			Pos.ptr<float>(0)[0] = i;
			Pos.ptr<float>(1)[0] = j;

			//Affine ngược
			Mat tPosition = ReverseAffine * Pos;
			float tx = tPosition.ptr<float>(0)[0];
			float ty = tPosition.ptr<float>(1)[0];
			if(tx >=0 && ty>=0 && tx < beforeImage.rows && ty <beforeImage.cols)
				for (int k = 0; k < beforeImage.step[1]; k++) {
					pDes[j * beforeImage.step[1] + k] = interpolator->Interpolate(tx, ty, pSrc + k, beforeImage.step[0], beforeImage.step[1]);
				}
		}
	}
	return 1;
}

int GeometricTransformer::RotateKeepImage(const Mat& srcImage, Mat& dstImage, float angle, PixelInterpolate* interpolator) {
	if (srcImage.empty()) {
		return 0;
	}

	int rows = srcImage.rows;
	int cols = srcImage.cols;
	int widthStep = srcImage.step[0];
	int nChannels = srcImage.step[1];

	AffineTransform *AT = new AffineTransform();

	//Đưa tâm ảnh về gốc tọa độ của ảnh gốc
	AT->Translate(-rows / 2, -cols / 2);

	//Xoay ảnh
	AT->Rotate(angle);

	float X[] = { 0, 0, srcImage.rows, srcImage.rows };
	float Y[] = { 0, srcImage.cols, 0, srcImage.cols };

	for (int i = 0; i < 4; i++) {
		AT->TransformPoint(X[i], Y[i]);
	}

	//Lấy 4 điểm xa nhất theo 4 hướng
	float minx = std::min({ X[0], X[1], X[2], X[3] });
	float maxx = std::max({ X[0], X[1], X[2], X[3] });
	float miny = std::min({ Y[0], Y[1], Y[2], Y[3] });
	float maxy = std::max({ Y[0], Y[1], Y[2], Y[3] });

	//Kích thước ảnh mới
	int newRows = (int)floor(maxx - minx);
	int newCols = (int)floor(maxy - miny);

	//Đưa tâm ảnh về tâm ảnh mới
	AT->Translate(newRows / 2, newCols / 2);
	dstImage = Mat(newRows, newCols, CV_8UC3, Scalar(0));
	return this->Transform(srcImage, dstImage, AT, interpolator);
}

int GeometricTransformer::RotateUnkeepImage(const Mat& srcImage, Mat& dstImage, float angle, PixelInterpolate* interpolator) {
	if (srcImage.empty()) {
		return 0;
	}

	int rows = srcImage.rows;
	int cols = srcImage.cols;

	int widthStep = srcImage.step[0];
	int nChannels = srcImage.step[1];

	AffineTransform AT;

	//Đưa tâm ảnh về gốc tọa độ
	AT.Translate(-rows / 2, -cols / 2);

	//Xoay ảnh
	AT.Rotate(angle);

	//Đưa tâm ảnh về chính giữa
	AT.Translate(rows / 2, cols / 2);

	//Tạo khung cho ảnh đích với cùng kích thước với ảnh gốc
	dstImage = Mat(rows, cols, CV_8UC3, Scalar(0));

	Transform(srcImage, dstImage, &AT, interpolator);
	
	return 1;
}

int GeometricTransformer::Scale(const Mat& srcImage, Mat& dstImage, float sx, float sy, PixelInterpolate* interpolator)
{
	if (srcImage.empty() || interpolator == NULL) {
		return 0;
	}

	AffineTransform* AT = new AffineTransform();
	AT->Translate(-srcImage.rows / 2, -srcImage.cols / 2);
	AT->Scale(sx, sy);
	AT->Translate(srcImage.rows / 2, srcImage.cols / 2);
	dstImage = Mat(srcImage.rows, srcImage.cols, CV_8UC3, Scalar(0));
	return this->Transform(srcImage, dstImage, AT, interpolator);
}

int GeometricTransformer::Resize(const Mat& srcImage, Mat& dstImage, int newWidth, int newHeight, PixelInterpolate* interpolator)
{
	if (srcImage.empty() || interpolator == NULL || newWidth <= 0 || newHeight <= 0)
		return 0;

	//Lấy các thông tin cần thiết từ ảnh gốc
	uchar* pSrc = srcImage.data;
	int src_rows = srcImage.rows;
	int src_cols = srcImage.cols;
	int nChannel = srcImage.channels();

	//Thiết lập Affine
	float sx = (1.0 * newHeight) / (1.0 * src_rows);
	float sy = (1.0 * newWidth) / (1.0 * src_cols);
	float scaleArray[] = { sx, 0, 0, 0, sy, 0, 0, 0, 1 };

	Mat scale = Mat(3, 3, CV_32FC1, scaleArray).clone();
	AffineTransform* scale_affine = new AffineTransform(scale);

	if (nChannel == 3)
	{
		dstImage = Mat(ceil(src_rows * sx), ceil(src_cols * sy), CV_8UC3);
	}
	else
	{
		dstImage = Mat(ceil(src_rows * sx), ceil(src_cols * sy), CV_8UC1);
	}

	int check = Transform(srcImage, dstImage, scale_affine, interpolator);

	if (check == 0) {
		return 0;
	}
	return 1;
}

int GeometricTransformer::Flip(const Mat& srcImage, Mat& dstImage, bool direction, PixelInterpolate* interpolator)
{
	if (srcImage.empty() || interpolator == NULL || direction < 0 || direction > 1)
		return 0;

	AffineTransform *AT = new AffineTransform();

	if (direction) {
		AT->Scale(-1, 1);
		AT->Translate(srcImage.rows,0);
	}
	else {
		AT->Scale(1, -1);
		AT->Translate(0, srcImage.cols);
	}
	dstImage = Mat(srcImage.rows, srcImage.cols, CV_8UC3);
	return this->Transform(srcImage, dstImage, AT, interpolator);
};