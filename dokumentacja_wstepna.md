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
- W każdym działaniu, gdzie występuje zmienna `int` i `flp`, `int` będzie konwertowane do `flp`, aby zminimalizować utratę danych
	- tzn. np. zarówno `4.5 + 4`, jak i `4 + 4.5` da nam wynik 8.5
	- *zapisanie tego wyniku do zmiennej typu `int` będzie jednak skutkowało obcięciem miejsca dziesiętnego*
 - Konwersja w przypadku operacji zmiennych typu `str` ze zmiennymi innego typu zależy od działania:
	 - W przypadku konkatenacji wszystkie zmienne nie będące typu `str` zostaną na niego zamienione
		 - tzn. np. `"Ala ma " ~ 23 ~ .5 ~ "roku: " ~ true` da nam wynik `"Ala ma 23.5 roku: true"`
	- W przypadku operacji na danych liczbowych `str` będzie konwertowane na `int` lub `flp` w zależności od zapisu:
		- `"3" + 3 = 6`
		- `"3.0" + 3 = 6.`
		- `"a" + 3` zwróci błąd
- Zmienne typu `bool` będą tak naprawdę reprezentowane poprzez `int`:
```
true = 1
false = 0
```
- Konwersja z `bool` na `int` jest więc trywialna
	- Jeżeli jednak chcemy użyć `flp` lub `int` jako wartości logicznej, zmienne o wartości 0 będą konwertowane na `false`, a każda inna na `true`
	- Konwersja ze `str` na `bool` jest **zakazana**
#### Jawne
- Gdyby konwersje niejawne były niewystarczające/prowadziły do sytuacji nieprzewidzianych przez programistę, dostępna jest również jawna konwersja typów zmiennych
- Wykorzystuje ona mechanizm podobny do języka C - aby przekonwertować typ wartości, należy otoczyć go nawiasami i umieścić przed nimi typ docelowy
- Np:
```
int(3.5) -> 3
flp(3) -> 3.
str(543) -> "543"

543 as str
```
- *Uwaga: obowiązują wszystkie zakazy wymienione w sekcji konwersji niejawnych - konwersje nie mające logicznego sensu będą odrzucane*

---
## Operacje
### Operacje na danych liczbowych
- Język będzie dysponował podstawowymi operacjami matematycznymi na danych liczbowych:
	- dodawanie (+)
	- odejmowanie (-)
	- mnożenie (\*)
	- dzielenie (/)
	- potęgowanie (^)
	- modulo (%)
	- oraz połączenie wszystkich powyższych ze znakiem = w celu manipulacją zmienną "wsadowo":
		- +=, -=, \*=, /=, ^=, %=

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
	- mnożenie (\*)
		- powtarza wartość tekstową określoną ilość razy: `"ala " * 3 # = "ala ala ala "`
	-  `.capitalize() / .upper() / .lower()` - metody odpowiednio zmieniające wielkość liter
	- `.split()` - metoda inspirowana Pythonem. Wywołana na `string`'u zwraca tablicę stringów podzielonych według podanego znaku oddzielającego
		- przykładowo: 
```
'1i2i3'.split('i') -> ['1','2','3']
```
### Hierarchia Operatorów

