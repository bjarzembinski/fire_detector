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

//funkcja przygotowuj¹ca date i czas 
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
	bool use_fire = false;
	bool use_explosion = false;
	int fps;
	Scalar meanYCrCb;
	Rect boundRect;
	Rect currentBound;
	Rect oldBound;
	VideoWriter video;

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
		EnhancedWindow settings(10, 50, 270, 200, "Panel sterowania");
		cvui::init(WINDOW_NAME);
		namedWindow(WINDOW_NAME);


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

			//otoczka wypuk³a dla pikseli pozaru
			boundRect = boundingRect(firePixels);
			//zainicjowanie pliku wideo
			video = initializeVideo(videoName, video);
			//kopia otoczki wypuklej
			currentBound = boundRect;

			//wykrywanie pozaru
			if (use_fire == true) {
				if (boundRect.area() > minArea) {
					//zapis pozaru do zmiennej
					fire = true;
					//narysowanie prostokata
					rectangle(fireDetectionFrame, boundRect.tl(), boundRect.br(), CV_RGB(0, 255, 0), 2);
					//umieszczenie napisu ALARM
					putText(fireDetectionFrame, "ALARM!", Point(15, 35), FONT_HERSHEY_SIMPLEX, 1.5, Scalar(0, 0, 255), 3);
				}
				else fire = false;
			}

			//wykryanie eksplozji
			if (use_explosion == true) {
				if (currentBound.area() > 1.5*(oldBound.area())) {
					//zapis eksplozji do zmiennej
					explosion = true;
					//umieszczenie napisu EKSPLOZJA
					putText(fireDetectionFrame, "EKSPLOZJA", Point(15, 75), FONT_HERSHEY_SIMPLEX, 1.5, Scalar(0, 0, 255), 3);
					//obecna klatka staje sie stara klatka
					oldBound = currentBound;
				}
				else explosion = false;
			}

			//uruchomienie zapisu do pliku
			if (fire == true && choice == KAMERA)
				recordVideo(video, fireDetectionFrame);
			else video.release();

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
			}
			settings.end();

			try {

				//wyswietlenie obrazow
				cvui::imshow(WINDOW_NAME, fireDetectionFrame);
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
		system("cls");
	}
	//zamkniecie wszystkich okien i zwolnienie pamieci
	cap.release();
	destroyAllWindows();
	return 0;
}