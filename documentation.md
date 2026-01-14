# Kartka
## Dokumentacja projektowa
### (zaktualizowany projekt wstępny / dokumentacja końcowa)

---

## Wprowadzenie

**Język został nazwany „Kartka” wyłącznie po to, aby na pytanie „Na czym piszesz?” móc odpowiedzieć: „Na kartce”.**

Kartka jest interpretowanym językiem programowania ogólnego przeznaczenia,projekt obejmuje pełen cykl tworzenia języka programowania:
od zaprojektowania składni i semantyki, poprzez implementację analizatora
leksykalnego i składniowego, aż po wykonujący program interpreter.

Celem projektu było zaprojektowanie języka, który:

- posiada dynamiczne i słabe typowanie
- obsługuje funkcje wyższego rzędu
- wspiera tryb interaktywny
- wykrywa i raportuje błędy składniowe oraz semantyczne
- jest w pełni testowalny za pomocą testów jednostkowych

Kartka nie aspiruje do bycia językiem produkcyjnym — jego celem jest
demonstracja mechanizmów spotykanych w rzeczywistych językach programowania
(C++, Python, JavaScript).

---

## Opis użytkowy języka

Kartka jest językiem:

- **imperatywnym** — program składa się z instrukcji wykonywanych sekwencyjnie,
- **dynamicznie typowanym** — typy zmiennych ustalane są w czasie wykonania,
- **słabo typowanym** — możliwe są automatyczne konwersje typów,
- **interpretowanym** — kod wykonywany jest bezpośrednio przez interpreter,
- **z funkcjami jako wartościami** — funkcje mogą być przekazywane i zwracane.

Język umożliwia:

- deklarowanie i przypisywanie zmiennych,
- definiowanie stałych (`const`),
- definiowanie i wywoływanie funkcji,
- rekurencję,
- instrukcje sterujące (`if`, `for`),
- wyrażenia arytmetyczne i logiczne,
- tworzenie i używanie krotek (tuple),
- częściowe wiązanie argumentów funkcji,
- dekorowanie funkcji innymi funkcjami,
- pracę w trybie interaktywnym (REPL).

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
a = 10;                 //dynamiczne typowanie
b = 3.5;                //dynamiczne typowanie
s = "Hello world";      //dynamiczne typowanie
flag = true;            //dynamiczne typowanie

c = a + 2 * 5;          // c = 20
ok = (a > 5) && flag;   // ok = true

result1 = a + b          // result1 = 13.5
result2 = s + a          // result2 = "Hello world10"
result3 = a + flag       // result3 = 11

```
### Implementacja

W implementacji interpretera wszystkie wartości reprezentowane są jako:
```
using Value = std::variant<
std::monostate,
long long,
double,
std::string,
bool,
std::shared_ptr<Function>,
std::shared_ptr<TupleValue>
>;
```

Pozwala to na bezpieczne i wydajne przechowywanie dowolnej wartości
w jednym typie C++.

## Zmienne i stałe

Zmienną można zadeklarować bez określenia typu – typ jest nadawany dynamicznie.
Zmienna może w późniejszym czasie przyjmować wartość innego typu (dynamiczne typowanie).
Stała (const) nie może być ponownie przypisana.

Słabe typowanie pozwala na wykonanie operacje między typami, nawet jeśli to nie ma sensu matematycznego.
Zmienna w Kartce powstaje w momencie pierwszego przypisania:
```
x = 10;
y = x + 5;
```

Nie jest wymagane wcześniejsze deklarowanie zmiennej.
### Stałe (const)
Zmienne oznaczone jako const nie mogą być ponownie przypisane.
```
Przykład:

const x = 10;
x = 20;   // błąd semantyczny
```

Interpreter wykrywa próbę zmiany stałej i zgłasza błąd wykonania.

### Zakres zmiennych

Kartka stosuje zasięg leksykalny (lexical scope)\
każdy blok ({ ... }) tworzy nowe środowisko,\
funkcje tworzą własne środowisko,\
możliwe jest odwoływanie się do zmiennych z zewnętrznych zakresów.


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
x = 10;

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
Można określić typ zwracany(Typ zwracany ma charakter informacyjny i nie jest statycznie sprawdzany.), listę parametrów oraz instrukcję return, oraz wspierane wywolania lańcuhowe.

### Funkcje jako wartości

Jedną z kluczowych cech Kartki jest pełne wsparcie dla funkcji wyższego rzędu. Oznacza to, że funkcje mogą być:

- przekazywane jako argumenty do innych funkcji,
- zwracane jako wartości,
- przechowywane w zmiennych,
- modyfikowane przez inne funkcje.

### Przykład:

```
fun int add(a: int, b) {
    return a + b;
}