| Pierwszeństwo | Operator                                                                    | Opis                                                                                |
| ------------- | --------------------------------------------------------------------------- | ----------------------------------------------------------------------------------- |
| **1**         | (wyrażenie)                                                                 | Nawias                                                                              |
| **2**         | typ(a)<br>a()<br>a[]<br>a!                                                  | Jawna konwersja typów<br>Wywołanie funkcji<br>Indeksowanie kolekcji<br>Moc kolekcji |
| **3**         | +a / -a<br>not a<br>a ^ b                                                   | Unarny plus/minus<br>Negacja (logiczna)<br>Potęgowanie                              |
| **4**         | a * b<br>a / b<br>a % b<br>a & b<br>a : b                                   | Mnożenie<br>Dzielenie<br>Modulo<br>Część wspólna kolekcji<br>Przecięcie kolekcji    |
| **5**         | a + b<br>a - b<br>a ~ b                                                     | Dodawanie<br>Odejmowanie<br>Konkatenacja                                            |
| **6**         | a < b<br>a > b<br>a <= b<br>a >= b                                          | Operatory relacyjne                                                                 |
| **7**         | a == b<br>a != b                                                            | Operatory porównania                                                                |
| **8**         | a and b                                                                     | Koniunkcja                                                                          |
| **9**         | a or b                                                                      | Alternatywa                                                                         |
| **10**        | a = b<br>a += b<br>a -= b<br>a *= b<br>a /= b<br>a ^= b<br>a %= b<br>a ~= b | Operatory przypisania                                                               |

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
```
- Zmienne będą przekazywane poprzez **referencję**, na co trzeba szczególnie uważać!
	- Dlatego też, póki co, nie przewiduję żadnego jawnego mechanizmu wskaźników lub referencji
- Literały będą przekazywane przez kopię
	- Ograniczenie przekazywania literałów do przekazywania przez referencję wprowadziłoby spory problem implementacyjny. Jeżeli literał zostanie przekazany przez zwykłą referencję, zniknie po wejściu w funkcję. Jeżeli z kolei zmusimy użytkownika do przekazywania literałów w postaci parametrów `const`, wprowadza to komplikacje w pisaniu uniwersalnych funkcji 
- Wynik będzie przekazywany przez **kopię**
- Po wywołaniu `return` funkcja zwraca odpowiednie wyrażenie i kończy swoje działanie

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
```
- Zaznaczyć należy, że wartości zmiennych używanych do inicjalizacji (oraz dodawanych po inicjalizacji) będą **kopiowane**
	- Takie rozwiązanie pozwala na uniknięcie problemów związanych z wiszącymi referencjami, które występowały by np. w następującym kawałku kodu:
```
int main()
{
	arr str test
	{
		str elem = "asdf"
		test = [elem]	
	}
	return 0
}
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
- Konkatenacja (~) - skleja dwie kolekcje
```
arr int a = [1, 2] ~ [3, 4]
```

- Różnica kolekcji (-) - usuwa każde wystąpienie podkolekcji z kolekcji
```
arr int a = [1, 2, 1, 2, 3] - [1, 2]
# a = [3]
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
	- w przypadku wykorzystania funkcji, jej typ wynikowy musi odpowiadać typowi kolekcji, oraz musi przyjmować jeden argument odpowiadający elementowi kolekcji
```
arr int a = [3, 4, 5]
arr int b = a[*3]    # b = [9, 12, 15]
arr int c = a[>4][*2] # c = [10]
arr int d = a[test_fun(_)]

int test_fun(int el)
{
	return el * 7 / 3
}
```

- Przecięcie kolekcji (:) - wycina wszystkie elementy od wybranego indeksu i zwraca je jako kolekcja
```
arr int a = [1, 2, 3, 4]
arr int b = a : 2        # a = [1, 2]; b = [3, 4]
```
### Metody
- `.insert(elem, idx)` - wstawia element `elem` do kolekcji w indeks `idx` (domyślnie na koniec)
```
arr int a = [1, 3, 4]
a.insert(5)     # a = [1, 3, 4, 5]
a.insert(2, 1)  # a = [1, 2, 3, 4, 5]
```
- `.remove(idx)` - usuwa element z danego indeksu `idx` i go zwraca
	- domyślnie zostanie usunięty ostatni element
```
arr int a = [1, 2, 3]
int removed = a.remove() # usunięta zostaje 3 i przypisana do removed
a.remove(0) # a = [2]
```

---
## Obsługa błędów
### Typy komunikatów + ich obsługa
#### 1. Błędy "odzyskiwalne"
- Niepożądane sytuacje, które mogą wystąpić w trakcie działania programu, z którymi program "wie" jak sobie poradzić
- np. dzielenie przez zero, użycie indeksu tablicy wykraczającego poza zakres, pętla nieskończona (przekroczenie limitu iteracji)
- W przypadku wykryciu takiego błędu program zatrzyma się i da programiście wybór odnośnie sposobu poradzenia sobie z problemem 
#### 2. Błędy krytyczne (panic)
- Błędy, w których stan programu jest naruszony i dalsza praca grozi np. wyciekiem danych lub błędy składniowe, typów (type mismatch) itd.
- Każdy błąd, który nie został zaklasyfikowany do błędów odzyskiwalnych
#### 3. Ostrzeżenie
- W sytuacjach, które są teoretycznie poprawne, ale mogą w przyszłości doprowadzić do błędu, zostanie wyświetlone ostrzeżenie
- Przewiduję wprowadzenie tylko kilku ostrzeżeń w ramach dowodu koncepcji
### Przykładowe komunikaty o błędach
#### Błąd "odzyskiwalny"
```
[PAUSE] Recoverable Error: Collection Index Out of Bounds
File: file.ds, Line: 42
Statement: int a = liczby[5]
--------------------------------------------------
Details: Attempted to access index [5]. 
Current size of collection 'liczby' is 3 (indices 0-2).

Choose an action to resume execution:
 [1] Return the default value for the target type (0 for int) and continue.
 [2] Skip this statement (variable 'a' remains uninitialized).
 [3] Throw a critical error (PANIC) and terminate the program.
> _
```
#### Błąd krytyczny
```
[PANIC] Critical Error: Type Mismatch in Arithmetic Operation
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
If this is intentional, use explicit conversion: int(4.9 + 2) to silence this warning.
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

# Nie można zamieniać str na bool - w zamian za to:

str s = "1"
bool b = int(s)  # b = true

# Niejawne konwersje w kolekcjach

arr int liczby = [1, 2, "3.5", true] 
# "3.5" -> 3
# true -> 1
# liczby = [1, 2, 3, 1]

# Przypadki brzegowe operacji na kolekcjach

arr int pusta = []
int x = pusta.remove()  # Błąd odzyskiwalny
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

---

## GRAMATYKA

### Znaki terminalne i literały

```EBNF
letter     = [A-Za-z];
digit      = [0-9];
underscore = "_";
newline    = "\n";

