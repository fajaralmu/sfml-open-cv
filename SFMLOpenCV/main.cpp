#include <iostream>
#include "SFML\Graphics.hpp"
#include "SFML\Window.hpp"
#include "SFML\System.hpp"

#include <vector>
#include <cstdlib>
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\core\core.hpp"
#include "opencv2\opencv.hpp"
#include "opencv2\ml\ml.hpp"
#include <conio.h>


using namespace cv;
using namespace std;
using namespace sf;
const int winW = 800, winH = 550;

int x, y, delta;
void update(RectangleShape* p);
bool adagerak = false;
vector<RectangleShape> boxes;
vector<Point> pts;

//Treshold
int threshold_value = 57;
int threshold_type = 0;
int const max_value = 255;
int const max_type = 4;
int const max_BINARY_value = 255;
Mat tresholded;
Mat imageGrayScale;
Mat edge;
char* treshWin = "Threshold Demo";

char* trackbar_type = "Type: \n 0: Binary \n 1: Binary Inverted \n 2: Truncate \n 3: To Zero \n 4: To Zero Inverted";
char* trackbar_value = "Value";

void Threshold_Demo(int, void*);

//hsv
Mat gambarHsv, hsv_out;
//int hueMin = 111, valMin = 63, satMin = 54;
//int hueMax = 159, valMax = 99, satMax = 86;
//int hueMin = 138, valMin = 172, satMin = 51;
//int hueMax = 154, valMax = 224, satMax = 81;
int hueMin = 140, valMin = 77, satMin = 55;
int hueMax = 159, valMax = 102, satMax = 148;


//in out
void prosesGambar(Mat& in, Mat& out);
Mat gambar;



int main()
{
	srand(time(NULL));
	//WEBCAM
	VideoCapture cap(0);
	namedWindow("jendelaasli", CV_WINDOW_AUTOSIZE);
	namedWindow(treshWin, CV_WINDOW_AUTOSIZE);
	namedWindow("HSVTrck", CV_WINDOW_AUTOSIZE);

	//HSV
	createTrackbar("Hue min", "HSVTrck", &hueMin, 179);
	createTrackbar("Hue max", "HSVTrck", &hueMax, 179);
	createTrackbar("Sat min", "HSVTrck", &satMin, 255);
	createTrackbar("Sat max", "HSVTrck", &satMax, 255);
	createTrackbar("val min", "HSVTrck", &valMin, 255);
	createTrackbar("val max", "HSVTrck", &valMax, 255);

	//TRESH
	createTrackbar(trackbar_type,
		treshWin, &threshold_type,
		max_type, Threshold_Demo);

	createTrackbar(trackbar_value,
		treshWin, &threshold_value,
		max_value, Threshold_Demo);

	//GAME
	sf::RenderWindow window(sf::VideoMode(winW, winH), "Jump");
	//window.setFramerateLimit(60);
	Texture tx;
	tx.loadFromFile("player.png");
	RectangleShape p(Vector2f(30, 70));
	p.setTexture(&tx);
	p.setPosition(Vector2f(winW / 4, winH / 2));

	RectangleShape floor(Vector2f(winW, 10));
	floor.setPosition(Vector2f(0, winH / 2 + 70));
	floor.setFillColor(Color(0, 255, 0, 255));

	RectangleShape* b;
	b = new RectangleShape(Vector2f(10, 100));

	for (int i = 0; i < 3; i++) {
		int x_ = rand() % winW;
		boxes.push_back(RectangleShape(*b));
		boxes[i].setPosition(Vector2f(x_, winH / 2 + 10));
		boxes[i].setFillColor(Color(255, 255, 0, 255));
	}


	int i = 0;
	while (window.isOpen())
	{

		cap >> gambar;
		Event e;
		while (window.pollEvent(e)) {
			if (e.type == Event::Closed)
				window.close();
		}

		//WEBCAM
		flip(gambar, gambar, 2);

		cvtColor(gambar, gambarHsv, CV_BGR2HSV);
		Threshold_Demo(0, 0);
		//GARIS DAN LINGKARAN
		/*for (int i = 0; i < pts.size(); i++) {
			if (i == 0) continue;
			line(gambar, pts[i], pts[i - 1], Scalar(0, 0, 255), 2);

		}*/

		if (pts.size() > 0) {
			auto xS = std::to_string(pts.back().x), yS = std::to_string(pts.back().y), d = to_string(delta);
			//string pt = xS + "," + yS;
			string pt = "vel: " + d;
			putText(gambar, pt, Point(500, pts.back().y + 50), FONT_ITALIC, 1, Scalar(0, 255, 255), 2);
			circle(gambar, pts.back(), 20, Scalar(255, 0, 0), 5);
		}

		imshow("jendelaasli", gambar);
		//imshow("HSV", gambarHsv);
		cv::erode(tresholded, tresholded, cv::Mat());
		dilate(tresholded, tresholded, Mat());
		prosesGambar(tresholded, gambar);
		//imshow(treshWin,Mat());
		imshow("tresh", tresholded);

		//Update
		update(&p);
		i++;
		if (i >= 10) {
			pts.clear();
			i = 0;
		}
		//Draw
		window.clear(sf::Color::White);
		window.draw(p);
		window.draw(floor);
		for (int i = 0; i < boxes.size(); i++) {
			window.draw(boxes[i]);
		}
		window.display();
	}
	return 0;
}

