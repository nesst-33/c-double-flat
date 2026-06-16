# SPRAWOZDANIE

## Cel projektu
Celem projektu jest implementacja języka programowania ogólnego przeznaczenia. Język programowania ma spełniać następujące wymagania:
- Zmienne mają być statycznie typowane, słabe oraz domyślnie mutowalne
- Zmienne mają być przekazywane do funkcji przez referencje
- Zaimplementowany ma być specjalny typ złożony - kolekcje

## Główne założenia
- Program składa się tylko i wyłącznie z instrukcji, wyrażenia nie mogą występować nigdzie indziej niż w ich kontekście
- Zakładając taką architekturę uznałem, że działania w wyrażeniach mogą chwilowo prowadzić do "niepoprawnych" typów. Jednak jako, że wyrażenia zawsze występują w kontekście instrukcji, gdzie konkretny typ jest wymagany, to przy ich wykonywaniu zostanie dokonane sprawdzanie i rzutowanie typu
- Przykładem takiego podejścia może być:

```c
# Literał zawiera wartości o różnych typach
# Najpierw zostaje stworzona tablica o tych wartościach,
# które są następnie rzutowane na odpowiedni typ
arr int a = ["12.5", 12, true, 5.4]
```

- Dlatego też, same literały oraz wyniki działań nie zawierają żadnej informacji o typie - niosą tylko czystą wartość, która jest następnie dostosowana do odpowiedniego kontekstu (można powiedzieć, że podobnie jak w TypeScript)

## Zasady działania języka
### Nazywanie zmiennych
- Identyfikatory zmiennych i funkcji muszą zaczynać się znakiem ASCII. Każdy następny znak może być znakiem ASCII, cyfrą lub znakiem podłogi (\_)
	- np. dozwolona jest nazwa `nazwa_zm123`, ale `3nazwa` już nie

### Niemutowalność
- Niemutowalność oznacza się poprzez kwalifikator `const`
- W przypadku przypisania wartości const do zmiennej non-const, dokonana zostaje głęboka kopia (ważne dla zachowania "const-correctness" dla tablic)
```c
const arr arr int a = [[1, 2], [3, 4]]
arr int b = a[0] # zostaje dokonana głęboka kopia
b[0] = 7 # to działanie nie zmienia tablicy 'a'
```

### Inicjalizacja
- Prymitywy nie muszą być inicjalizowane przy deklaracji. W przypadku braku jawnej inicjalizacji, zostaną im przypisane następujące wartości:
	- `int`: 0
	- `flp`: 0.0
	- `str`: ""
	- `bool`: false
- Zmienne tablicowe muszą być jawnie zainicjalizowane

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

#### `arr`
- Kolekcja `arr` to typ danych, który będzie przechowywał określoną liczbę danych pewnego typu
- Kolekcje będą *homogeniczne* oraz *uporządkowane*
- Mogą zmieniać swoją wielkość dynamicznie
- Definiowane w następujący sposób:
```
arr <typ> nazwa_zmiennej = [elem1, elem2, ...]
```

## Wyrażenia

### Operacje liczbowe
#### Operatory arytmetyczne (+, -, /, \*, %)
- Poniższa tabela opisuje, do jakiego typu zostaną zrzutowane argumenty działań z ww. operatorami (gdzie wiersze to typy pierwszego argumentu, a kolumny drugiego):

|          | int         | flp | str         | bool        |
| -------- | ----------- | --- | ----------- | ----------- |
| **int**  | int         | flp | int/flp(\*) | int         |
| **flp**  | flp         | flp | flp         | flp         |
| **str**  | int/flp(\*) | flp | int/flp(\*) | int/flp(\*) |
| **bool** | int         | flp | int         | int         |

- Na ogół staramy się zachować jak największą ilość danych w działaniu:
	- Z tego powodu faworyzujemy typ `flp` - tzn. zarówno `4.5 + 4`, jak i `4 + 4.5` da nam wynik `8.5`
	- *zapisanie tego wyniku do zmiennej typu `int` będzie jednak skutkowało obcięciem miejsca dziesiętnego*
