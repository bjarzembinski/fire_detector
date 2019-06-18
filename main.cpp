#include <conio.h>
#include <iostream>
#include <filesystem>
#include <stdio.h>
#include <windows.h>
#include <vector>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>

#define ESC 27					

using namespace cv;
using namespace std;
namespace fs = std::experimental::filesystem;

//zmienne globalne
string videosPath = "C:/path/to/videos/";

// funkcja wyboru zrodla obrazu
int menu() {
	int choice;
	cout << "Wybierz tryb:" << endl;
	cout << "1. KAMERA" << endl;
	cout << "2. FILM" << endl;
	cout << endl;
	cout << "Twoj wybor:" << endl;
	cin >> choice;
	cout << endl;
	return choice;
}

// funkcja pozwalajaca na wybor video
string videoChoice() {
	cout << "Wybierz video:" << endl;
	//wypisanie dostepnych plikow video
	int i = 1;
	for (auto entry : fs::directory_iterator(videosPath)) {
		cout << i << ". ";
		cout << entry.path().filename() << endl;
		i++;
	}

	// wybor video
	int videoNumber;
	cout << endl << "Twoj wybor:" << endl;
	cin >> videoNumber;
	i = 1;
	for (auto entry : fs::directory_iterator(videosPath)) {
		if (i == videoNumber) {
			return entry.path().string();
		}
		i++;
	}
}

// funkcja sprawdzajaca czy piksel posiada kolor taki jak ogien
bool isFireColor(Mat frameY, Mat frameCr, Mat frameCb, const int row, const int column, int threshold, int meanY, int meanCr, int meanCb) {
	bool condition;
	int valueY = frameY.at<uchar>(row, column);
	int valueCr = frameCr.at<uchar>(row, column);
	int valueCb = frameCb.at<uchar>(row, column);

	if ((valueY > valueCb) && (valueCr > valueCb + threshold) && (valueY > meanY)) {
		return true;
	}
	else {
		return false;
	}
}

int main() {
	//deklaracja obiektu przechwytywania
	VideoCapture cap;

	//deklaracja macierzy obrazow
	Mat frame;
	Mat fireDetectionFrame;
	Mat fireFrameYCrCb;
	Mat fireFrameY;
	Mat fireFrameCr;
	Mat fireFrameCb;
	Mat fireMask;
	Mat motionMask;

	//deklaracja zmiennych
	int threshold = 60;
	int minArea = 0;
	bool initialized = false;
	int fps;
	Scalar meanYCrCb;
	Rect boundRect;

	//ekstraktor tla MOG2
	Ptr<BackgroundSubtractorMOG2> pMOG2;
	pMOG2 = createBackgroundSubtractorMOG2();

	// wybor zrodla obrazu
	int mode = menu();

	switch (mode) {
		case 1: {
			cap.open(0); //otwarcie kamery
			fps = 50;
			break;
		}
		case 2: {
			string path = videoChoice();
			cap.open(path); //wczytanie pliku wideo
			fps = cap.get(CAP_PROP_FPS);
			break;
		}
		default: {
			cout << "Niepoprawnie wybrano tryb." << endl; //komunikat o bledzie i zamkniecie programu
			return -1;
		}
	}

	//komunikat o poprawnosci inicjalizacji przechwytywania
	if (!cap.isOpened()) {
		cout << endl << "Inicjalizacja przechwytywania zakonczona niepowodzeniem." << endl;
	}
	else {
		cout << endl << "Inicjalizacja przechwytywania zakonczona powodzeniem." << endl;
	}

	//stworzenie okna dla wyswietlanego obrazu
	namedWindow("Fire Detection", WINDOW_AUTOSIZE);
	namedWindow("Parameters", WINDOW_AUTOSIZE);
	createTrackbar("Threshold", "Parameters", &threshold, 100);
	createTrackbar("Min Area", "Parameters", &minArea, 10000);

	while (cap.read(frame)) {
		//kopia klatki 
		fireDetectionFrame = frame.clone();

		//inicjalizacja maski ognia
		if (initialized == false) {
			fireMask = Mat(frame.rows, frame.cols, CV_8UC1);
			initialized = true;
		}
		
		//utworzenie maski ruchu 
		pMOG2->apply(frame, motionMask);
		erode(motionMask, motionMask, Mat(), Point(-1, -1), 2);
		dilate(motionMask, motionMask, Mat(), Point(-1, -1), 1);

		//konwersja obrazu do przestrzni YCrCb
		cvtColor(frame, fireFrameYCrCb, COLOR_BGR2YCrCb);

		//wyekstrahowanie wartosci poszczegolnych kanalow
		extractChannel(fireFrameYCrCb, fireFrameY, 0);
		extractChannel(fireFrameYCrCb, fireFrameCr, 1);
		extractChannel(fireFrameYCrCb, fireFrameCb, 2);

		//obliczenie wartosci srednich dla poszczegolnych kanalow
		meanYCrCb = mean(frame);
		int meanY = meanYCrCb[0];
		int meanCr = meanYCrCb[1];
		int meanCb = meanYCrCb[2];

		//utworzenie wektora pikseli pozaru
		vector<Point> firePixels;

		//sprawdzenie dla kazdego piksela czy jest na pierwszym planie i czy spelnia zalozenia koloru ognia
		for (int y = 0; y < fireFrameYCrCb.rows; y++) {
			for (int x = 0; x < fireFrameYCrCb.cols; x++) {
				if (motionMask.at<uchar>(y, x) > 0 && isFireColor(fireFrameY, fireFrameCr, fireFrameCb, y, x, threshold, meanY, meanCr, meanCb)) {
					fireMask.at<uchar>(y, x) = 255;
					//dodanie piksela do wektora pikseli pozaru
					firePixels.push_back(Point(x, y));
				}
				else {
					fireMask.at<uchar>(y, x) = 0;
				}
			}
		}
		
		//narysowanie prostokata wskazujacego obszar pozaru
		boundRect = boundingRect(firePixels);
		if (boundRect.area() > minArea) {
			rectangle(fireDetectionFrame, boundRect.tl(), boundRect.br(), CV_RGB(0, 255, 0), 2);
		}

		try {
			//wyswietlenie obrazow
			imshow("Fire Detection", fireDetectionFrame);
			//imshow("maska ruchu", motionMask);
			//imshow("maska ognia", fireMask);
			//imshow("Y", fireFrameY);
			//imshow("Cr", fireFrameCr);
			//imshow("Cb", fireFrameCb);
		}

		catch (Exception &e) {
			cout << e.what() << endl;
			return 1;
		}
		if (waitKey(1000 / fps) == ESC) {
			break;
		}
	}

	//zamkniecie wszystkich okien i zwolnienie pamieci
	cap.release();
	destroyAllWindows();
	return 0;
}