void update(RectangleShape* p) {
	int up = sf::Keyboard::isKeyPressed(sf::Keyboard::Up) * (-1);
	p->move(0, up);
	//cout << p->getPosition().y << "/" << winH / 2 << endl;
	int size = pts.size();

	//if (pts[pts.size() - 1].y != pts[pts.size() - 2].y && pts.size()>1)
	if (pts.size() > 1 && p->getPosition().y > 100 && adagerak)
	{
		delta = pts[size - 2].y - pts[size - 1].y;
		if (delta < winH / 3)
			p->move(0, delta > 1 ? (pts[size - 1].y - pts[size - 2].y) * 2 : p->getPosition().y < winH / 2 ? 20 : 0);
	}
	else if (!up && p->getPosition().y < winH / 2) {
		p->move(0, 20);
	}
	else {
		p->move(0, 0);
	}

	for (int i = 0; i < boxes.size(); i++) {

		boxes[i].move(-10, 0);
		if (boxes[i].getPosition().x < 0)
			boxes[i].setPosition(Vector2f(winW, boxes[i].getPosition().y));
	}
	//p->setPosition(Vector2f(p->getPosition().x, y));

}

void Threshold_Demo(int, void*) {
	/* 0: Binary
	1: Binary Inverted
	2: Threshold Truncated
	3: Threshold to Zero
	4: Threshold to Zero Inverted
	*/
	inRange(gambarHsv, Scalar(hueMin, satMin, valMin), Scalar(hueMax, satMax, valMax), imageGrayScale);

	//cvtColor(hsv_out, imageGrayScale, CV_BGR2GRAY);
	string tipe = "";
	switch (threshold_type)
	{
	case 0:
		tipe = "binary: if the intensity of the pixel src(x,y) is higher than thresh, then the new pixel intensity is set to a MaxVal. Otherwise, the pixels are set to 0.";
		break;
	case 1:
		tipe = "binary inverted: If the intensity of the pixel src(x,y) is higher than thresh, then the new pixel intensity is set to a 0. Otherwise, it is set to MaxVal.";
		break;
	case 2:
		tipe = "treshold truncated : if src(x,y) is greater, then its value is truncated.";
		break;
	case 3:
		tipe = "tresh to zero: If src(x,y) is lower than thresh, the new pixel value will be set to 0.";
		break;
	case 4:
		tipe = "tresh to zero inverted: If  src(x,y) is greater than thresh, the new pixel value will be set to 0.";
		break;
	default:
		break;
	}
	//cout << "TIPE :" << tipe << endl;
	threshold(imageGrayScale, tresholded, threshold_value, max_BINARY_value, threshold_type);

}

void prosesGambar(Mat& in, Mat& out)
{
	adagerak = false;
	//output_image = input_image.clone();

	for (int row = 1; row < in.rows - 1; row++)
		for (int col = 1; col < in.cols - 1; col++)
		{
			//int tengah_atas = in.at<uchar>(row - 1, col);
			int kiri = in.at<uchar>(row, col - 1);
			int tengah = in.at<uchar>(row, col);
			int kanan = in.at<uchar>(row - 1, col + 1);
			//int tengah_bawah = in.at<uchar>(row + 1, col);

			if (
				//tengah_atas == 255 &&
				kiri == 255 &&
				kanan == 255 &&
				tengah == 255
				//tengah_bawah == 255
				)

			{
				x = col;
				y = row;
				auto xS = std::to_string(col);
				auto yS = std::to_string(row);
				pts.push_back(Point(col, row));
				adagerak = true;
				//	Mouse::setPosition(Vector2i(col, row));
				goto skip;
			}
		}



skip:
	;
}