- Rzutowanie `str` jest szczególnym przypadkiem
	- typ, do którego rzutujemy `str` zależy od jego wartości
	- jeżeli wartość tekstowa reprezentuje liczbę całkowitą (np. `"434"`), to rzutujemy na `int`; jeżeli zmiennoprzecinkową (np. `".143"`), to rzutujemy na `flp`
	- (\*) Aby `str` był rzutowany na wartość liczbową, musi zawierać tylko cyfry, separator dziesiętny, oraz ewentualnie apostrof
		- `"43", "43.4", ".14"` to przykłady `str` rzutowalnych
		- próba rzutowania np. `"a"` na `int` zwróci błąd
- Konwersje `bool <-> int/flp` będą zachodziły w następujący sposób:
```
true -> 1
false -> 0

0 -> false
wszystko inne -> true
```

#### Modulo
- Wykorzystuje funkcję do wykonywania modulo zmiennoprzecinkowych dostępną w bibliotece C (`std::fmod`), dlatego też wynik wyrażenia jest zawsze wyrażony w postaci `flp`
### Operacje porównania
- W wyniku zwracają `true` lub `false` (bool)

#### Mniejszość/większość (`<, <=, >, >=`)
- Porównywać można tylko typy liczbowe i tekstowe
- Zakazane jest porównywanie tablic
- Zmienne tekstowe porównujemy leksykograficznie
- W przypadku porównywania zmiennej liczbowej i tekstowej podejmujemy próbę niejawnego rzutowania `str` na `int` lub `flp` (zgodnie z zasadami używanymi w operacjach liczbowych)

#### Równość\nierówność (`==, !=`)
- Porównywać można wszystkie typy co powyżej plus tablice i bool
- Tablice można porównywać tylko ze sobą (nie będą rzutowane na prymitywy)
- Porównywanie tablic o różnych typach skutkuje rzutowaniem (takim samym jak w operacjach większości/mniejszości)

### Operacje na zbiorach
#### Różnica kolekcji (`-`)
- Usuwa każde wystąpienie podkolekcji z kolekcji
- Obowiązuje tylko dla tablic - `str` będzie rzutowane na liczbę (aby wykonać zwykłe odejmowanie)
```
arr int a = [1, 2, 1, 2, 3] - [1, 2]
# a = [3]
```

#### Mnożenie kolekcji (`*`)
- powtarza zbiór określoną ilość razy
- zasady dla `str` takie same
```
arr int a = [1, 2]
arr int b = a * 3   # b = [1, 2, 1, 2, 1, 2]
```

#### Moc zbioru (`!`)
- Operator może być wywoływany tylko na tablicach i zmiennych tekstowych

#### Indeksowanie (`[]`)
- Indeksować można tablice oraz `str`
- `int`/`double` są rzutowane na `str` w celu indeksowania
- Indeks musi być typu `int`
```c
123[0] # daje "1"
```

#### Łączenie zbiorów (`~`)
- Łączyć można tablice i `str`
- Jeżeli przynajmniej jeden czynnik operacji jest prymitywem, oba zostają rzutowane na `str`

#### Ogon (`:`)
- Zwraca elementy od danego indeksu włącznie
	- indeks ponownie musi być typu `int`
- Działa na tablicach lub `str` (inne prymitywy zostają rzutowane na `str`, podobnie jak w przypadku indeksowania)

#### Koniunkcja (`&`)
- Zwraca część wspólną zbiorów
- Zasady działania dokładnie te same co w przypadku ww. operacji
```
arr int a = [1, 2, 3] & [2, 3, 4] # a = [2, 3]
```

#### Dodawanie elementu (`<<`)
- Służy do dodawania elementu do tablicy
- Operacja zwraca nową tablicę (nie modyfikuje oryginalnej tablicy)
- Dodawać elementy można tylko do tablic (do typów tekstowych należy wykorzystać konkatenację)

#### Ekstrakcja elementu (`>>`)
- Usuwa element z tablicy o danym indeksie i go zwraca
- Operacja działa tylko i wyłącznie na tablicach

#### Mapowanie względem funkcji
- Aby mapować funkcję na każdy element tablicy wystarczy skorzystać z następującej składni:

```c
int mult(int a, int b) {
	return a * b
}

arr int test = [1, 2, 3]
print(test[mult(_, 3)]) # Mnoży wszystkie elementy przez 3
```

### Operacje logiczne (`and`, `or`, `not`)
- Operacji logicznych nie można wykonywać na tablicach
- Typy podstawowe będą przekształcane na `bool`:
	- `flp -> bool`
		- `0. -> false`
		- `wszystko inne -> true`
	- `int -> bool`
		- `0 -> false`
		- `wszystko inne -> true`
	- `str -> bool`
		- `"" -> false`
		- `wszystko inne -> true`

