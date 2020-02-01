
# Move semantics part 1

**Disclaimer**: What I will describe here is what the standard says for C++17, but it also apply to C++14 if the compiler support copy elision (and if it's not disabled), what every sane compiler should do.

## 1. Bases of move semantics

### 1.1. What does "moving" an object means ?

Moving an object `A` to an object `B` means that `B` will take the value of `A` and that `A` will be left in a valid but unspecified state.
Valid state means that you can still call functions of `A`, unspecified state means that you don't now what's the value of `A`. Mainly, what you will be able to do is call the assignment operator on it to have a specified state or call (indirectly) its destructor when the object will be out of scope.

```cpp
std::string otherStr{"stuff"};
std::string str{std::move(otherStr)};
```

Here, `str` now contains the value `"stuff"` and `otherStr` contains something unspecified, but for almost all implementations it will just contains an empty string.

And what does `std::move` do ? It's just a cast to an rvalue reference, so the two lines below are identical:

```cpp
std::string str;

std::move(str);
static_cast<std::string&&>(something);
```

The only purpose of this is to have the move constructor being chosen during the overload resolution of the constructor of the string.

```cpp
// Somewhere in <string> header.
string(const string&);  // 1
string(string&&);       // 2

// In your code
std::string str;

std::string str2{str}; // Copy constructor (1) is used.
std::string str3{std::move(str)}; // Move constructor (2) is used.
```

### 1.2. What is "copy elision" ?

Sometimes compilers can elide copies (and moves) to optimize the code. In C++17 some type of copy elision are mandatory, like the one below.

```cpp
std::pair object = std::pair{1, '2'};
```

|Without copy elision|With copy elision|
|--------------------|-----------------|
|The right object is constructed with values `1` and `'2'` (call `pair(int,char)`)|The left object is constructed with values `1` and `'2'` (call `pair(int,char)`)|
|The left object is move constructed from the right object (call `pair(pair&&)`)|Nothing more !|

### 1.3. What's an "expression" ?

A expression is an computation that may produce a result, if it produce nothing then it's a void expression.
Results of expressions are entities, these entities can be of different types, the ones that are important for the move semantics are:
- **Values**: Data not stored in the memory\*, used to construct objects.
- **Objects**: Data stored in the memory.
- **References**: Something that refer to an object (or a function).

*\* It's a little bit more complicated than that, but we will discuss about it more in depth during the part about prvalues.*

```cpp
std::string str1, str2; // Only a statement.
str1;                   // 1
str1 += str2;           // 2
str1 + str2;            // 3
5;                      // 4
```

Here every whole line is a statement, but what's more interesting is the expression behind that statement.
For **1** the expression is an "id-expression", and it result in the object `str1`.
For **2**, we have a call to function that return a reference, so the result of the expression is an unnamed reference to the object `str1`.
For **3**, we have a call to a function that return a object, so the result of the expression is an unnamed object.
And lastly, for **4** we just have a value, nothing more\*.

*\* In fact there is more, but again we will discuss about it more in depth later, during the part about prvalues.*

## 2. Values categories of expressions

### 2.1. Overview

Every expression have a value category, it determines how its result can be used and what operations are available on it.

Historically the only value category were **lvalue** and **rvalue**, it's inherited from C. **Lvalues** were expressions that could appear on the **L**eft hand side of the operator `=`, **rvalues** were expressions that could *only* appear on the **R**ight hand side of an operator `=`.

Since C++11 it's more complicated, here is a graph that shows the different values categories:
```
         expression
          |      |
          V      V
     glvalue    rvalue
     |     |    |    |
     V     V    V    V
lvalue     xvalue    prvalue
```

- **glvalue**: Generalized left value, can be either an lvalue or an xvalue.
- **rvalue**: Right value, can either be an xvalue or a prvalue.
- **lvalue**: Left value, sometimes call locator value because you can call the unary operator `&` on it. Basically it's for named objects.
- **xvalue**: Expiring value. Basically it's for temporary objects.
- **prvalue**: Pure right value. Basically it's just a value, so it's not an object.

We will go more in depth about each main value categories, but before that just some details about what I've said earlier: how value category change how an expression can be used.

The most important uses for the move semantics is how an expression can be used to initialize an object, there is the table:

|                |lvalue|xvalue|prvalue|
|---------------:|:----:|:----:|:-----:|
|        **copy**|     ✓|     ✓|      ✓|
|        **move**|      |     ✓|      ✓|
|**copy elision**|      |      |      ✓|

An expression can still be converted to another expression with a different value category, sometimes even implicitly.

### 2.2. Identifying value categories
