# Temat projektu
Napisać program, który wykrywa nocne pożary i eksplozje, a następnie wywołuje alarm, na podstawie obrazu rejestrowanego z kamery i plików video. Dużo przykładów pożarów i eksplozji można znaleźć na kanale youtube, w tym również spowodowane wystrzałami z broni. W przypadku gdy rozpoznawanie „pożarów i eksplozji” w nocy okaże się łatwe do zrealizowania, to można również spróbować zrealizować to zadanie dla pory dziennej. 

# Podejście do problemu:
- Wykrywanie polega na dwóch podstawowych podejściach: wykryciu pierwszego planu i odpowiednich zależnościach pomiędzy kanałami Y, Cb i Cr przestrzeni kolorów YCbCr. Zależności pomiędzy kanałami dotyczą zarówno konkretnych wartości pikseli, jak i (w przypadku luminacji) wartości średniej całej klatki.

# Zrealizowane funkcje:
- menu z wyborem źródła: kamera i plik video (po zakończonym/przerwanym wykrywaniu pożaru/wybuchu następuje powrót do menu)
- aktualny czas i data wyświetlany na obrazie
- wykrywanie pożaru oraz eksplozji w porze dziennej i nocnej
- zaznaczanie pożaru na video
- sygnalizowanie wykrytego pożaru oraz eksplozji stosownym komunikatem na ekranie
- zapisywanie video po wykryciu pożaru (tylko dla kamery) do pliku
- tryb całego pożaru/źródła pożaru
- interfejs oparty na cviu.h i EnhancedWindow.h umożliwiający modyfikowanie progów w czasie rzeczywistym oraz zarządzanie funkcjami programu

# Hardware
Do programu został zbudowany interpreter sprzętowy oparty o płytkę Arduino Uno imitujący syrenę alarmową. 
Więcej informacji na temat koncepcji, budowy oraz sposobu działania znajduje się z folderze Arduino.
