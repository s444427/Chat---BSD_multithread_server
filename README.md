# GenericNetworkSender
Generic Network App for sending data between server and client(s)


Opis projektu:

Projekt składa się z dwóch części - klient i serwer. Serwer czeka na połączenie z klientem/klientami
i w odpowiedź na zapytanie klienta tworzy tekstowy plik z losowo wygenerowanymi danymi o wskazanej
przez klienta wielkości pliku. Po otrzymaniu pliku klient kończy działanie. Serwer jest wielowątkowy,
a zatem jest w stanie obsłużyć wielu klientów.


Projekt został napisany w 2 językach C i C#:
* serwer(język C)
* klient 1(język C)
* klient 2(język C#)


Jak skompilować:

Kompilacja klienta i serwera w języku C:
```
make mstuff
```

Kompilacja  i uruchomienie klienta w języku C#:
```
csc Program.cs
mono Program.exe
```

uruchom serwer, uruchom klienta, podaj adres serwera.

Przy udanym połączeniu w kliencie pojawi się symbol zachęty ```>>```.

Słownik poleceń:

```exit``` - przerwanie połączenia z serwerem

```dummy X``` - zapytanie o wysłanie pliku z losowymi danymi o wielkości X bitów (X musi być większe od zera)

Po udanym pobraniu z serwera, plik będzie się znajdował w tym samym folderze co program-klient, pod nazwą "dummy_recieved.txt" 


