#include <conio.h>
#include <iostream>
#include <filesystem>
#include <stdio.h>
#include <windows.h>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/opencv.hpp>


//inicjalizacja interfejsu
#define CVUI_IMPLEMENTATION
#include "opencv2/cvui.h"
#include "opencv2/EnhancedWindow.h" 
#define WINDOW_NAME	"Detektor pozarow i eksplozji"


//skroty klawiszowe
#define ESC 27
#define KAMERA 49
#define FILM 50

using namespace cv;
using namespace std;
namespace fs = std::experimental::filesystem;
using namespace System;


//zmienne globalne dla sciezek
string videosPath = "C:/Users/joann/Desktop/opencv/videos/"; //sciezka do folderu z filmami
string outputpath = "C:/Users/joann/Desktop/opencv/output/"; //sciezka do folderu output

//zmienne globalne dla alarmu
bool fire = false;
bool explosion = false;
int choice;


//funkcja wyboru zrodla obrazu
int menu() {

	cout << "Wybierz tryb:" << endl;
	cout << "1. KAMERA" << endl;
	cout << "2. FILM" << endl;
	cout << endl;
	cout << "Wyjscie: ESC" << endl;
	choice = _getch();
	cout << endl;

	if (choice == KAMERA) {
		cout << "Wybrano tryb: KAMERA" << endl;
	}
	else if (choice == FILM) {
		cout << "Wybrano tryb: FILM" << endl;
	}
	else if (choice == ESC) {
		cout << "Zamykam program, nacisnij dowolny klawisz, zeby kontynuowac" << endl;
	}
	else {
		cout << "Niepoprawnie wybrano tryb, nacisnij dowolny klawisz, zeby kontynuowac" << endl;
	}
	_getch();
	cout << endl;
	return choice;
}

