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

// In your code.
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

*\* It's a little bit more complicated than that, but we will discuss about it more in depth later.*  

```cpp
std::string str1, str2; // Only a statement.
str1;                   // 1
str1 += str2;           // 2
str1 + str2;            // 3
5;                      // 4
```

Here every whole line is a statement, but what's more interesting is the expression behind that statement.  
For **1** the expression is an "id-expression" (expression that only contains a name), and it result in the object `str1`.  
For **2**, we have a call to function that return a reference, so the result of the expression is an unnamed reference to the object `str1`.  
For **3**, we have a call to a function that return a object, so the result of the expression is an unnamed object.  
And lastly, for **4** we just have a value, nothing more\*.  

*\* In fact there is more, but again we will discuss about it more in depth later.*  

## 2. Values categories of expressions  

### 2.1. Overview  

Every expression have a value category, it determines how its result can be used and what operations are available on it.  

Historically the only value category were *lvalue* and *rvalue*, it's inherited from C. **L**values were expressions that could appear on the **L**eft hand side of the operator `=`, **R**values were expressions that could *only* appear on the **R**ight hand side of an operator `=`.  

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

### 2.2. Value categories  

Values categories have two properties that distinguish them. The first one is if they have an *identity*, it means that you can compare the result of two expressions to know if they are the same object, by comparing the addresses of these results. The second one is if they are *movable*, that means you can use the expression as a parameter for a move constructor, or anything that take a rvalue reference of the same type as the result of the expression.  

Glvalue is the group of expressions that have an identity, and because of this you can have another expression in another statement that will have the same result. The result object is not limited to the statement of the expression.  
Rvalue is the group of expressions that can be used as parameter for a move constructor, and because of this it means the value of their result will not be used anymore, it can be safely stolen in the move constructor.  

#### 2.2.1. Properties of lvalues  

They have an identity, you can use the unary operator `&` on it to know their address, but they aren't movable because you can't use them as parameter in a move constructor.  

```cpp
// Somewhere in a header.
OnlyMovableObject();
OnlyMovableObject(const OnlyMovableObject&) = delete;
OnlyMovableObject(OnlyMovableObject&&);

// In your code.
OnlyMovableObject obj;
OnlyMovableObject& ref = obj;

&(obj) == &(ref);                  // 1: Valid code.
OnlyMovableObject otherObj{(obj)}; // 2: Invalid code.
OnlyMovableObject otherObj{(ref)}; // 3: Invalid code.
```

*I've put parenthesis around variables names to be able to differentiate the id-expression (expression that only contain a name) and the variable in itself).*  
 
As you can see in **1**, you can compare the addresses of `(obj)` and `(ref)`, so they have both an identity, and as you can see in **2** and **3**, you can't use them to move-construct an object, so they aren't movable. So `(obj)` and `(ref)` are both lvalues.  

You can think of lvalues as expressions where the result need to be fully usable after the end of the statement. That means you can't steal (move) the data it contains and that there should be a name that make the object accessible again. So basically it's a named object / reference (I will detail how to identify lvalues later).  

#### 2.2.2. Properties of xvalues  

They have an identity, you can't use the unary operator `&` directly on it but you should be able to access it from elsewhere. They are movable because you can use them as parameter in a move constructor.  

```cpp
// Somewhere in a header.
OnlyMovableObject();
OnlyMovableObject(const OnlyMovableObject&) = delete;
OnlyMovableObject(OnlyMovableObject&&);

// In your code.
OnlyMovableObject obj;

&(std::move(obj));                            // 1: Invalid code.
&(obj);                                       // 2: Valid code.
OnlyMovableObject otherObj{(std::move(obj))}; // 3: Valid code.
```

The first thing to notice here in **1** is that you can't take the address of `(std::move(obj))`, but because you know it results in a rvalue reference to `obj` then you can take the address of `(obj)` and it will be the same, so the expression has an identity. Then in **3** you can see that you can use the expression as a parameter to a move constructor, so it's movable.  

You can think of xvalues as expression that results in an expiring object (like in the name). That means the value of the object will not be needed after the end of the statement (you can steal it), but the object itself will still be accessible and used, that's why it need to be kept in a valid state. So basically it's a unnamed rvalue reference (I will detail how to identify xvalues later).  

#### 2.2.3. Properties of prvalues  

They don't have an identity, you can't use the unary operator `&` in it nor you can do it from elsewhere, but they can be used as a parameter of a move constructor.  

```cpp
// Somewhere in a header.
OnlyMovableObject();
OnlyMovableObject(const OnlyMovableObject&) = delete;
OnlyMovableObject(OnlyMovableObject&&);

// In your code.
&(OnlyMovableObject{});                            // 1: Invalid code.
OnlyMovableObject otherObj{(OnlyMovableObject{})}; // 2: Valid code.
```

