# SYRENA ALARMOWA ARDUINO
# Sprzęt: 
- płytka Arduino UNO 
- płytka prototypowa
- odbiornik IRD - port 11 *
- przetwornik IRD *
- pilot bezprzewodowy *
- 5 diod sygnalizacyjnych:
  - zielona dioda gotowości - port 5
  - biała dioda kontrolna (detekcja pożaru) - port 10
  - biała dioda kontrolna (detekcja eksplozji) - port 11
  - czerwona dioda sygnalizująca wykrycie pożaru - port 6
  - żółta dioda sygnalizująca wykrycie eksplozji - port 7 
- buzzer pasywny - port analogowy 5 
- przewód komunikacji USB
- obudowa 

*do wersji zarządanej pilotem 

# Koncepcja 
W stanie gotowości - kiedy urządzenie alarmowe jest prawidłowo podłączone - zapalona jest dioda zielona. Jeżeli flaga wykrywania pożaru będzie oznaczona jako true zapalona zostanie biała dioda kontrolna na porcie 10. Jeżeli flaga wykrywania pożaru będzie oznaczona jako true zapalona zostanie biała dioda kontrolna na porcie 10. Jeżeli flaga wykrywania eksplozji będzie oznaczona jako true zapalona zostanie biała dioda kontrolna na porcie 9. W przypadku wykrycia pożaru zostanie uruchomiona dioda czerwona na porcie 6 oraz buzzer. W przypadku wykrycia eksplozji - dioda żółta na porcie 7.

# Oprogramowanie
Skrypt dla płytki został stowrzony w środowisku Arduino IDE. Komunikacja odbywa się przy pomocy portu szeregowego COM.
W Visual Studio wykorzystano bibliotekę Tserial do obsługi portu szeregowego bez konieczności konwersji na aplikację CLR. 
