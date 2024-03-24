// ReSharper disable CppDependentTypeWithoutTypenameKeyword
// ReSharper disable CppLocalVariableMayBeConst
// ReSharper disable CppParameterMayBeConst
// ReSharper disable All
#include <array>
#include <concepts>
#include <iostream>
#include <list>
#include <vector>

namespace N1
{
    struct MyContainer
    {
        int begin() const;
    };

    int MyContainer::begin() const
    {
        volatile int i = 5;
        return i;
    }

    template <typename T>
    concept IsInt = std::same_as<T, int>;

    // template <typename T>
    // auto getBegin(T& c) // (bad 1)
    // {
    //     T::iterator it = c.begin();
    //     return it;
    // }

    template <typename T>
    T::iterator getBegin(T& c) // (1)
    {
        return c.begin();
    }

    template <typename T>
    auto getBegin(const T& c)
    {
        return c.begin();
    }

    template <typename T>
    struct TemplatedStruct
    {
        T::iterator nonTemplateFunc(int);
        template <typename U>
        T::iterator func(U u);
    };

    template <typename T>
    typename T::iterator TemplatedStruct<T>::nonTemplateFunc(int)
    {}

    template <typename T>
    template <typename U>
    typename T::iterator TemplatedStruct<T>::func(U u)
    {}
} // namespace N1

namespace N2
{
    void func(int);
    void func(double);
    void func(int, int);
} // namespace N2

namespace N3
{
    // Lors de la compilation, ce code
    // ne génère pas d'assembleur.
    template <typename T>
    T func(T t)
    {
        return t + 1;
    }

    // Code généré lors de la compilation
    // et qui sera présent dans le binaire.
    int func(int t)
    {
        return t + 1;
    }

    void main()
    {
        // Instancie la fonction
        // func(int).
        func(5);
    }
} // namespace N3

namespace N4
{
    template <typename T>
    std::enable_if<std::is_integral<T>::value, int>::type func(T t)
    {
        return static_cast<int>(t + 1);
    }

    template <typename T>
    requires std::integral<T>
    int func(T t)
    {
        return static_cast<int>(t + 1);
    }

    template <typename T>
    struct is_integral
    {
        bool value; // T est-il un entier ?
    };

    template <bool valid, typename Ret>
    struct enable_if
    {
        Ret type; // Uniquement présent si valid == true.
    };

    void func(auto val)
    {
        (void) val;
    }
} // namespace N4

namespace N5
{
    template <typename T>
    struct TemplatedStruct
    {
        void func(T::iterator it)
        requires std::same_as<decltype(it), int*>
        {
            (void) it;
        }
    };

    template <typename T>
    void func(T val)
    {
        (void) val;
    }
} // namespace N5

namespace N6
{
    int someFunc();

    template <std::integral I>
    void func(I integral, std::incrementable auto val)
    {
        std::same_as<int> auto anInt = someFunc();
    }

    template <typename T>
    void func(T val)
    {
        (void) val;
    }
} // namespace N6

namespace N7
{
    template <typename T>
    requires std::integral<T>
    void func(T val);

    template <typename T>
    requires std::integral<T>
    void func(T val)
    {
        (void) val;
    }
} // namespace N7

namespace N7_5
{
    struct WithTSize2
    {
        using type = int;
        std::array<char, 2> a;
    };
    struct WithoutTSize4
    {
        std::array<char, 4> a;
    };
    struct WithTSize4
    {
        using type = int;
        std::array<char, 4> a;
    };
    struct WithoutTSize2
    {
        std::array<char, 4> a;
    };

    template <typename T>
    requires(sizeof(T) == 2)
    T::type templateFunc()
    {}
    template <typename T>
    struct S
    {
        void nonTemplate(T::type t)
        requires(sizeof(t) == 2)
        {}
    };

    template <typename U>
    void templateFunc() {}