escape_sequence = "\\" , ( "\"" | "'" | "\\" | "n" | "t" | "r" ) ;

character_except_single_quote = [^']
character_except_double_quote = [^"]

bool_lit   = "true" | "false" ;
int_lit    = [ "+" | "-" ] , digit , { [ "'" ] , digit} ;

(* flp może być w 3 formach: 3.14, 3., .14 *)
flp_lit    = [ "+" | "-" ] , (
				( digit, 
					{ [ "'" ] , digit , "." , { [ "'" ] , digit } )
				| ( "." , digit , { [ "'" ] , digit } ) 
				| ( digit , { [ "'" ] , digit } , "." ) 
             ) ;

str_lit    = ('"' , 
				{ character_except_double_quote | escape_sequence } ,
				'"' )
		   | ( "'" , 
				{ character_except_single_quote | escape_sequence } ,
			    "'" ) ;
				
arr_lit    = "[" , [ expression, { "," , expression } ] , "]" ;

```

### Typy i zmienne

```EBNF
base_type  = "int" | "flp" | "str" | "bool" ;
type       = base_type | ( "arr" , base_type ) ;
identifier = letter , { letter | digit | underscore } ;

var_decl   = [ "const" ] , type, identifier , [ "=" , expression ] ;
```

### Wyrażenia

```EBNF
expression  = logical_or , [ assign_op , expression ] ;
assign_op   = "="
			| "+="
			| "-="
			| "*="
			| "/="
			| "^="
			| "%="
			| "~=" ;

logical_or  = logical_and , { "or" , logical_and } ;
logical_and = equality , { "and" , equality } ;
equality    = relational , [ ( "==" | "!=" ) , relational ] ; 
relational  = additive , [ ( "<" | ">" | "<=" | ">=" ) , additive ] ;
additive    = multipl , { ( "+" | "-" | "~" ) , multipl } ;
multipl     = unary_power , { ( "*" | "/" | "%" | "&" | ":" ) , 
				unary_power } ;

unary_power = ( ( "+" | "-" | "not" ) , unary_power )
			| ( postfix , { "^" , unary_power } ) ;

(*DO ZMIANY*)
postfix     = primary , {
		      ( "(" , [ arg_list ] , ")" )	
			| ( "[" , expression , "]" )
			| "!"
			| ( "." , identifier , "(" , [ arg_list ] , ")" )
			} ;
			
primary     = identifier
			| int_lit
			| flp_lit
			| str_lit
			| bool_lit
			| arr_lit
			| ( "(" , expression , ")" ) ;

arg_list    = expression , { ",", expression } ;
```

### Instrukcje, zakresy, funkcje

```EBNF
scope       = "{" , newline , { statement } , "}" ;

if_stmt     = "if" , "(" , expression , ")" , scope , 
				[ "else" , scope ] ;
while_stmt  = "while" , "(" , expression , ")" , scope ;

func_params = [ "const" ] , type , identifier , { "," , [ "const" ] , type , identifier } ;
func_decl   = [ "const" ] , type , identifier , 
				"(" , [ func_params ] , ")" , scope ;

ret_stmt    = "return" , [ expression ] ;

statement   = ( var_decl | expression | ret_stmt ) , newline
			| if_stmt , newline
			| while_stmt , newline
			| func_decl , newline
			| scope , newline ;	

program = { statement } ;
```