//funkcja pozwalajaca na wybor video
string videoChoice() {
	cout << "Wybierz video:" << endl;
	//wypisanie dostepnych plikow video
	int i = 1;
	for (auto entry : fs::directory_iterator(videosPath)) {
		cout << i << ". ";
		cout << entry.path().filename() << endl;
		i++;
	}

	//wybor video
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

//funkcja przygotowujaca date i czas 
string prepareDateTime(string purpose) {
	SYSTEMTIME lt;
	GetLocalTime(&lt);
	string locDate, locTime, locDateTime;
	if (purpose == "frame") {
		locDate = format("%02d.%02d.%04d", lt.wDay, lt.wMonth, lt.wYear);
		locTime = format("%02d:%02d:%02d", lt.wHour, lt.wMinute, lt.wSecond);
		locDateTime = locDate + " " + locTime;
	}
	else if (purpose == "filename") {
		locDate = format("%02d-%02d-%04d", lt.wDay, lt.wMonth, lt.wYear);
		locTime = format("%02dh%02dm%02ds", lt.wHour, lt.wMinute, lt.wSecond);
		locDateTime = locDate + "_" + locTime;
	}
	return locDateTime;
}

//funkcja sprawdzajaca czy piksel posiada kolor taki jak ogien
bool isFireColor(Mat frameY, Mat frameCr, Mat frameCb, const int row, const int column, int threshold, int meanY) {
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

//fukcja zapisujaca wideo z wykrytym pozarem
VideoWriter initializeVideo(string outputVideoName, VideoWriter video) {
	string outputPath = outputpath + outputVideoName;
	int codec = CV_FOURCC('M', 'J', 'P', 'G');
	Size size = Size(640, 480);
	video.open(outputPath, codec, 15, size, true);
	return video;
}

int recordVideo(VideoWriter video, Mat fireDetectionFrame) {
	if (fire) video.write(fireDetectionFrame);
	return 0;
}

int get_largest_contour_id(vector <vector<cv::Point>> contours) {
	double largest_area = 0;
	int largest_contour_id = 0;
	for (int i = 0; i < contours.size(); i++) {
		double area = contourArea(contours.at(i));
		if (area > largest_area) {
			largest_area = area;
			largest_contour_id = i;
		}
	}
	return largest_contour_id;
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
	int minArea = 100;
	bool initialized = false;
	bool use_fire = true;
	bool use_explosion = true;
	bool use_source_of_fire = false;
	bool videoInitialized = false;
	int fps;
	Scalar meanYCrCb;
	Rect boundRect;
	Rect currentBound;
	Rect oldBound;
	VideoWriter video;
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	vector<Point> largest_contour;

	//ekstraktor tla MOG2
	Ptr<BackgroundSubtractorMOG2> pMOG2;
	pMOG2 = createBackgroundSubtractorMOG2();

	while (true) {
		// wybor zrodla obrazu
		int mode = menu();

		switch (mode) {
		case KAMERA: {
			cap.open(0); //otwarcie kamery
			fps = 50;
			break;
		}
		case FILM: {
			string path = videoChoice();
			cap.open(path); //wczytanie pliku wideo
			fps = cap.get(CAP_PROP_FPS);
			break;
		}
		case ESC: {
			return 0;
		}
		default: {
			return 1;
		}
		}

		//komunikat o poprawnosci inicjalizacji przechwytywania
		if (!cap.isOpened()) {
			cout << endl << "Inicjalizacja przechwytywania zakonczona niepowodzeniem." << endl;
		}
		else {
			cout << endl << "Inicjalizacja przechwytywania zakonczona powodzeniem." << endl;
		}

		//stworzenie okna dla wyswietlanego obrazu i panelu sterowania
		EnhancedWindow settings(368, 2, 270, 210, "Panel sterowania");
		cvui::init(WINDOW_NAME);
		namedWindow(WINDOW_NAME);

		//petla wykrywania pozaru/wybuchu
		while (cap.read(frame)) {

			//kopia klatki 
			fireDetectionFrame = frame.clone();

			//inicjalizacja czasu oraz nazwy pliku
			string datetimeFrame = prepareDateTime("frame");
			string datetimeFile = prepareDateTime("filename");
			string videoName = "fire_" + datetimeFile + ".avi";

			//umieszczenie daty i czasu na obrazie
			putText(fireDetectionFrame, datetimeFrame, Point(15, 465), FONT_HERSHEY_SIMPLEX, 1.25, Scalar(255, 255, 255), 2);

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

			//obliczenie wartosci sredniej luminacji
			meanYCrCb = mean(frame);
			int meanY = meanYCrCb[0];

			//utworzenie wektora pikseli pozaru
			vector<Point> firePixels;

			//sprawdzenie dla kazdego piksela czy jest na pierwszym planie i czy spelnia zalozenia koloru ognia
			for (int y = 0; y < fireFrameYCrCb.rows; y++) {
				for (int x = 0; x < fireFrameYCrCb.cols; x++) {
					if (!use_source_of_fire) {
						if (motionMask.at<uchar>(y, x) > 0 && isFireColor(fireFrameY, fireFrameCr, fireFrameCb, y, x, threshold, meanY)) {
							fireMask.at<uchar>(y, x) = 255;
							//dodanie piksela do wektora pikseli pozaru
							firePixels.push_back(Point(x, y));
						}
						else {
							fireMask.at<uchar>(y, x) = 0;
						}
					}
					else {
						if (isFireColor(fireFrameY, fireFrameCr, fireFrameCb, y, x, threshold, meanY)) {
							fireMask.at<uchar>(y, x) = 255;
						}
						else {
							fireMask.at<uchar>(y, x) = 0;
						}
					}
				}
			}

			//wyznaczenie prostokata wskazujacego obszar pozaru
			if (use_source_of_fire) {
				findContours(fireMask, contours, hierarchy, CV_RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));
				if (contours.size() > 0) {
					int largest_contour_id = get_largest_contour_id(contours);
					largest_contour = contours[largest_contour_id];
					boundRect = boundingRect(largest_contour);
					currentBound = boundRect;
				}
			}
			else {
				boundRect = boundingRect(firePixels);
				currentBound = boundRect;
			}

			//wykrywanie pozaru
			if (use_fire == true) {
				if (boundRect.area() > minArea && (!use_source_of_fire || (use_source_of_fire && contours.size() > 0))) {
					//zapis pozaru do zmiennej
					fire = true;
					//narysowanie prostokata
					rectangle(fireDetectionFrame, boundRect.tl(), boundRect.br(), CV_RGB(0, 255, 0), 2);
					//umieszczenie napisu ALARM
					putText(fireDetectionFrame, "ALARM!", Point(15, 35), FONT_HERSHEY_SIMPLEX, 1.5, Scalar(0, 0, 255), 3);
				}
				else {
					fire = false;
				}
			}

			//wykrywanie eksplozji
			if (use_explosion == true) {
				if (currentBound.area() > minArea && currentBound.area() > 1.5*oldBound.area() && (!use_source_of_fire || (use_source_of_fire && contours.size() > 0))) {
					//zapis eksplozji do zmiennej
					explosion = true;
					//umieszczenie napisu EKSPLOZJA
					putText(fireDetectionFrame, "EKSPLOZJA", Point(15, 75), FONT_HERSHEY_SIMPLEX, 1.5, Scalar(0, 0, 255), 3);
					//obecna klatka staje sie stara klatka
					oldBound = currentBound;
				}
				else {
					explosion = false;
				}
			}

			//uruchomienie zapisu do pliku
			if (fire == true && choice == KAMERA) {
				//zainicjowanie pliku wideo
				if (!videoInitialized) {
					video = initializeVideo(videoName, video);
					videoInitialized = true;
				}
				recordVideo(video, fireDetectionFrame);
			}
			else {
				video.release();
			}

			if (videoInitialized && fire == false && choice == KAMERA) {
				videoInitialized = false;
			}

			//dostosowanie panelu sterowania
			settings.begin(fireDetectionFrame);
			if (!settings.isMinimized()) {
				cvui::text("Treshold", 0.4, 0xff0000);
				cvui::trackbar(settings.width() - 20, &threshold, 1, 150);
				cvui::text("Min Area", 0.4, 0xff0000);
				cvui::trackbar(settings.width() - 20, &minArea, 1, 10000);
				cvui::space(10);
				cvui::checkbox("Detekcja pozaru", &use_fire);
				cvui::space(5);
				cvui::checkbox("Detekcja eksplozji", &use_explosion);
				cvui::space(5);
				cvui::checkbox("Zrodlo pozaru", &use_source_of_fire);
			}
			settings.end();

			cvui::image(fireDetectionFrame, 10, 10, fireFrameYCrCb);

			try {

				//wyswietlenie obrazow
				cvui::imshow(WINDOW_NAME, fireDetectionFrame);
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
		//wyczyszczenie konsoli
		system("cls");
	}
	//zamkniecie wszystkich okien i zwolnienie pamieci
	cap.release();
	destroyAllWindows();
	return 0;
}