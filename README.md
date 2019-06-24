# Temat projektu
Napisać program, który wykrywa nocne pożary i eksplozje, a następnie wywołuje alarm, na podstawie obrazu rejestrowanego z kamery i plików video. Dużo przykładów pożarów i eksplozji można znaleźć na kanale youtube, w tym również spowodowane wystrzałami z broni. W przypadku gdy rozpoznawanie „pożarów i eksplozji” w nocy okaże się łatwe do zrealizowania, to można również spróbować zrealizować to zadanie dla pory dziennej. 

# Podejście do problemu:
- Wykrywanie polega na dwóch podstawowych podejściach: wykryciu pierwszego planu i odpowiednich zależnościach pomiędzy kanałami Y, Cb i Cr przestrzeni kolorów YCbCr. Zależności pomiędzy kanałami dotyczą zarówno konkretnych wartości pikseli, jak i (w przypadku luminacji) wartości średniej całej klatki.

# Zrealizowane funkcje:
- menu z wyborem źródła: kamera i plik video (po zakończonym/przerwanym wykrywaniu pożaru/wybuchu następuje powrót do menu)
- wykrywanie pożaru/eksplozji w porze dziennej i nocnej
- zaznaczanie pożaru na video
- zapisywanie video po wykryciu pożaru (tylko dla kamery)
- tryb całego pożaru/źródła pożaru
- interfejs do bieżącej modyfikacji trybów oraz progów