int result = add(2, 3);
#=====================================#
fun int factorial(n) {
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

fun int inc(x: int) {
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
### Operator dekoracji funkcji &*&
Operator &*& (operator dekoracji) umożliwia tworzenie nowej funkcji
poprzez „opakowanie” jednej funkcji inną.

Formalnie:
```
decorated = base &*& decorator
```

gdzie:

base – funkcja bazowa\
decorator – funkcja przyjmująca jako pierwszy argument funkcję bazową,
a następnie jej argumenty.
### Przykład:

```
fun int ident(x) {
    return x;
}

fun int add1_decorator(f, x) {
    return f(x + 1);
}

decorated = ident &*& add1_decorator;
print(decorated(7));   // 8

```
### Operator wiązania argjmentów =>>
Idea operatora

Operator =>> umożliwia częściowe wiązanie argumentów funkcji.

`bound = (a, b) =>> f`\
oznacza:\
`bound(x) = f(a, b, x)`
### Przyklad:
```
fun int add(x, y, z) {
    return x + y + z;
}

add_5_7 = (5, 7) =>> add;
print(add_5_7(3));   // 15
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

### Rodzaje błędów
- ParseError – błędy składniowe,
- RuntimeError – błędy semantyczne i wykonania.

### Przykłady komunikatów:

| Rodzaj błędu                                | Kod źródłowy                                          | Komunikat                                                                |
| ------------------------------------------- | ----------------------------------------------------- | ------------------------------------------------------------------------ |
| Błąd składniowy (Syntax Error)              | `a = ;`                                           | `ParseError: ParseError [1:5]: Expected primary expression (got ';')`                         |
| Nieznana zmienna (Name Error)               | `inc(a);`                                             | `RuntimeError: Undefined variable: a`                                |
| Ponowne przypisanie do stałej (Const Error) | `const  x = 5; x = 10;`                            | `RuntimeError: Cannot assign to const variable 'x'`                         |
| Niezgodność liczby argumentów (Call Error)  | `fun  add( x,  y) { return x + y; } add(1);` | `RuntimeError: Wrong number of arguments: expected 2, got 1`             |
| Błąd składniowy w nawiasach                 | `if (x > 0 { print("ok"); }`                          | `ParseError: ParseError [1:11]: Expected token ) (got '{')`                                           |

# Sposób uruchomienia Wejście / Wyjście

Wejście: \
plik tekstowy zawierający kod źródłowy programu,\
lub standardowe wejście (stdin) w trybie interaktywnym.\
Wyjście:\
standardowy strumień wyjścia (stdout),\

### Tryb interaktywny

Kartka posiada tryb interaktywny umożliwiający:\
- wpisywanie kodu linia po linii,\
- natychmiastowe wykonanie,\
- kontynuację pracy po błędzie.\

### Przykład

```
Kartka interactive interpreter
Type :quit or Ctrl+D to exit
> 1;
1
> a = 2; b = a + "2";
4
> a = "2"; b = a + 2;
22
> a = ;
ParseError: ParseError [1:5]: Expected primary expression (got ';')
> a = 4;
4
> :quit

Process finished with exit code 0

```

### Sposób korzystania z interpretera:
Tryb plikowy:
`./interpreter program.txt` \
Tryb interaktywny: `./interpreter`

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
Parsing completed successfully!
Program:
  FuncDecl(int add(a:int, b))
    Block:
      Return:
        Binary('+')
          Identifier(a)
          Identifier(b)
  ExprStmt:
    Assign(result)
      Call:
        Callee:
          Identifier(add)
        Args:
          Literal(2)
          Literal(3)
  ExprStmt:
    Call:
      Callee:
        Identifier(print)
      Args:
        Identifier(result)
  FuncDecl(int factorial(n))
    Block:
      If:
        Cond:
          Binary('<=')
            Identifier(n)
            Literal(1)
        Then:
          Block:
            Return:
              Literal(1)
      Return:
        Binary('*')
          Identifier(n)
          Call:
            Callee:
              Identifier(factorial)
            Args:
              Binary('-')
                Identifier(n)
                Literal(1)
                .
                .
                .
                .
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

# Zwięzły opis implementacji
## Lexer

- leniwa tokenizacja
- obsługa komentarzy
- precyzyjna lokalizacja błedów.

## Parser
- parser rekurencyjny z priorytetami operatorów,
- AST jako struktura klas C++,
- precyzyjna lokalizacja błedów.

## Interpreter
- środowiska zagnieżdżone (Environment),
- funkcje jako obiekty (Function),
- obsługa ReturnSignal jako wyjątku sterującego.

# Testowanie

## Co warto przetestować

Miałem na celu weryfikację poprawności działania wszystkich elementów języka:

- analizy leksykalnej i składniowej (czy parser akceptuje poprawny kod)
- poprawności interpretacji wyrażeń arytmetycznych i logicznych
- działania instrukcji sterujących (if, for)
- poprawności przekazywania i zwracania wartości w funkcjach (w tym funkcje wyższego rzędu)
- obsługi błędów wykonania
- automatycznej konwersji typów w słabym typowaniu

## Metoda testowania

Interpreter był testowany w trybach:

- skryptowym —   test stanowi osobny plik , zawierający fragment programu w języku.
- Jednostkowym - badanię osobnych modulów i funkcję(GoogleTest, testy pozytywne,testy negatywne,testy brzegowetesty semantyczne.).
- Akceptacyjne - badanie dzialań konstrukcji języka.
- Testy błędów - Weryfikują poprawność komunikatów o błędach
