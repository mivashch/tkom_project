## Opis

- Dynamiczne, słabe typowanie
- Domyślna mutowalność
- Semantyka przez wartość\

Tworzony język jest imperatywnym o składni inspirowanej językami Python, C++ i Rust.
Obsługuje deklaracje zmiennych, instrukcje warunkowe if, pętle for, funkcje z argumentami, wyrażenia arytmetyczne i logiczne oraz typowanie statyczne.
Język wspiera również rekurencję i posiada stałe (const).

## Typy danych i operatory

Typy podstawowe: int, float, str, bool, fun \
Operatory:

- Arytmetyczne: + - \* / %
- Porównania: == != < <= > >=
- Logiczne: && ||
- Specjalne (może uliegać zmianie): &\*&, =>>

Język używa typowania dynamicznego (typ określany jest w czasie wykonania) oraz słabego typowania, co oznacza, że możliwe są automatyczne konwersje typów w kontekście wyrażeń.

Przykłady automatycznych konwersji:

- int + float → float
- str + int → str (konkatenacja)
- bool w kontekście arytmetycznym → int (true = 1, false = 0)

### Przykład:

```
a = 10;
b = 3.5;
s = "Hello world";
flag = true;

c = a + 2 * 5;          // c = 20
ok = (a > 5) && flag;   // ok = true

result1 = a + b          // result1 = 13.5
result2 = s + a          // result2 = "Hello world10"
result3 = a + flag       // result3 = 11

```

## Zmienne i stałe

Zmienną można zadeklarować bez określenia typu – typ jest nadawany dynamicznie.
Zmienna może w późniejszym czasie przyjmować wartość innego typu (słabe typowanie).
Stała (const) nie może być ponownie przypisana.

### Przykład:

```
x = 5;         // x ma typ int
x = "Hello";   // teraz x ma typ str

const y = 10;
y = 20;        // Błąd semantyczny – stałej nie można zmieniać

```

## Instrukcje warunkowe

Instrukcja `if` pozwala wykonywać różne bloki kodu w zależności od warunku.
Można użyć opcjonalnego else.

### Przykład:

```
int x = 10;

if (x > 0) {
    print("positive");
} else {
    print("negative");
}
```

## Pętle for

Instrukcja for ma formę zbliżoną do C/C++:\
`for (inicjalizacja; warunek; aktualizacja) { ... }`

### Przykład:

```
for (int i = 0; i < 5; i = i + 1) {
    print(i);
}
```

## Funkcje

Deklaracja funkcji rozpoczyna się słowem fun.
Można określić typ zwracany, listę parametrów oraz instrukcję return, oraz wspierane wywolania lańcuhowe.

### Przykład:

```
fun int add(int a, int b) {
    return a + b;
}

int result = add(2, 3);
#=====================================#
fun int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

int f = factorial(5); // wynik: 120
#====================================#
fun fun get_func() {
    return inc;
}

fun int inc(int x) {
    return x + 1;
}

get_func()(7);

```

## Wyrażenia i priorytety operatorów

Kolejność wykonywania operatorów:

1. Wywołania funkcji i nawiasy
2. Unary -
3. \* / %
4. \+ -
5. && ||
6. &\*& i =>>

### Przykład:

```
int r = 2 + 3 * 4;   // wynik: 14
int x = (2 + 3) * 4; // wynik: 20
```

## Łańcuchy i znaki specjalne

Łańcuch znaków zapisujemy w "...", obsługiwane są sekwencje \n, \t, \\\\.

### Przyklad:

```
str s = "Hello\nWorld\t!";
```

## Komentarze

Do pisania komentarzy uzywane symboli // i /\*.

```
// komentarz jednoliniowy

/*
   komentarz wieloliniowy
*/
```

# Obsługa błędów

Interpreter języka wykrywa błędy składniowe, typów oraz semantyczne.
Każdy komunikat zawiera rodzaj błędu, numer linii i krótkie wyjaśnienie.
Interpreter zatrzymuje wykonanie przy pierwszym błędzie składniowym.Błędy semantyczne są wykrywane podczas interpretacji.

### Przykłady komunikatów:

| Rodzaj błędu                                | Kod źródłowy                                          | Komunikat                                                                |
| ------------------------------------------- | ----------------------------------------------------- | ------------------------------------------------------------------------ |
| Błąd składniowy (Syntax Error)              | `int a = ;`                                           | `Error [line 1]: Expected expression after '='.`                         |
| Nieznana zmienna (Name Error)               | `inc(a);`                                             | `Error [line 1]: Undefined variable 'a'.`                                |
| Ponowne przypisanie do stałej (Const Error) | `const int x = 5; x = 10;`                            | `Error [line 2]: Cannot assign to constant 'x'.`                         |
| Brak wartości zwracanej (Return Error)      | `fun int f() { }`                                     | `Error [line 1]: Missing return statement for function 'f' of type int.` |
| Niezgodność liczby argumentów (Call Error)  | `fun int add(int x, int y) { return x + y; } add(1);` | `Error [line 3]: Function 'add' expects 2 arguments, got 1.`             |
| Błąd składniowy w nawiasach                 | `if (x > 0 { print("ok"); }`                          | `Error [line 1]: Missing ')'.`                                           |

# Sposób uruchomienia Wejście / Wyjście

Wejście: \
plik tekstowy z rozszerzeniem .mi, zawierający kod źródłowy programu,\
lub standardowe wejście (stdin) w trybie interaktywnym.\
Wyjście:\
standardowy strumień wyjścia (stdout),\
komunikaty o błędach kierowane są do stderr.

### Przykład

```
tkom program.mi
================
tkom
>>> int x = 5;
>>> print(x + 2);
7
```

# Struktura

## Analizator leksykalny

Odpowiada za przekształcenie kodu źródłowego w listę tokenów.
Rozróżniane są następujące tokeny:

- słowa kluczowe: `fun, return, if, for, const`
- operatory: `+, -, *, /, %, =, ==, !=, <, >, <=, >=, &&, ||,&*&,=>>`
- nawiasy i klamry: `(, ), {, }, ,, ;`
- identyfikatory i literały: `IDENTIFIER, NUMBER, STRING, BOOLEAN`

Lexer wykorzystuje prosty automat stanowy i zwraca listę struktur:

```
 Token { type: string, value: string, line: int, column: int }
```

Będzie realizowana leniwa tokenizacja.

## Analizator składniowy

Na podstawie tokenów buduje drzewo AST zgodne z gramatyką EBNF.
Każdy węzeł drzewa odpowiada elementowi języka:\
`ExpressionNode, StatementNode, IfNode, ForNode, FunctionNode, CallNode etc.`

### Przykład drzewa:

```
FunctionNode {
  name: "get_func",
  params: [],
  body: [
    ReturnNode(
      value = FunctionLiteral(params=["x"], body=[
        ReturnNode(BinaryOp("+", Variable("x"), Literal(1)))
      ])
    )
  ]
}
```

## Interpreter

Interpreter przechodzi po drzewie AST i wykonuje instrukcje zgodnie z typami węzłów.
Wykorzystuje kontekst w postaci słownika:

```
Environment {
  variables: { name → value },
  functions: { name → definition }
}
```

Dzięki dynamicznemu i słabemu typowaniu interpreter przechowuje wszystkie wartości jako obiekty typu:

```
Value {
  type: "int" | "float" | "str" | "bool" | "function",
  value: any
}
```

Przy operacjach takich jak + lub % interpreter automatycznie konwertuje typy.

## Moduł błędów

Każdy moduł może zgłaszać wyjątki typu:

```
Error { typ: "SyntaxError" | "RuntimeError" | "TypeError", description: string, row: int column: int }
```

# Testowanie

## Co warto przetestować

Mam na celu weryfikację poprawności działania wszystkich elementów języka:

- analizy leksykalnej i składniowej (czy parser akceptuje poprawny kod)
- poprawności interpretacji wyrażeń arytmetycznych i logicznych
- działania instrukcji sterujących (if, for)
- poprawności przekazywania i zwracania wartości w funkcjach (w tym funkcje wyższego rzędu)
- obsługi błędów wykonania
- automatycznej konwersji typów w słabym typowaniu

## Metoda testowania

Interpreter był testowany w trybach:

- skryptowym — każdy test stanowi osobny plik .src, zawierający fragment programu w języku.
- Jednostkowym - badanię osobnych modulów i funkcję.
- Akceptacyjne - badanie dzialań konstrukcji języka.
- Testy błędów - Weryfikują poprawność komunikatów o błędach