### Jawne konwersje typów
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
"asdf" as int
"def" as flp
```
- Rzutować można również tablice:
```
arr flp a = [1.5, 3.5]
arr str b = a as int
```
## Instrukcje (statements)
### Przypisywanie wraz z działaniem
- Dostępne są poniższe kombinacje operatorów z przypisywaniem:
	- +=
	- -=
	- \*=
	- /=
	- %=
	- ~=
### Instrukcje warunkowe/pętli
- W celu uproszczenia implementacji projektu, okroję dostępne instrukcje warunkowe/pętli dostępne w innych popularnych językach do absolutnego minimum (choć nie wykluczam ich rozszerzenia w przyszłych iteracjach projektu)
- Przewiduję:
	- instrukcje if/else
	- pętlę while
- Będą one działać podobnie do innych popularnych języków programowania:
```
if (condition) {
	...
} else {
	...
}

while (condition) {
	...
}
```

### Definiowanie i wołanie funkcji
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
- Funkcje deklarować można tylko w zakresie globalnym
- Zmienne będą przekazywane poprzez **referencję**
	- Niejawne rzutowanie w przypadku referencji jest zabronione
	- R-Value będą rzutowane niejawnie
- Po wywołaniu `return` funkcja zwraca odpowiednie wyrażenie i kończy swoje działanie
- Przeciążanie funkcji jest zakazane (język jest słabo typowany - to rozwiązanie powinno wystarczyć)

### Przypisywanie do tablic
- W przypadku przypisywania literału tablicy, prymitywy w nim zawarte zostaną przekształcone na odpowiedni typ (przykład na początku dokumentu)
- Jeżeli głębokość zagnieżdżenia tablic w literale nie odpowiada deklarowanemu typowi, zostanie zwrócony błąd
```c
arr arr int c = [1, 2, 3] # BŁĄD
arr arr int d = [[1,2], 3] # również błąd
```

- W ramach literału można również jako elementy umieszczać nazwy zmiennych, w tym tablicowych (jeśli ich głębokość zagnieżdżenia jest odpowiednia)
	- Wartości zmiennych w literałach są przekazywane przez płytką kopię (w większości przypadków)
	- Jeżeli przekażemy do literału zmienną tablicową o bazowym typie nie odpowiadającym typowi deklarowanej zmiennej, wartość zostanie przekazana przez głęboką kopię (każdy element musi zostać rzutowany)
	- W przeciwnym przypadku, przekażemy wskaźnik do tablicy zawartej w zmiennej. Oznacza to, że zmiana elementów w tablicy przekazanej do literału, będzie również zmieniać elementy w nowej zmiennej
```c
const arr int a = [1, 2, 3]
arr int b = a # dokonana jest głęboka kopia
```
- Nie można modyfikować zmiennych tekstowych w tablicach za pomocą indeksowania
```c
arr str a = ["asdf"]
a[0][0] = "a" # błąd
```

### Modyfikowanie `str`
- Zmienne tekstowe można modyfikować za pomocą indeksowania podobnie jak tablice (należy tylko uważać na to, aby przypisywać tylko jedną literę)
```c
str a = "asdf"
a[0] = "abc" # BŁĄD
```

## Hierarchia Operatorów

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

## Komunikaty o błędach
### Warning
- Ostrzeżenie
```
WARNING: Missing terminating newline
 --> integration_tests/lexer_error.cb (line: 1, column: 11)
  |
1 | int a = 34l
  |           ^
  |
```
### Lexical Error
- Błąd leksera
```
Lexical Error: Stray backslash (no newline after backslash)
 --> integration_tests/lexer_error.cb (line: 1, column: 11)
  |
1 | int a = 34 \
  |            ^
  |
```
### Syntax Error
- Błąd parsera
```
Syntax Error: Invalid statement
 --> integration_tests/global_return.cb (line: 1, column: 1)
  |
1 | return 3 + 4
  | ^
  |
```
### Runtime Error
- Błąd interpretera
```
Runtime Error: Array was indexed too many times; max depth is 1
 --> integration_tests/str_modification_test.cb (line: 7, column: 6)
  |
7 | a[0][0] = "b"
  |      ^
  |
```

## Przykłady

```c
arr arr int b = ["3"]

