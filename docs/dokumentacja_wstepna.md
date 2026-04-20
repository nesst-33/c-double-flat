# SPRAWOZDANIE WSTĘPNE

## Cel projektu
Celem projektu jest implementacja języka programowania ogólnego przeznaczenia. Język programowania ma spełniać następujące wymagania:
- Zmienne mają być statycznie typowane, słabe oraz domyślnie mutowalne
- Zmienne mają być przekazywane do funkcji przez referencje
- Zaimplementowany ma być specjalny typ złożony - kolekcje
	- Kolekcje mają przechowywać wartości o określonym typie
	- Należy zaimplementować min. 6 różnych operatorów dla kolekcji
## Zasady działania języka
### Nazywanie zmiennych
- Identyfikatory zmiennych i funkcji muszą zaczynać się znakiem ASCII. Każdy następny znak może być znakiem ASCII, cyfrą lub znakiem podłogi (\_)
	- np. dozwolona jest nazwa `nazwa_zm123`, ale `3nazwa` już nie
### Zakresy widoczności zmiennych
- Długość życia zmiennych opiera się na zakresach określanych przez klamerki ( {} )
- Zmienna zdefiniowana w danym zakresie "żyje" do jego zakończenia
- Zmienne zdefiniowane w "młodszych" zakresach przykrywają "starsze"
- Nie mogą zostać zdefiniowane dwie zmienne o tym samym identyfikatorze w jednym zakresie
- np.:
```
int a = 3 # zmienna statyczna, "żyje" przez cały okres działania programu

{
	int b = 4
	int a = 5  # Zmienna przykrywa zmienną a ze "starszego" zakresu
} # Wychodzimy z zakresu; zmienna a oraz b zostaje zniszczona

str a = "coś" # BŁĄD - identyfikator już zajęty
```
### Komentarze
- Komentarze oznaczane są znakiem krzyżyka (#)
- Dostępne są tylko komentarze jednolinijkowe 
```
# To jest komentarz
```
### Zapisywanie instrukcji
- Pełna instrukcja zajmuje jedną linijkę i kończy się znakiem nowej linii
	- Znak nowej linii == `\n`
- Zakazane jest więc wpisywanie kilku instrukcji w jednej linijce (jak np. w C++)
```
int a = 3
const int b = a + 3 # OK
int c = 3 const int d = c + 3 # BŁĄD
```

---
### Podstawowe typy danych
Język pozwala na wykorzystanie 4 podstawowych typów danych:
- Całkowitoliczbowy (`int`)
- Zmiennoprzecinkowy (`flp`)
- Tekstowy (`str`)
- Logiczny (`bool`)
#### `int`
- Oznaczany przez stałą liczbową
- W połączeniu z unarnym operatorem `-` reprezentuje liczbę ujemną
- Dozwolone jest wykorzystanie separatorów w celu poprawienia czytelności większych liczb. Znakiem oznaczającym separator będzie apostrof (')
	- np. 1'000'000 lub 10'00'000 jest tożsame liczbie 10000000
	- Apostrof nie może występować na początku ani na końcu liczby (aby nie uznać stałej za wartość tekstową)
#### `flp`
 - W pamięci zapisywany tak samo jak typ `double` w C
 - Separatorem dziesiętnym jest kropka (.)
 - Również dozwala używanie separatorów 
 - Przykłady:
	 - 3.14
	 - .14
	 - 0.14
	 - 1.
	 - -.454'454
#### `str`
- Stała tekstowa otoczona musi być apostrofami pojedynczymi lub podwójnymi (podobnie do Pythona)
- Wszelkie znaki specjalne muszą być "escapowane" symbolem \
	- Wyjątkiem jest sytuacja, gdy w stałej tekstowej oznaczonej pojedynczymi apostrofami zapisujemy podwójne apostrofy i vice versa (np. `str test = " ' "` jest poprawnym zapisem)
#### `bool`
- Dostępne są dwie stałe logiczne: `true` oraz `false`
- Wyrażenia logiczne zwracają typ `bool`
#### Mutowalność
- Jako, że język ma być domyślnie mutowalny, niemutowalność zmiennych musi być ręcznie ustawiona poprzez słowo kluczowe `const`:
```
const int liczba_niemut = 123344
liczba_niemut += 1 # BŁĄD
```
### Konwersje typów podstawowych
#### Niejawne
- Jako, że język ma być słabo typowany konieczne jest wykorzystanie rozbudowanego systemu niejawnych konwersji typów

##### Operatory arytmetyczne (+, -, /, \*, %)
- Poniższa tabela opisuje, do jakiego typu zostaną zrzutowane argumenty działań z ww. operatorami (gdzie wiersze to typy pierwszego argumentu, a kolumny drugiego):

|          | int         | flp | str         | bool        |
| -------- | ----------- | --- | ----------- | ----------- |
| **int**  | int         | flp | int/flp(\*) | int         |
| **flp**  | flp         | flp | flp         | flp         |
| **str**  | int/flp(\*) | flp | int/flp(\*) | int/flp(\*) |
| **bool** | int         | flp | int         | int         |

- Na ogół staramy się zachować jak największą ilość danych w działaniu:
	- Z tego powodu faworyzujemy typ `flp` - tzn. zarówno `4.5 + 4`, jak i `4 + 4.5` da nam wynik `8.`
	- *zapisanie tego wyniku do zmiennej typu `int` będzie jednak skutkowało obcięciem miejsca dziesiętnego*
- Rzutowanie `str` jest szczególnym przypadkiem
	- typ, do którego rzutujemy `str` zależy od jego wartości
	- jeżeli wartość tekstowa reprezentuje liczbę całkowitą (np. `"434"`), to rzutujemy na `int`; jeżeli zmiennoprzecinkową (np. `".143"`), to rzutujemy na `flp`
	- (\*) Aby `str` był rzutowany na wartość liczbową, musi zawierać tylko cyfry, separator dziesiętny, oraz ewentualnie apostrof
		- `"43", "43.4", "434'000", ".14"` to przykłady `str` rzutowalnych
		- próba rzutowania np. `"a"` na `int` zwróci błąd
- Konwersje `bool <-> int/flp` będą zachodziły w następujący sposób:
```
true -> 1
false <-> 0

0 -> false
wszystko inne -> true
```

##### Operatory tekstowe (~, !, :, [])
- Każdy typ podstawowy zostanie przekształcony na `str`

##### Operatory logiczne (and, or, not)
- Typy podstawowe będą przekształcane na `bool`:
	- `flp -> bool`
		- `0. -> false`
		- `wszystko inne -> true`
	- `int -> bool`
		- `0 -> false`
		- `wszystko inne -> true`
	- `str -> bool`
		- `"true" -> true`
		- `"false" -> false`
		- w przeciwnym przypadku błąd
#### Jawne
- Gdyby konwersje niejawne były niewystarczające/prowadziły do sytuacji nieprzewidzianych przez programistę, dostępna jest również jawna konwersja typów zmiennych
- Np:
```
3.5 as int # -> 3
3 as flp   # -> 3.
543 as str # -> "543"
true as str # -> "true"
```
- *Uwaga: obowiązują wszystkie zakazy wymienione w sekcji konwersji niejawnych - konwersje nie mające logicznego sensu będą odrzucane (obecnie są to tylko niepoprawne konwersje stringów)*
	- np:
```
Błędne kownersje:
"1" as bool
"asdf" as int
"def" as flp
```

---
## Operacje
### Operacje na danych liczbowych
- Język będzie dysponował podstawowymi operacjami matematycznymi na danych liczbowych:
	- dodawanie (+)
	- odejmowanie (-)
	- mnożenie (\*)
	- dzielenie (/)
	- modulo (%)
	- oraz połączenie wszystkich powyższych ze znakiem =:
		- +=, -=, \*=, /=, %=

### Operacje logiczne
-  Przyjmują zmienne typu `bool`:
	- negacja: `not a`
	- alternatywa: `a or b`
	- koniunkcja: `a and b`

### Operacje porównania i relacji
- Dokładna kalka z większości popularnych języków programowania ogólnego zastosowania (Python, C++, Java, itd...)
```
a == b
a != b
a < b
a > b
a <= b
a >= b
```
### Operacje na danych tekstowych
- Obecnie dla zmiennych typu `string` przewiduję następujące operacje:
	- konkatenacja (~)
		- np. "Ala " ~ "ma kota"
		- dostępny również w postaci operatora przypisania `~=`
	- liczba liter (!)
		- operator postfiksowy - zwraca liczbę liter
		- `"123"! = 3`
	- przecięcie (:)
		-  przecina wartość tekstową w danym indeksie
	```
	str ala = "Ala ma kota"	
	str kota = ala : 4 # kota = "ma kota"; ala = "Ala "
	```
	- indeksowanie ([]) 
		- dozwolony jest dostęp do poszczególnych liter wartości tekstowej
		- jeśli zmienna jest mutowalna, można również zmieniać poszczególne litery
	```
	str ala = "Ala ma kota"
	ala[2] = "e" # ala = "Ale ma kota"
	ala[0] = "La" # BŁĄD
	```

### Hierarchia Operatorów

| Pierwszeństwo | Operator                                                          | Opis                                                                                                                                    |
| ------------- | ----------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------- |
| **1**         | (wyrażenie)                                                       | Nawias                                                                                                                                  |
| **2**         | a as typ<br>a()<br>a[]<br>a!                                      | Jawna konwersja typów<br>Wywołanie funkcji<br>Indeksowanie<br>Moc kolekcji/liczba liter                                                 |
| **3**         | +a / -a<br>not a                                                  | Unarny plus/minus<br>Negacja (logiczna)<br>                                                                                             |
| **4**         | a * b<br>a / b<br>a % b                                           | Mnożenie<br>Dzielenie<br>Modulo<br>                                                                                                     |
| **5**         | a + b<br>a - b                                                    | Dodawanie<br>Odejmowanie                                                                                                                |
| **6**         | a ~ b<br>a & b<br>a : b<br>a << b<br>a >> b                       | Konkatenacja<br>Część wspólna kolekcji<br>Przecięcie kolekcji/stringa<br>Dodanie elementu do kolekcji<br>Ekstrakcja elementu z kolekcji |
| **7**         | a < b<br>a > b<br>a <= b<br>a >= b                                | Operatory relacyjne                                                                                                                     |
| **8**         | a == b<br>a != b                                                  | Operatory porównania                                                                                                                    |
| **9**         | a and b                                                           | Koniunkcja                                                                                                                              |
| **10**        | a or b                                                            | Alternatywa                                                                                                                             |
| **11**        | a = b<br>a += b<br>a -= b<br>a *= b<br>a /= b<br>a %= b<br>a ~= b | Operatory przypisania                                                                                                                   |

## Instrukcje warunkowe/pętli
- W celu uproszczenia implementacji projektu, okroję dostępne instrukcje warunkowe/pętli dostępne w innych popularnych językach do absolutnego minimum (choć nie wykluczam ich rozszerzenia w przyszłych iteracjach projektu)
- Przewiduję:
	- instrukcje if/else
	- pętlę while
- Będą one działać podobnie do innych popularnych języków programowania:
```
if (condition) {...}
else {...}

while (condition) {...}
```
- W przypadku wykorzystania pojedynczych instrukcji w ciele instrukcji warunkowych/pętli można pominąć nawiasy:
```
if (condition) statement
else statement

while (condition) statement
```

---
## Definiowanie i wołanie funkcji
- Funkcje będą definiowane i wywoływane w standardowy sposób (według mnie najbardziej wygodny w przypadku języków statycznie typowanych):
```
<typ> nazwa_funkcji(lista argumentów) {...}

# np.:
const int add(int a, int b)
{
	return a + b
}

int a = 3
int b = 5
const int c = add(a, b)

void glob()
{
	a += 3
	return
}
```
- Zmienne będą przekazywane poprzez **referencję**, na co trzeba szczególnie uważać!
	- Dlatego też, póki co, nie przewiduję żadnego jawnego mechanizmu wskaźników lub referencji
- Literały będą przekazywane przez kopię
	- Ograniczenie przekazywania literałów do przekazywania przez referencję wprowadziłoby spory problem implementacyjny. Jeżeli literał zostanie przekazany przez zwykłą referencję, zniknie po wejściu w funkcję. Jeżeli z kolei zmusimy użytkownika do przekazywania literałów w postaci parametrów `const`, wprowadza to komplikacje w pisaniu uniwersalnych funkcji 
- Wynik będzie przekazywany przez **kopię**
- Po wywołaniu `return` funkcja zwraca odpowiednie wyrażenie i kończy swoje działanie
- Funkcja jest widoczna tylko w swoim zakresie
- Przeciążanie funkcji jest zakazane (język jest słabo typowany - to rozwiązanie powinno wystarczyć)

---
## Kolekcje
- Kolekcja `arr` to typ danych, który będzie przechowywał określoną liczbę danych pewnego typu
- Kolekcje będą *homogeniczne* oraz *uporządkowane*
	- W działaniu będzie zasadniczo podobna do `std::vector` z C++, czy tablic w Pythonie
### Inicjalizacja i definiowanie
- Kwestie alokacji będą ukryte przed użytkownikiem - wielkość alokowanej pamięci na kolekcję będzie się automatycznie powiększać/zmniejszać przy dodawaniu/usuwaniu elementów z kolekcji
- Definiowana będzie w następujący sposób:
```
arr <typ> nazwa_zmiennej = [elem1, elem2, ...]

arr int liczby # tworzy pustą tablicę
arr str = ["abc", 4, 4.5] # tworzy kolekcję 3-elementową ["abc", "4", "4.5"]
const arr coords = [3, 4] # tworzy niemutowalną kolekcję (podobnie do krotek w Pythonie)
liczby = [1, 2, 3] # zmienia zawartość tablicy

arr arr int = [[1, 2], [1, 2, 3]] # tworzenie tablic zagnieżdżonych
```
- Zaznaczyć należy, że wartości zmiennych używanych do inicjalizacji (oraz dodawanych po inicjalizacji) będą **przekazywane przez referencję**
	- Takie rozwiązanie będzie powodowało problemy (co widać na przykład poniżej), ale jest to wymuszone specyfikacją zadania
```
arr str test
{
	str elem = "asdf"
	test = [elem]	
}
# test[0] = ?
```
### Adresacja
- Wszystkie elementy kolekcji są indywidualnie adresowalne poprzez indeks (zaczynając od 0) 
	- Użycie indeksu wykraczającego poza wielkość kolekcji zwróci błąd
- Każdy element tablicy może zostać zmieniony za pomocą adresowania
```
arr int liczby = [3, 4, 5, 6]
liczby[0] = 7  # [7, 4, 5, 6]
liczby[4] # BŁĄD
```
### Operatory
- Dodawanie elementu (<<) - dodaje element na koniec kolekcji
```
arr int a = [1, 2]
a << 3 # a = [1, 2, 3]
```

- Ekstrakcja elementu (>>) - usuwa element z kolekcji na danym indeksie i go zwraca
```
arr int a = [1, 2, 3]
int b = a >> 2 # b = 3
```

- Konkatenacja (~) - skleja dwie kolekcje
```
arr int a = [1, 2] ~ [3, 4]
```

- Różnica kolekcji (-) - usuwa każde wystąpienie podkolekcji z kolekcji
```
arr int a = [1, 2, 1, 2, 3] - [1, 2]
# a = [3]
```

- Przecięcie kolekcji (:) - wycina wszystkie elementy od wybranego indeksu i zwraca je jako kolekcja
```
arr int a = [1, 2, 3, 4]
arr int b = a : 2        # a = [1, 2]; b = [3, 4]
```

- Moc zbioru (a!) - zwraca liczbę elementów w zbiorze
```
arr int a = [1, 2, 3]
int b = a! # b = 3
```

- Mnożenie kolekcji (\*) - powtarza zbiór określoną ilość razy
```
arr int a = [1, 2]
arr int b = a * 3   # b = [1, 2, 1, 2, 1, 2]
```

- Część wspólna (&) - zwraca część wspólną zbiorów
```
arr int a = [1, 2, 3] & [2, 3, 4] # a = [2, 3]
```

- Filtracja elementów - zwraca kolekcję elementów spełniające dany predykat
```
arr int a = [1, 2, 4, 5, 6]
arr int b = a[>3] # b = [4, 5, 6]
```

- Mapowanie względem działania/funkcji - może być łączone z filtracją
	- w przypadku wykorzystania funkcji, jej typ wynikowy musi odpowiadać typowi kolekcji, oraz musi przyjmować min. jeden argument odpowiadający elementowi kolekcji (reprezentowany placeholderem - _)
```
arr int a = [3, 4, 5]
arr int b = a[*3]    # b = [9, 12, 15]
arr int c = a[>4][*2] # c = [10]
arr int d = a[between(_, 2, 4)] # d = [3]

bool between(int el, int lower, int upper)
{
	return lower < el and el < upper
}
```

### Rzutowanie tablic
- W przypadku operacji dwuargumentowych na tablicach różnych typów (konkatenacja, część wspólna), kolekcja będąca prawym argumentem zostanie przekonwertowana na typ kolekcji będącej lewym argumentem
```
arr str lit = ["a", "b", "c"]
arr int liczby = [1, 2, 3]
lit ~ liczby # ["a", "b", "c", "1", "2", "3"]
```
- Tego samego można dokonać z pomocą rzutowania jawnego
- Rzutowanie pomiędzy tablicami o różnych stopniach zagnieżdżenia jest **zakazane**
```
[[1, 2, 3]] as arr int # BŁĄD
```
- Nie można rzutować kolekcji na typy podstawowe poza `str`
```
[1, 2, 3] as int # BŁĄD
[1, 2, 3] as str # "[1, 2, 3]"
```


---
## Obsługa błędów
### Typy komunikatów + ich obsługa
#### 1. Błędy "odzyskiwalne"
- Niepożądane sytuacje, które mogą wystąpić w trakcie działania programu, z którymi program "wie" jak sobie poradzić
- Po napotkaniu takiego błędu interpreter dalej będzie działał
#### 2. Błędy krytyczne (panic)
- Błędy, w których stan programu jest naruszony i dalsza praca grozi np. wyciekiem danych lub błędy składniowe, typów (type mismatch) itd.
- Każdy błąd, który nie został zaklasyfikowany do błędów odzyskiwalnych
#### 3. Ostrzeżenie
- W sytuacjach, które są teoretycznie poprawne, ale mogą w przyszłości doprowadzić do błędu, zostanie wyświetlone ostrzeżenie
- Przewiduję wprowadzenie tylko kilku ostrzeżeń w ramach dowodu koncepcji
### Przykładowe komunikaty o błędach
#### Błąd "odzyskiwalny"
```
[ERROR] Error: Collection Index Out of Bounds
File: file.ds, Line: 42
Statement: return 2
--------------------------------------------------
Details: No closing bracket after return from function
```
#### Błąd krytyczny
```
[FATAL] Critical Error: Type Mismatch in Arithmetic Operation
File: file.ds, Line: 22
Statement: int wynik = "test" - 5
--------------------------------------------------
Details: Cannot perform subtraction (-) between type 'str' ("test") and type 'int' (5). 
The language has no implicit conversion rule from 'str' to 'int' for non-numeric characters.
```
#### Ostrzeżenie
```
[WARNING] Potential Data Loss (Implicit Conversion)
File: file.ds, Line: 18
Statement: int a = 4.9 + 2
--------------------------------------------------
Details: Implicit conversion of the 'flp' result (6.9) to the declared 'int' type caused truncation of the decimal part (result: 6). 
If this is intentional, use explicit conversion: `(4.9 + 2) as int` to silence this warning.
```

---
## Przykłady

```
# Formatowanie liczb całkowitych:

int calkowita = 1
int duza = +1'000'00001
int mala = -3'000
int sep = 7'7'7 # Dozwolona jest dowolna ilość separatorów 
# Nie mogą one jednak występować obok siebie

# Formatowanie liczb zmiennoprzecinkowych:

flp przecinkowa = 0.333
flp przecinkowa2 = 0.333'33
flp przecinkowa3 = -.33


# Zapisywanie zmiennych tekstowych:

str napis1 = "ok"
str napis2 = 'ok'
str napis3 = "'ok'" # zapisuje 'ok' (zainspirowane Python'em) 
str napis4 = "\"" # zapisuje " (znaki specjalne muszą być escapowane)

# Przykłady niejawnych konwersji:

str wynik = "10" + 5      # wynik = "15"
int x = true + true + 10  # x = 12
flp y = true + 0.5        # y = 1.5

# Niejawne konwersje w kolekcjach

arr int liczby = [1, 2, "3.5", true] 
# "3.5" -> 3
# true -> 1
# liczby = [1, 2, 3, 1]

# Przypadki brzegowe operacji na kolekcjach

arr int pusta = []
arr int y = pusta[>10]  # Przypisze do y pustą kolekcję

arr int a = [1, 2] * 0  # Wynikiem będzie []
arr int b = [1, 2] * -1 # Błąd krytyczny

# Dodawanie wartości do kolekcji

arr int globalna
{
	int x = 5
	globalna = [x]
} 
# Po wyjściu z zakresu zmienna x zostanie zniszczona...
# Mimo to, globalna = [5], ponieważ dokonujemy kopii wartości

# Przykrywanie zmiennych

int a = 10
int fun(int a) # Przykrywamy globalne a
{
	{
		int a = 5  # Kolejne przykrycie
		return a   # Funkcja zwróci 5	
	}
}

# Przekazywanie przez referencję

int zmiana(int ref) 
{
	ref += 1
	return ref
}
const int stala = 10
int wynik = zmiana(stala)  # błąd krytyczny (const został pogwałcony)

int stala2 = 10
int wynik2 = zmiana(stala2)
# Teraz stala2 = 11 i wynik2 = 11

wynik2 += 1
# wynik2 = 12; stala2 = 11 (wynik funkcji jest kopią, nie referencją)

# Przekazywanie przez referencję elementu tablicy

int modyfikuj(int element)
{
	element = 99
	return 1
}
arr int liczby = [1, 2, 3]
modyfikuj(liczby[0]) # liczby = [99, 2, 3]

# Operatory i priorytety

bool wynik = not 5 + 2 * 3 ^ 2 == 23 and 10 / 2 > 4
# Rozpisanie priorytetów: 
# 1. 3 ^ 2 = 9 
# 2. 2 * 9 = 18 
# 3. 5 + 18 = 23 
# 4. not 23 -> false (bo 23 to true) 
# 5. false == 23 -> false 
# 6. 10 / 2 = 5 # 7. 5 > 4 -> true 
# 8. false and true -> false

# Konkatenacja a dodawanie

str a = "Wynik: " ~ 3 + 4    # Błąd krytyczny
str b = "Wynik: " ~ 3 ~ 4    # b = "Wynik: 34"
str c = 3 + 4 ~ "Wynik"      # b = "7Wynik"

# Aliasing

int globalna = 10

int zepsuj(int a, int b)
{
	a = 99
	return b
}

int wynik = zepsuj(globalna, globalna) 
# wynik = 99
# b również zostanie zmodyfikowane (problem przekazywania przez 
# referencję)

```

### Obliczanie n-tej liczby Fibonacciego

```
int fib(int n)
{
	if (n <= 1)
	{
		return n
	}
	return fib(n - 1) + fib(n - 2)	
}
```