As you can see in **1**, it's impossible to get the address of a prvalue because in fact its result is not an object, it's just a value, so it's not stored in the memory. Then in **2**, we can see that it's possible to use a prvalue to move construct an object, so it's movable, in fact the move constructor won't even be called here, because copy elision will happen (like we've seen earlier). A prvalue is only available in the expression that created it, so in the next statement it won't be accessible by any mean.  

I know it may be unclear why the result of a prvalues isn't an object, but just a value, because in fact in the above example what I create is what we commonly call an unnamed object. So for now, just remember that in C++17 unnamed objects aren't objects, but values, and that values aren't in the memory but are used to create objects that will be in the memory, we will see how it really work in the in-depth part about copy elision.  

#### 2.2.4. Identifying value categories  

OK, I've said a lot of "we will see later", now time has come to see some of them. We will see what kind of expressions are lvalues, xvalues, and prvalues, and why. Let's start with a reminder:  

|                     |glvalue|rvalue|lvalue|xvalue|prvalue|
|--------------------:|:-----:|:----:|:----:|:----:|:-----:|
|  **has an identity**|      ✓|      |     ✓|     ✓|       |
|**can be moved from**|       |     ✓|      |     ✓|      ✓|

Now let's identify some expressions.  

```cpp
std::string obj;
std::string& objLRef = obj;
std::string& lRefFunc();
std::string&& objRRef = std::move(obj);
std::pair<int, int> pairObj;

(obj);             // 1
(objLRef);         // 2
(lRefFunc());      // 3
(objRRef);         // 4
(pairObj.first);   // 5
```

All of the numbered lines contains lvalues expressions, we've already seen why for **1** and **2** so I will just talk about the others.  
For **3**, we have a call to a function that return an unnamed lvalue reference, it's like a named reference it has the same properties, that's why it's an lvalue.  
For **4** it's a little less obvious because we have an rvalue reference, but rvalue references just means that the reference can only be bound to an rvalue, after that you have a name that refer to your object, so its value shouldn't be stolen by someone.  
For **5**, we have a sub-object of an lvalue, it follow the same principles as the others, so the expression is an lvalue.  

To summary, expressions that result in named objects / rvalue references or lvalue references (named or not) are lvalues, as well as sub-objects of these expressions.  

```cpp
std::string obj;
std::string&& rRefFunc();

(std::move(obj));                // 1
(rRefFunc());                    // 2
(std::pair<int, int>{}.first);   // 3
(std::move(obj).first);          // 4
```

Here, all the numbered lines contains xvalues expressions, we've already seen why **1** is an xvalue, and it fact it's the same for **2** (they are both functions that return an rvalue reference). Let's see for the others.  
So, **3** isn't an lvalue because we can move construct an object from it, why ? Because its value won't be available in the next statement, the object is expiring. Now why it's not a prvalue ? Because we can know its address: inside the destructor of `std::pair` we will be able to do `&first`. It can be moved from and it has an identity, so it's an xvalue.  
For **4** it's the same as **3**, we said that the `std::pair` is expiring (as well as its sub-objects) but we will still be able to access its address from anywhere in the scope (because the `std::pair` is just an xvalue this time). So that why it's also an xvalue.  

Now you may ask *"if we can know the address of `first` inside the destructor of `std::pair`, we can know the address of the `std::pair` as well, so why it's a prvalue ?"*. We will see that in the next chapter.  

To summary, expressions that result in an unnamed rvalue reference or in a sub-object of an rvalue (static data members aren't sub-objects) are xvalues.  

```cpp
std::string objFunc();

(std::string{});                // 1
(objFunc());                    // 2
```

This one will be short, we've already seen why **1** is a prvalue, and it's the same thing for **2**: in both case we have a function that return an unnamed object, in **1** because we call the constructor on an unnamed object, in **2** because we just call a function that return an object (so it can't have a name). And because we've seen that unnamed objects are just values, and that prvalues result in values, then they are prvalue.  

To summary, expressions that result in an unnamed object (value) are prvalues.  

## 3. The subtleties of prvalues  

### 3.1. Temporary materialization  

```cpp
class PrintOwnAddr
{
public:
    void printAddr() { std::cout << this << std::endl; }
};

(PrintOwnAddr{}).printAddr(); // Print something like "0x7ffe973cfb6f".
```

How does it work ? How does the prvalue can have access to its address if it's suppose to be a value not it the memory, so without an address ? It's because of *temporary materialization*.  

When an object is needed for computation, but the expression used is just a prvalue, then a temporary object is materialized, effectively converting the prvalue to an xvalue.  
In the example above we try to access a member of a prvalue, this will materialize a temporary object, allowing us to do so.  

Here some examples of things that materialize a temporary object:  

```cpp
void function(const std::pair&);

(std::pair<int, int>{}).first;        // 1
function((std::pair<int, int>{}));    // 2
(std::pair<int, int>{});              // 3
```

We've already seen why **1** materialize a temporary object, for **2** it's not really different, a reference cannot be bound to a value, it needs an object, that's why a temporary object is materialized.  
For the last one, **3**, it's a little bit more interesting, we do nothing with the prvalue, so why a temporary object should be materialized ? Because it will be strange to not have a constructor and a destructor called for something like this. It's called a *discarded expression*, it's when the result of the expression isn't used for anything, in this case a temporary object will be materialized only to be able to call the constructor and destructor.  

### 3.2. Guaranteed copy elision: What the standard says  

And finally, the part that will explain how prvalues actually work, what's really behind them.  

Because it's easier to understand with an example:  

```cpp
VerboseClass iBuildStuff()
{
           // 1
    return VerboseClass{};
}

int main()
{                // 3      // 2
    VerboseClass imStuff = iBuildStuff();
    return 0;
}
```

*`VerboseClass` is a class that print a message when one of its constructor is called.*  

What happens when copy elision is disabled (only possible with C++14):  

```
// flags "-fno-elide-constructors --std=c++14"
VerboseClass default constructor.
VerboseClass move constructor.
VerboseClass move constructor.
```

And what happens when copy elision isn't disabled:  

```
// flags "--std=c++14"
VerboseClass default constructor.
```

Why ? Because of prvalues.  
When copy elision is disabled, in **1** we create an rvalue, in **2** the rvalue is moved to the return slot of the function, and in **3** it's used for the move constructor of `imStuff`. But copy elision is never disabled in practice, and it's not a simple rvalue: it's a prvalue. Because you can see prvalues as just values, without any object in memory, then the move of the value from **1** to **2** and **2** to **3** is free: there isn't any object to move, the object in **3** is directly constructed from the value in **1**.  

To have a copy elision, you just need to have an object initialized from a prvalue of the same type. If it's not of the same type, then a conversion will need to be done, effectively creating another object.  
If we take the example above but instead of returning a `VerboseClass` from the function we return something that can be used to construct a `VerboseClass`:  

```cpp
VerboseClass iBuildStuffFromParam()
{
           // 1
    return VerboseParameter{};
}

int main()
{                // 3      // 2
    VerboseClass imStuff = iBuildStuffFromParam();
    return 0;
}
```

*`VerboseParameter` is like `VerboseClass`, it's a class that print a message when one of its constructor is called, but it can be used to construct a `VerboseClass`.*  

```
// flags "--std=c++14"
VerboseParameter default constructor.
VerboseClass parameter constructor.
```

In **1** we create a prvalue that isn't of the same type as the return of the function, so no copy elision happen between **1** and **2**, just a conversion from `VerboseParameter` to `VerboseClass`, and then we have a copy elision between **2** and **3**.  

### 3.3. Guaranteed copy elision: How the compiler implement it  

There isn't any magic behind copy elision, the compiler can't just magically hold values somewhere without having something in the memory. What happens in fact is that the compiler knows where the temporary will be materialized at the end in advance, so when the prvalue is created an object is directly constructed in its final destination, let's see this with an example.  

```cpp
SuperVerboseClass iBuildStuff()
{
    SuperVerboseClass noisyStuff{123};
           // 1
    return SuperVerboseClass{1};
}

int main()
{
    SuperVerboseClass firstStuff{0};
                      // 3          // 2
    SuperVerboseClass secondStuff = iBuildStuff();
    SuperVerboseClass thirdStuff{2};
    return 0;
}
```

*`SuperVerboseClass` is a class that print a message when one of its constructor is called, showing its address and its id (constructor parameter).*  

What happens when copy elision is disabled (only possible with C++14):  

```
// flags "-fno-elide-constructors --std=c++14"
VerboseClass arg constructor. (0: 0x78)
VerboseClass arg constructor. (123: 0x40)
VerboseClass arg constructor. (1: 0x38)
VerboseClass move constructor. (1: 0x68)
VerboseClass move constructor. (1: 0x70)
VerboseClass arg constructor. (2: 0x60)
```

And what happens when copy elision isn't disabled:  

```
// flags "--std=c++14"
VerboseClass arg constructor. (0: 0x78)
VerboseClass arg constructor. (123: 0x40)
VerboseClass arg constructor. (1: 0x70)
VerboseClass arg constructor. (2: 0x68)
```

### TODO 3.4. What C++17 bring us  

## TODO 4. NRVO  

It's big.  