    void kdfjhkfgsjhgljdhgkj()
    {
        // templateFunc est appelé.
        templateFunc<WithTSize2>();    // (f1)
        // La contrainte de templateFunc n'est pas satisfaite,
        // la fonction n'est pas dans l'overload set.
        templateFunc<WithTSize4>();    // (f2)
        // La fonction trigger SFINAE, la fonction n'est pas
        // dans l'overload set.
        templateFunc<WithoutTSize2>(); // (f3)
        // La fonction trigger SFINAE, la fonction n'est pas
        // dans l'overload set.
        templateFunc<WithoutTSize4>(); // (f4)

        // S est instancié avec nonTemplate.
        S<WithTSize2> s1;    // (s1)
        // S est instancié sans nonTemplate.
        S<WithTSize4> s2;    // (s2)
        // S n'est pas instancié,
        // erreur de compilation.
        //S<WithoutTSize2> s3; // (s3)
        // S n'est pas instancié,
        // erreur de compilation.
        //S<WithoutTSize4> s4; // (s4)
    }
} // namespace N7_5

namespace N8
{
    int func(std::incrementable auto val)
    {
        return val.size();
    }
} // namespace N8

namespace N8_bis
{
    template <typename T>
    requires std::incrementable<T>
    int func(T val)
    {}

    template <typename T>
    int func(T val)
    requires std::incrementable<T>
    {}
} // namespace N8_bis

namespace N9
{
    struct Something
    {
        std::string size()
        {
            return "oui";
        };
    };

    auto func(auto val, int)
    requires requires {
        {
            val.size()
        } -> std::integral;
    }
    {
        std::cout << "constrained" << std::endl;
        return val.size();
    }

    void func(auto val, double)
    {
        std::cout << "not constrained" << std::endl;
    }
} // namespace N9

namespace N10
{
    template <typename T>
    requires(sizeof(T) == 2)
    void func(T val)
    requires std::is_same<T, int>::value
    {
        (void) val;
    }

    template <typename T>
    concept MyConcept = (T::value or std::integral<T>) and sizeof(T) == 1;

    namespace SubN10
    {
        template <typename T>
        concept MyConcept =
        ((std::same_as<typename T::value, bool> and T::value) or std::integral<T>) and sizeof(T) == 1;
    }

    struct FalseValue
    {
        static constexpr bool value = false;
    };

    struct TrueValue
    {
        static constexpr bool value = true;
    };

    struct NoValue
    {};

    struct IntValue
    {
        static constexpr int value = 5;
    };

    template <typename T>
    void printValue(T t)
    {
        if constexpr (requires { t.value; })
        {
            std::cout << "Value of t: " << t.value << std::endl;
        }
    }

    void lsqdkfhboldifqghlieshf()
    {
        // False car T::value vaut false et T n'est pas integral.
        MyConcept<FalseValue>; // (1)
        // True.
        MyConcept<TrueValue>;  // (2)
        // False car T::value trigger SFINAE et
        // donc vaut false, et T n'est pas integral.
        MyConcept<NoValue>;    // (3)
        // False car sizeof(T> != 1.
        MyConcept<int>;        // (4)
        // True.
        MyConcept<char>;       // (5)
        // Erreur de compilation car T::value n'est pas un bool.
        MyConcept<IntValue>;   // (6)
    }
} // namespace N10

namespace N11
{
    template <typename T>
    concept MyConcept = requires {
        T::value; // Requirement valide, est satisfait
        // si T::value est un nom qui existe.
        // std::string{}.invalid(); // Requirement invalide,
        // car ne respecte pas SFINAE.
    };
} // namespace N11

namespace N12
{
    struct Value
    {
        static constexpr bool value = true;
    };

    struct Type
    {
        using value = int;
    };

    template <typename T>
    concept MyConcept = requires { typename T::value; };

    template <typename T>
    concept MyBadConcept = requires {
        typename T::value; // Requirement valide,
        // est satisfait si T::value
        // est un type.
        // typename T::object.func(); // Requirement invalide,
        // l'expression ne peut pas
        // être un type.
    };