arr arr int test = [[.5, "21"], [true]]
print(test)

int a = 4.5
arr arr int b = [[3.2]]

# b should be deeply copied into c, because the types diverge
arr arr arr flp c = [b, [[a]]]
print("c array before changing b: ")
print(c)
print("")

b[0] = [5]
print("c array after changing b: ")
print(c)
print("")

print("--------------")

arr int d = [1, 2]
arr arr int e = [d, [3, 4]]
print("e array before changing d: ")
print(e)
print("")

d[0] = 3
print("e array after changing d: ")
print(e)
print("")

d = [3, 4]
print("e array after assigning a new array literal to d: ")
print(e)
print("")

print("--------------")

arr arr int f = [[1,2], [3,4]]
arr arr int g = [f[0], [5,6]]

print("g array before changing f: ")
print(g)
print("")

f[0][0] = 7

print("g array after changing f: ")
print(g)
print("")

f[0] = [7, 8]
print("g array after assigning a new nested array to f: ")
print(g)

flp a = 3.5
print(a)

a += 4
print(a)

a -= "3.5"
print(a)

a *= 2
print(a)

a /= 2
print(a)

# This shouldn't change anything (we're adding past the precision point)
a ~= "34"
print(a)

a %= 3
print(a)

int b = 2137
b ~= "85.3"
print(b)

print("Now there should be an error: ")
b = "2a1b3c7a"


void func(int b) {

    }

print("This should throw an error: ")
const int a = 4
func(a)

const arr arr int a = [[1,2], [3,4]]
print("This should throw an error: ")
a[0][0] = 3

const arr arr int a = [[1, 2], [3,4]]
print("a before modifying b: " ~ a)
arr int b = a[0]
b[0] = 7

print("a after modifying b: " ~ a)
print("")

# Now i'll try to see if the deep copy in b can be stored by ref
print("b before modifying c: " ~ b)
arr int c = b
c[0] = 2137
print("b after modifying c: " ~ b)
print("")

const arr int arrTest(const arr arr int a) {
    return a[0]
}

b = arrTest(a)
b[0] = 7
print("a after passing it through a function: " ~ a)

int test(const int a) {
        a = 4
    }

print("This should throw an error: ")
test(4)

const int a = 4
print("This should throw an error: ")
a -= 3

int a
print(a)

flp b
print(b)

str c
print(c)

bool d
print(d)

print("This should throw an error:")
arr arr int e

# This should throw an error
int func() {
}

func()

int mult(int a, int b) {
    return a * b
}

arr int nestedMult(arr int a, int b) {
    return a[mult(_, b)] 
}

arr int test1 = [1, 2, 3]
print("test1 before function mapping:\t" ~ test1)
print("test1 with function mapped:\t" ~ test1[mult(_, 4)])

arr arr int nested = [[1, 2], [3, 4]]
print("Nested function map:\t\t" ~ nested[nestedMult(_, 3)])

# This should throw an error (there's a type mismatch)
print(nested[mult(_, 3)])

{
    return
}

print(123)

int a = 3
void glob(int n) {
    int i 
    while (i < n) {
        a += 1
        i += 1
    }
}

glob(10)
print(a)

int a = 34 \ 


{
    void test() {

    }
}

str a() {
    return "ads"
}

int a = 4

int a = 4
{
    int a = 5
    print(a)
}
print(a)

int func(arr int a)
{
    return 3
}

arr arr int b = [[3, 2]]
func(b)


int func(int a) {
        int a = 4
        return 3
    }

func(4)

void func(int a) {
    a = 10        
}

int a = 3
print("a before calling func: " ~ a)
func(a)
print("a after calling func: " ~ a)

void arrFunc(arr int a) {
    a = [3, 2, 1]
}

arr int b = [1, 2, 3]
print("")
print("b before calling arrFunc: " ~ b)
arrFunc(b)
print("b after calling arrFunc: " ~ b)

print("")
print("Now there should be an error:")
void badFunc(arr int c) {
    c = 4
}
badFunc(b)


void func(int a) {

    }

flp b = 3.5
func(b)

int a = 1'000'000
print(a)

str b = "asdf"
print("b before modification: " ~ b)
b[0] = "z"
print("b after modification: " ~ b)

print("This should throw an error: ")
b[0] = "abcd"

void func() {
        return 3 + 2
    }

func()

```