    void qslmkfjlskdghlksdyghidh()
    {
        // False car T::value trigger SFINAE et vaut donc false.
        MyConcept<int>;   // (1)
        // False car T::value est un nom de variable.
        MyConcept<Value>; // (2)
        // True.
        MyConcept<Type>;  // (3)
    }
} // namespace N12

namespace N13
{
    template <typename T>
    concept MyConcept = requires {
        {
            T::func()
        } noexcept -> std::same_as<int>;
    };
} // namespace N13

namespace N14
{
    template <typename T>
    concept MyConcept = requires {
        requires sizeof(T) == 2 and std::same_as<typename T::type, int>;
        requires T::value; // T::value doit être un booléen, sinon le concept ne compilera
        // pas, comme pour les requires-clauses.
        std::same_as<typename T::type, int>; // ATTENTION: c'est un requirement simple, il ne vérifie
        // pas que le concept est satisfait, il vérifie seulement
        // que l'expression est valide (mais elle peut valoir false).
    };

    template <typename T>
    void dpmokamlkjfglkdjg()
    {
        // T::type x;          // Le compilateur ne peut pas savoir ce que "T::type" est, une variable ? un type ?
        typename T::type y; // Il faut ajouter "typename" pour lui dire que "T::type" est bien un type.
    }
} // namespace N14

namespace N15
{
    struct AbstractStruct
    {
        virtual void nonConstFunc() = 0;
        virtual void constFunc() const = 0;
    };

    template <typename T>
    concept MyConcept = requires(T isntConst, const T isConst) {
        isntConst.nonConstFunc();
        isConst.constFunc();
    };

    void sdfsdgfgfhfg()
    {
        MyConcept<AbstractStruct>; // Valide et est satisfait.
    }
} // namespace N15

namespace N16
{
    template <typename T>
    void searchInVec(const T& val)
    {
        std::vector<std::string> myVec;
        std::find(myVec.begin(), myVec.end(), val);
    }

    template <typename T>
    requires std::equality_comparable_with<T, std::string>
    void searchInVec(const T& val)
    {
        std::vector<std::string> myVec;
        std::find(myVec.begin(), myVec.end(), val);
    }

    void sdkjfhsdkjfhkjh()
    {
        // On cherche un int dans un conteneur
        // de string. Ce n'est pas possible et le
        // compilateur nous le dit.
        // searchInVec(5);
    }
} // namespace N16

namespace N17
{
    template <typename T>
    constexpr bool IsABigInt = std::is_integral<T>::value and sizeof(T) > 4;

    template <typename T>
    std::enable_if<IsABigInt<T>>::type takeBigInt(T t)
    {
        (void) t;
    }

    void sldkfjsdljflksdjf()
    {
        // takeBigInt(5); // Un int mais pas suffisament grand.
    }
} // namespace N17

namespace N18
{
    template <typename T>
    concept IsABigInt = std::integral<T> and sizeof(T) > 4;

    template <typename T>
    requires IsABigInt<T>
    void takeBigInt(T t)
    {
        (void) t;
    }

    void sldkfjsdljflksdjf()
    {
        // Un int mais pas suffisament grand.
        // takeBigInt(5);
    }
} // namespace N18

namespace N19
{
    template <size_t val>
    concept IsABigSizeT = val > 5;

    void pqojdfldfqjgg()
    {
        IsABigSizeT<2>;  // Vaut false.
        IsABigSizeT<10>; // Vaut true.
    }

    template <typename T>
    requires requires { sizeof(T) == 2; }
    //  ^        ^ requires expression
    //  | requires clause
    void function()
    {}

    template <typename T>
    void remove(const T& item)
    {
        typename std::list<T>::iterator it;
    }
} // namespace N19

namespace N20
{
    template <typename T>
    requires true
    int printEtc(T, double)
    {
        return 1;
    }

    template <typename T>
    int printEtc(T, int)
    {
        return 2;
    }

    template <typename T>
    int printEtc(T, double)
    {
        return 3;
    }

    void sdlghdslqgjldkfghj()
    {
        printEtc(nullptr, 5);   // Retourne 2.
        printEtc(nullptr, 5.5); // Retourne 1.
    }
} // namespace N20

namespace N21
{
    template <typename T>
    concept IsInt = std::integral<T>;

    template <typename T>
    concept IsBig = sizeof(T) > 4;

    template <typename T>
    concept IsIntAndBig = IsInt<T> and IsBig<T>;

    template <typename T>
    concept IsIntOrBig = IsInt<T> or IsBig<T>;

    int printEtc(IsInt auto)
    {
        return 1;
    }

    int printEtc(IsBig auto)
    {
        return 2;
    }

    int printEtc(IsIntAndBig auto)
    {
        return 3;
    }

    int printEtc(IsIntOrBig auto)
    {
        return 4;
    }

    int printEtc(double);

    void pojkljqhfslcnvb()
    {
        // Satisfait IsInt et IsIntOrBig.
        // IsInt étant plus contraignant, retounrne 1.
        printEtc(5);   // A
        // Satisfait tous les concepts. IsIntAndBig
        // étant le plus contraignant, retourne 3.
        printEtc(5ll); // B
        // Erreur de compilation car ne satisfait
        // aucun concepts.
        printEtc(5.f); // C
        // Satisfait IsBig et IsIntOrBig.
        // IsBig étant plus contraignant, retounrne 2.
        printEtc(5.l); // D
    }
} // namespace N21

namespace N22
{
    template <typename T>
    concept IsInt = std::integral<T>;
    //              ^ contrainte 1

    template <typename T>
    concept IsBig = sizeof(T) > 4;
    //              ^ contrainte 2

    template <typename T>
    concept IsIntAndBigGood = IsInt<T> and IsBig<T>;
    // Après décomposition en contraintes atomiques,
    // le compilateur voit que ce concept vaut "1 ET 2".

    template <typename T>
    concept IsIntAndBigBad = IsInt<T> and sizeof(T) > 4;
    //                                    ^ contrainte 3
    // Après décomposition en contraintes atomiques,
    // le compilateur voit que ce concept vaut "1 ET 3".
} // namespace N22

namespace N23
{
    template <typename T>
    concept HasFunc = requires(T& t) { t.func(); };

    struct IsNotFriend
    {
        template <typename T>
        static bool paramHasFunc(const T&)
        {
            return HasFunc<T>;
        }
    };

    struct IsFriend
    {
        template <typename T>
        static bool paramHasFunc(const T&)
        {
            return HasFunc<T>;
        }
    };

    class WithPrivateFunc
    {
    private:
        void func();

        friend IsFriend;
    };

    void sdpofgjlkxjdfhkfjszhedfkjshdf()
    {
        // Les deux vaudront false.
        IsNotFriend::paramHasFunc(WithPrivateFunc{});
        IsFriend::paramHasFunc(WithPrivateFunc{});

        // Les deux vaudront true.
        IsFriend::paramHasFunc(WithPrivateFunc{});
        IsNotFriend::paramHasFunc(WithPrivateFunc{});
    }
} // namespace N23

namespace N24
{
    template <size_t N>
    void printFuzzBizz()
    {
        std::cout << N << std::endl;
    }
} // namespace N24

int main()
{
    using namespace N1;

    std::vector<int> v;
    getBegin(v); // (1, 2)

    MyContainer c;
    getBegin(c); // (2)

    // TemplatedStruct<int> ts; // error

    std::cout << "Hello, World!" << std::endl;

    std::cout << std::integral<int> << std::endl;

    if constexpr (std::integral<int>)
    {
        std::cout << "Int est un entier" << std::endl;
    }

    N9::func(std::string{}, 5);
    N9::func(N9::Something{}, 5);

    return 0;
}
