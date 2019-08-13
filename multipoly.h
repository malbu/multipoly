#ifndef MULTIPOLY_H
#define MULTIPOLY_H

#include <iostream>
#include <vector>
#include <memory>

template<int n, class T, class Alloc = std::allocator<T> >
class poly;
/*
  Represents a polynomial in n indeterminates with coefficients from T. This is implemented
  recursively: poly<n, T> is a polynomial in one indeterminate with coefficients in poly<n-1,
  T>. The last instance, poly<0, T>, is essentially equal to T.

  In fact, the complete polynomial type is poly<n, T, A>, where A is an allocator for T, for example
  std::allocator<T>; this is also the default parameter. Usually this parameter does not needs to be
  changed.

  Let us describe the interface:

  * Every polynomial has a predicate isZero(), testing whether the polynomial is the zero
    polynomial.

  * Moreover, it has basic arithmetic, i.e. addition, subtraction, multiplication; this is realized
    via operator overloading. Assignment and compare operators are also included.

  * Polynomials have a degree function degree(), returning the degree as a polynomial in the first
    indeterminate, as well as a function leading(), returning the leading term (which is of type
    const poly<n-1, T> &). Note that the degree of the zero polynomial is -1.

  * The i-th coefficient can be obtained by writing f[i] if f is of type poly<n, T>; then f[i] is of
    type poly<n - 1, T>. In case i is larger than the degree of f, a zero polynomial will be
    returned, and in case f is not constant, the internal space for the coefficients will be
    enlarged to have space for the i-th coefficient. Note that one should call f.normalize()
    afterwards if one accessed f[i] for non-const f to ensure that degree and leading coefficients
    will be computed correctly afterwards.

  * The polynomial can be evaluated using operator(). If f is of type poly<n, T, A>, then f(x) with
    x of type S will return an object of type poly_evaluator<n, T, A, S>. This object can be casted
    to poly<n-1, S> to obtain f with the first indeterminate evaluated as x. Note that S must be a
    type to which elements of T can be casted. One can also continue evaluating with the
    poly_evaluator<n, T, A, S> object, yielding poly_evaluator_impl<k, T, ..., A, S> objects, k <
    n. These object hierarchy ensures that polynomial evaluation is efficient, i.e. after (good
    enough) optimiziation of the compiler, is essentially a block of code of (n-k+1) nested
    for-loops.

  * Polynomials can be written to std::ostream's using operator<<. There is also a member function
    print(std::ostream &) const. Note that the first indeterminate in poly<n, T> is denoted by X_0,
    and the last by X_{n-1}.



  The coefficients are stored in a std::vector<>; while poly<1, T> uses std::vector<T>, poly<n, T>
  for n > 1 uses std::vector<poly<n-1, T> *>, i.e. pointers to coefficients are stored. This is
  implemented using the "intelligent" vector poly_ivector<> template.

  Creation can be accomplished using the functions Monomial<T, A>(e_0, ..., e_{n-1}), where A again
  is std::allocator<T> by default. This creates a polynomial of type poly<n, T, A> which consists of
  exactly one monomial X_0^{e_0} * ... * X_{n-1}^{e_{n-1}} having coefficient 1. One can create a
  polynomial for example using
      poly<2, int> f = Monomial<int>(1, 2) + 3 * Monomial<int>(4, 5);
  to obtain
      f = X_0 X_1^2 + 3 X_0^4 X_1^5.


 */



template<int n, class T, class Alloc = std::allocator<T>, class S = T>
// Helper template to evaluate a polynomial of type poly<n, T, Alloc> in a point of S. This template
// is returned from poly<n, T, Alloc>::operator().
//
// Note that evaluation is more complex, since we want that the compiler optimizes evaluation to a
// set of nested for-loops. The original implementation was done recursively, returning a polynomial
// of type poly<n-1, T> evaluated in the first indeterminate, and than the operator() of poly<n-1,
// T> was called to evaluate in the next indeterminate, etc. This proved to be very ineffective (not
// very surprisingly). If evaluation is seldomly done, this is ok, but in my case, it was the main
// bottleneck, whence I implemented this more complicated, but also much more efficient solution.
//
// ...
class poly_evaluator;

template<int n, class T, class HL, class Alloc, class S>
// Another helper for polynomial evaluation. This template is returned from poly_evaluator<n, T,
// Alloc>::operator().
//
// The template parameter HL is the type of the "owner" of this template, i.e. either
// poly_evaluator<n+1, T, Alloc> or poly_evaluator_impl<n+1, T, ..., Alloc>.
class poly_evaluator_impl;

template<class T, class HL, class Alloc, class S>
class poly_evaluator_impl<1, T, HL, Alloc, S>
{
    template<int nn, class TT, class AA, class SS>
    friend class poly_evaluator;

    template<int nn, class TT, class HLHL, class AA, class SS>
    friend class poly_evaluator_impl;

private:
    const HL & d_owner; // The "owner"
    const S & d_evalpoint; // The evaluation point on this level

    inline poly_evaluator_impl(const HL & owner, const S & evalpoint)
        : d_owner(owner), d_evalpoint(evalpoint)
    {
    }

    class eval_fun
    // Evaluates a polynomial of type poly<1, T> in d_evalpoint.
    {
        const poly_evaluator_impl<1, T, HL, Alloc, S> & d_owner;

    public:
        inline eval_fun(const poly_evaluator_impl<1, T, HL, Alloc, S> & owner)
            : d_owner(owner)
        {
        }

        inline S operator() (const poly<1, T, Alloc> & p) const
        // Evaluate
        {
            S res = (S)0;
            S xx = (S)1;
            for (int i = 0; i < (int)p.d_value.size(); ++i)
            {
                res += (S)(T)p.d_value[i] * xx;
                xx = xx * d_owner.d_evalpoint;
            }
            return res;
        }
    };

public:
    inline operator S() const
    // Cast to S
    {
        S res = (S)0;
        d_owner.evaluate(res, eval_fun(*this));
        return res;
    }

    inline S operator() () const
    // Explicit evaluate. Essentially calls operator S().
    {
        return (S)(*this);
    }
};

template<int n, class T, class HL, class Alloc, class S>
class poly_evaluator_impl
{
    template<int nn, class TT, class AA, class SS>
    friend class poly_evaluator;

    template<int nn, class TT, class HLHL, class AA, class SS>
    friend class poly_evaluator_impl;

private:
    const HL & d_owner; // The "owner"
    const S & d_evalpoint; // The evaluation point on this level

    inline poly_evaluator_impl(const HL & owner, const S & evalpoint)
        : d_owner(owner), d_evalpoint(evalpoint)
    {
    }

    template<class SS, class Fun>
    class eval_fun
    // Evaluates a polynomial of type poly<n, T> in d_evalpoint; the coefficients of the first
    // indeterminate are evaluated using the given functor of type Fun.
    {
        const poly_evaluator_impl<n, T, HL, Alloc, S> & d_owner;
        const Fun & d_evalfun;

    public:
        inline eval_fun(const poly_evaluator_impl<n, T, HL, Alloc, S> & owner, const Fun & evalfun)
            : d_owner(owner), d_evalfun(evalfun)
        {
        }

        inline SS operator() (const poly<n, T, Alloc> & p) const
        // Evaluate: the first indeterminate is replaced by d_evalpoint, and the coefficients of the
        // first indeterminate (which are polynomials of type poly<n-1, T>) are evaluated using
        // d_evalfun.
        {
            SS res = (SS)0;
            S xx = (S)1;
            for (int i = 0; i < (int)p.d_value.size(); ++i)
            {
                res += d_evalfun(p.d_value[i]) * (SS)xx;
                xx = xx * d_owner.d_evalpoint;
            }
            return res;
        }
    };

    template<class SS, class Fun>
    inline void evaluate(SS & res, const Fun & evalfun) const
    // This will be called from a child (i.e. a class of type poly_evaluator_impl<n-1, T,
    // poly_evaluator<n,T,HL,Alloc,S>, Alloc, S>) to trigger evaluation.
    {
        // We have to pass evaluation on to our owner, but give a new functor which now evaluates
        // polynomials of type poly<n, T>.
        d_owner.evaluate(res, eval_fun<SS, Fun>(*this, evalfun));
    }

    class eval_fun2
    // Evaluates a polynomial of type poly<n, T> in d_evalpoint; the coefficients of the first
    // indeterminate (which are polynomials of type poly<n-1, T>) are casted to poly<n-1, S>, and
    // the result is of type poly<n-1, S> as well.
    {
        const poly_evaluator_impl<n, T, HL, Alloc, S> & d_owner;

    public:
        inline eval_fun2(const poly_evaluator_impl<n, T, HL, Alloc, S> & owner)
            : d_owner(owner)
        {
        }

        inline poly<n - 1, S, typename Alloc::template rebind<S>::other> operator() (const poly<n, T, Alloc> & p) const
        // Evaluate: the first indeterminate is replaced by d_evalpoint, and the coefficients of the
        // first indeterminate (which are polynomials of type poly<n-1, T>) are casted to the type
        // poly<n-1, S>.
        {
            poly<n - 1, S, typename Alloc::template rebind<S>::other> res;
            S xx = (S)1;
            for (int i = 0; i < (int)p.d_value.size(); ++i)
            {
                res += ((poly<n - 1, S, typename Alloc::template rebind<S>::other>)p.d_value[i]) * xx;
                xx = xx * d_owner.d_evalpoint;
            }
            return res;
        }
    };

public:
    inline operator poly<n - 1, S, typename Alloc::template rebind<S>::other>() const
    // Allows casting to poly<n-1, S>.
    {
        poly<n - 1, S, typename Alloc::template rebind<S>::other> res; // missing: determine allocator object
        // We need to pass evaluation on to our owner
        d_owner.evaluate(res, eval_fun2(*this));
        return res;
    }

    template<class SS>
    inline poly_evaluator_impl<n - 1, T, poly_evaluator_impl<n, T, HL, Alloc, S>, Alloc, SS> operator() (const SS & x) const
    // Continues evaluation with the next indeterminant.
    {
        return poly_evaluator_impl<n - 1, T, poly_evaluator_impl<n, T, HL, Alloc, S>, Alloc, SS>(*this, x);
    }
};

template<class T, class Alloc, class S>
class poly_evaluator<1, T, Alloc, S>
// The top level polynomial evaluation class, in case n = 1, i.e. does direct evaluation.
{
    friend class poly<1, T, Alloc>;

private:
    const poly<1, T, Alloc> & d_poly; // The polynomial in question
    const S & d_evalpoint; // The evaluation point

    inline poly_evaluator(const poly<1, T, Alloc> & poly, const S & evalpoint)
        : d_poly(poly), d_evalpoint(evalpoint)
    {
    }

public:
    inline operator S() const
    // Casting to S is done by evaluation in d_evalpoint.
    {
        S res = (S)0;
        S xx = (S)1;
        for (int i = 0; i < (int)d_poly.d_value.size(); ++i)
        {
            res += ((S)(T)d_poly.d_value[i]) * xx;
            xx = xx * d_evalpoint;
        }
        return res;
    }

    inline S operator() () const
    // Evaluates and returns value in S.
    {
        return (S)(*this);
    }
};

template<int n, class T, class Alloc, class S>
class poly_evaluator
// The top level polynomial evaluation class, in case n > 1, i.e. in case the coefficients are
// polynomials by themselves.
{
    friend class poly<n, T, Alloc>;

    template<int nn, class TT, class HLHL, class AA, class SS>
    friend class poly_evaluator_impl;

private:
    const poly<n, T, Alloc> & d_poly; // The polynomial in question
    const S & d_evalpoint; // the evaluation point

    inline poly_evaluator(const poly<n, T, Alloc> & poly, const S & evalpoint)
        : d_poly(poly), d_evalpoint(evalpoint)
    {
    }

    template<class SS, class Fun>
    inline void evaluate(SS & res, const Fun & evalfun) const
    // Will be called by "child". Evaluates the polynomial into an element of SS (which must not
    // necessarily be S, it can also be poly<k, S> for some k < n - 1) using the given functor to
    // evaluate the coefficients, which are of type poly<n-1, T>.
    {
        S xx = (S)1;
        for (int i = 0; i < (int)d_poly.d_value.size(); ++i)
        {
            res += evalfun(d_poly.d_value[i]) * xx;
            xx = xx * d_evalpoint;
        }
    }

public:
    inline operator poly<n - 1, S, typename Alloc::template rebind<S>::other >() const
    // Evaluate to polynomial of type poly<n-1, S>.
    {
        poly<n - 1, S, typename Alloc::template rebind<S>::other> res(d_poly.get_allocator());
        S xx = (S)1;
        for (int i = 0; i < (int)d_poly.d_value.size(); ++i)
        {
            res += poly<n - 1, S, typename Alloc::template rebind<S>::other>(d_poly.d_value[i]) * xx;
            xx = xx * d_evalpoint;
        }
        return res;
    }

    template<class SS>
    inline poly_evaluator_impl<n - 1, T, poly_evaluator<n, T, Alloc, S>, Alloc, SS> operator() (const SS & x) const
    // Continue evaluation to lower level.
    {
        return poly_evaluator_impl<n - 1, T, poly_evaluator<n, T, Alloc, S>, Alloc, SS>(*this, x);
    }
};

template<class T, class Alloc>
class poly<0, T, Alloc>
// Stores a polynomial of degree 0, i.e. a scalar of type T. We assume that the type T is not "too"
// complex, otherwise this class will be partially not very effective.
{
private:
    Alloc d_allocator;
    T d_value;

public:
    inline poly(const T & v = 0, const Alloc & allocator = Alloc())
        : d_allocator(allocator), d_value(v)
    {
    }

    inline poly(const Alloc & allocator)
        : d_allocator(allocator), d_value(T())
    {
    }

    inline bool isZero() const
    {
        return d_value == 0;
    }

    inline operator const T & () const
    {
        return d_value;
    }

    inline poly & operator = (const T & v)
    {
        d_value = v;
        return *this;
    }

    inline T operator() () const // evaluate
    {
        return d_value;
    }

    inline poly operator * (const T & v) const
    {
        return poly(d_value * v);
    }

    inline poly operator / (const T & v) const
    {
        return poly(d_value / v);
    }

    inline poly operator + (const T & v) const
    {
        return poly(d_value + v);
    }

    inline poly operator - (const T & v) const
    {
        return poly(d_value - v);
    }

    inline poly operator - () const
    {
        return poly(-d_value);
    }

    inline poly & operator *= (const T & v)
    {
        d_value *= v;
        return *this;
    }

    inline poly & operator /= (const T & v)
    {
        d_value /= v;
        return *this;
    }

    inline poly & operator += (const T & v)
    {
        d_value += v;
        return *this;
    }

    inline poly & operator -= (const T & v)
    {
        d_value -= v;
        return *this;
    }

    inline bool operator == (const T & v) const
    {
        return d_value == v;
    }

    inline bool operator != (const T & v) const
    {
        return d_value != v;
    }

    void print(std::ostream & s, int N = 0) const
    {
        s << d_value;
    }



    Alloc get_allocator() const
    {
        return d_allocator;
    }
};

// Next, we want to define the storage class poly_ivector<T, Alloc, usePointers>. It behaves like a
// subset of std::vector<T>'s capabilities (i.e. access elements, get size, set size, get last
// element, swap with other storage of same type), but uses std::vector<T*> in case usePointers is
// true.
//
// The advantage of this approach is that if T is a more complex object, reallocation done with
// resize() can be very costly.

template<class T, class Alloc = std::allocator<T>, bool usePointers = false>
// The version just using std::vector<T>.
class poly_ivector
{
private:
    std::vector<T, Alloc> d_vec;

public:
    poly_ivector(const Alloc & allocator = Alloc())
        : d_vec(allocator)
    {
    }

    poly_ivector(unsigned size, const Alloc & allocator = Alloc())
        : d_vec(size, T(), allocator)
    {
    }

    poly_ivector(unsigned size, const T & entry, const Alloc & allocator = Alloc())
        : d_vec(size, entry, allocator)
    {
    }

    unsigned size() const
    {
        return d_vec.size();
    }

    void resize(unsigned size, const T & entry = T())
    {
        d_vec.resize(size, entry);
    }

    const T & operator[] (unsigned i) const
    {
        return d_vec[i];
    }

    T & operator[] (unsigned i)
    {
        return d_vec[i];
    }

    const T & back() const
    {
        return d_vec.back();
    }

    T & back()
    {
        return d_vec.back();
    }



    Alloc get_allocator() const
    {
        return d_vec.get_allocator();
    }
};

template<class T, class Alloc>
// The version using std::vector<T*>, but behaving like std::vector<T>.
class poly_ivector<T, Alloc, true>
{
private:
    Alloc d_allocator;
    std::vector<typename Alloc::pointer, typename Alloc::template rebind<typename Alloc::pointer>::other> d_vec;

    void create(unsigned begin, unsigned end, typename Alloc::const_reference entry)
    {
        for (unsigned i = begin; i < end; ++i)
        {
            d_vec[i] = d_allocator.allocate(sizeof(T));
            d_allocator.construct(d_vec[i], entry);
        }
    }

    void free(unsigned begin, unsigned end)
    {
        for (unsigned i = begin; i < end; ++i)
        {
            d_allocator.destroy(d_vec[i]);
            d_allocator.deallocate(d_vec[i], sizeof(T));
        }
    }

    template<class A>
    void copy_from(const std::vector<typename Alloc::pointer, A> & source)
    {
        for (unsigned i = 0; i < d_vec.size(); ++i)
        {
            d_vec[i] = d_allocator.allocate(sizeof(T));
            d_allocator.construct(d_vec[i], *source[i]);
        }
    }

public:
    poly_ivector(const Alloc & allocator = Alloc())
        : d_allocator(allocator), d_vec(allocator)
    {
    }

    poly_ivector(unsigned size, const Alloc & allocator = Alloc())
        : d_allocator(allocator), d_vec(size, allocator)
    {
        create(0, size, T());
    }

    poly_ivector(unsigned size, const T & entry, const Alloc & allocator = Alloc())
        : d_allocator(allocator), d_vec(size, allocator)
    {
        create(0, size, entry);
    }

    poly_ivector(const poly_ivector & v)
        : d_vec(v.size())
    {
        copy_from(v.d_vec);
    }

    ~poly_ivector()
    {
        free(0, d_vec.size());
    }

    poly_ivector & operator = (const poly_ivector & v)
    {
        if (&v != this)
        {
            free(0, d_vec.size());
            d_vec.resize(v.size());
            copy_from(v.d_vec);
        }
        return *this;
    }

    unsigned size() const
    {
        return d_vec.size();
    }

    void resize(unsigned size, const T & entry = T())
    {
        unsigned oldsize = d_vec.size();
        if (oldsize > size)
            free(size, oldsize);
        d_vec.resize(size);
        if (oldsize < size)
            create(oldsize, size, entry);
    }

    const T & operator[] (unsigned i) const
    {
        return *d_vec[i];
    }

    T & operator[] (unsigned i)
    {
        return *d_vec[i];
    }

    const T & back() const
    {
        return *d_vec.back();
    }

    T & back()
    {
        return *d_vec.back();
    }



    Alloc get_allocator() const
    {
        return d_vec.get_allocator();
    }
};



template<int n, class T, class Alloc>
class poly
// The main polynomial class. Stores a polynomial in n indeterminates, with n > 0.
{


    template<int nn, class TT, class AA, class SS>
    friend class poly_evaluator;

    template<int nn, class TT, class HLHL, class AA, class SS>
    friend class poly_evaluator_impl;

private:
    static poly<n - 1, T, Alloc> d_zero;
    poly_ivector<poly<n - 1, T, Alloc>, typename Alloc::template rebind<poly<n - 1, T, Alloc> >::other, (n>1)> d_value;

    inline poly(bool, unsigned s, const Alloc & allocator)
        : d_value(s) // initialize array of length s
    {
    }

public:
    inline void normalize()
    // Adjusts the size of d_value that the leading term and degree can be computed trivially. This
    // must be called only after calls to the non-const operator[], in which the degree of the
    // polynomial has potentially been changed.
    {
        unsigned dp1 = d_value.size();
        while (dp1)
        {
            if (d_value[dp1 - 1].isZero())
                --dp1;
            else
                break;
        }
        if (dp1 < d_value.size())
            d_value.resize(dp1);
    }

    inline poly(const Alloc & allocator = Alloc())
    // Constructs a zero polynomial
        : d_value(allocator)
    {
    }

    inline poly(const T & v, const Alloc & allocator = Alloc())
    // Constructs a constant polynomial with constant term v.
        : d_value(1, poly<n - 1, T, Alloc>(v), allocator)
    {
    }

    inline poly(const poly<n - 1, T, Alloc> & pdm1, const Alloc & allocator = Alloc())
    // Constructs a polynomial of type poly<n, T> which is initialized with a polynomial of type poly<n-1, T>.
        : d_value(1, pdm1)
    {
    }

    template<class S, class A>
    inline poly(const poly<n, S, A> & p, const Alloc & allocator = Alloc())
    // Casts a polynomial of type poly<n, S> to a polynomial of type poly<n, T>.
        : d_value(p.degree() + 1, poly<n - 1, T, Alloc>(), allocator)
    {
        for (unsigned i = 0; i < d_value.size(); ++i)
            d_value[i] = p[i];
        normalize();
    }

    template<class S, class A>
    inline poly & operator = (const poly<n, S, A> & p)
    // Copies a polynomial of type poly<n, S> to this polynomial (of type poly<n, T>).
    {
        d_value.resize(p.degree() + 1);
        for (unsigned i = 0; i < d_value.size(); ++i)
            d_value[i] = p[i];
        normalize();
        return *this;
    }

    inline int degree() const
    // Returns the degree of this polynomial. If this is the zero polynomial, the degree is -1.
    {
        return d_value.size() - 1;
    }

    inline const poly<n - 1, T, Alloc> & leading() const
    // Returns the leading term (of type poly<n-1, T>) of the first indeterminate. Returns 0 (of
    // type poly<n-1, T>) in case of the zero polynomial.
    {
        return d_value.size() ? d_value.back() : d_zero;
    }

    inline bool isZero() const
    // Tests whether this polynomial is the zero polynomial.
    {
        return d_value.size() == 0;
    }

    inline poly<n - 1, T, Alloc> & operator[] (unsigned i)
    // Returns a reference to the i-th coefficient. If i > degree(), the array d_value is
    // enlarged. Afterwards, one should better call normalize() to make sure future operations are
    // correct.
    {
        if (i >= d_value.size())
            d_value.resize(i + 1);
        return d_value[i];
    }

    inline const poly<n - 1, T, Alloc> & operator[] (unsigned i) const
    // Returns a reference to the i-th coefficient, or zero if i > degree().
    {
        return i < d_value.size() ? d_value[i] : d_zero;
    }

    inline poly_evaluator<n, T, Alloc, T> operator() (const T & x) const
    // Evaluate in x.
    {
        return poly_evaluator<n, T, Alloc, T>(*this, x);
    }

    template<class S>
    inline poly_evaluator<n, T, Alloc, S> operator() (const S & x) const
    // Evaluate in x of type S.
    {
        return poly_evaluator<n, T, Alloc, S>(*this, x);
    }

    inline poly operator * (const T & v) const
    // Multiply by constant.
    {
        poly r(*this);
        for (unsigned i = 0; i < d_value.size(); ++i)
            r[i] *= v;
        return r;
    }

    inline poly operator / (const T & v) const
    // Division by constant.
    {
        poly r(*this);
        for (unsigned i = 0; i < d_value.size(); ++i)
            r[i] /= v;
        return r;
    }

    inline poly & operator *= (const poly & p)
    // Multiplication by a polynomial.
    {
        return *this = *this * p;
    }

    inline poly & operator *= (const T & v)
    // Multiplication by a constant.
    {
        poly r(*this);
        for (unsigned i = 0; i < d_value.size(); ++i)
            d_value[i] *= v;
        return *this;
    }

    inline poly & operator /= (const T & v)
    // Division by a constant.
    {
        for (unsigned i = 0; i < d_value.size(); ++i)
            d_value[i] /= v;
        return *this;
    }

    friend inline poly operator * (const T & v, const poly & p)
    // Multiplication by a constant from the left.
    {
        poly r(p);
        for (unsigned i = 0; i < p.d_value.size(); ++i)
            r[i] *= v;
        return r;
    }

    inline poly operator - () const
    // Returns the additive inverse of the polynomial.
    {
        poly r(true, d_value.size(), get_allocator());
        for (unsigned i = 0; i < d_value.size(); ++i)
            r[i] = -d_value[i];
        return r;
    }

    inline poly operator + (const poly & q) const
    // Computes the sum of two polynomials.
    {
        poly r(*this);
        if (r.d_value.size() < q.d_value.size())
            r.d_value.resize(q.d_value.size());
        for (unsigned i = 0; i < q.d_value.size(); ++i)
            r[i] += q[i];
        r.normalize();
        return r;
    }

    inline poly operator - (const poly & q) const
    // Computes the difference of two polynomials.
    {
        poly r(*this);
        if (r.d_value.size() < q.d_value.size())
            r.d_value.resize(q.d_value.size());
        for (unsigned i = 0; i < q.d_value.size(); ++i)
            r[i] -= q[i];
        r.normalize();
        return r;
    }

    inline poly & operator += (const poly & q)
    // Adds q to this polynomial.
    {
        if (d_value.size() < q.d_value.size())
            d_value.resize(q.d_value.size());
        for (unsigned i = 0; i < q.d_value.size(); ++i)
            d_value[i] += q[i];
        normalize();
        return *this;
    }

    inline poly & operator -= (const poly & q)
    // Subtracts q from this polynomial.
    {
        if (d_value.size() < q.d_value.size())
            d_value.resize(q.d_value.size());
        for (unsigned i = 0; i < q.d_value.size(); ++i)
            d_value[i] -= q[i];
        normalize();
        return *this;
    }

    inline poly operator + (const poly<n - 1, T, Alloc> & q) const
    // Computes the sum of this polynomial with one of degree one less.
    {
        poly r(*this);
        if (r.d_value.size() < 1)
            r.d_value.resize(1);
        r[0] += q;
        r.normalize();
        return r;
    }

    inline poly operator - (const poly<n - 1, T, Alloc> & q) const
    // Computes the difference of this polynomial with one of degree one less.
    {
        poly r(*this);
        if (r.d_value.size() < 1)
            r.d_value.resize(1);
        r[0] -= q;
        r.normalize();
        return r;
    }

    inline poly operator + (const T & v) const
    // Computes the sum with a constant.
    {
        poly r(*this);
        if (r.d_value.size() < 1)
            r.d_value.resize(1);
        r[0] += v;
        r.normalize();
        return r;
    }

    inline poly operator - (const T & v) const
    // Computes the difference to a constant.
    {
        poly r(*this);
        if (r.d_value.size() < 1)
            r.d_value.resize(1);
        r[0] -= v;
        r.normalize();
        return r;
    }

    inline poly & operator += (const poly<n - 1, T, Alloc> & q)
    // Adds a polynomial of degree n-1.
    {
        if (d_value.size() < 1)
            d_value.resize(1);
        d_value[0] += q;
        normalize();
        return *this;
    }

    inline poly & operator -= (const poly<n - 1, T, Alloc> & q)
    // Subtracts a polynomial of degree n-1.
    {
        if (d_value.size() < 1)
            d_value.resize(1);
        d_value[0] -= q;
        normalize();
        return *this;
    }

    inline poly & operator += (const T & v)
    // Adds a constant.
    {
        if (d_value.size() < 1)
            d_value.resize(1);
        d_value[0] += v;
        normalize();
        return *this;
    }

    inline poly & operator -= (const T & v)
    // Subtracts a constant.
    {
        if (d_value.size() < 1)
            d_value.resize(1);
        d_value[0] -= v;
        normalize();
        return *this;
    }

//    inline bool operator == (const poly & q) const
//    // Compares two polynomials.
//    {
//        if (d_value.size() != q.d_value.size())
//            return false;
//        for (unsigned i = 0; i < d_value.size(); ++i)
//            if (d_value[i] != q[i])
//                return false;
//        return true;
//    }

//    inline bool operator != (const poly & q) const
//    // Compares two polynomials for inequality.
//    {
//        return !(*this == q);
//    }

//    inline bool operator == (const T & v) const
//    // Compares a polynomial to a constant.
//    {
//        if ((v == 0) && (d_value.size() == 0))
//            return true;
//        if (d_value.size() != 1)
//            return false;
//        return d_value[0] == v;
//    }

//    inline bool operator != (const T & v) const
//    // Compares a polynomial to a constant for inequality.
//    {
//        return !(*this == v);
//    }

    void print(std::ostream & s, int N = n) const
    // Prints this polynomial to the stream s. N gives the number of variables; this is needed for
    // recursive printing.
    {
        if (isZero())
            s << (T)0;
        else
        {
            unsigned nonzero = 0;
            for (unsigned i = 0; i < d_value.size(); ++i)
                if (!d_value[i].isZero())
                    ++nonzero;
            if (nonzero > 1)
                s << "(";
            bool first = true;
            for (unsigned i = 0; i < d_value.size(); ++i)
                if (!d_value[i].isZero())
                {
                    if (first)
                        first = false;
                    else
                        s << " + ";
                    d_value[i].print(s, N);
                    if (i > 0)
                    {
                        s << " ";
                        s << "X_" << N - n;
                        if (i > 1)
                            s << "^" << i;
                    }

                }
            if (nonzero > 1)
                s << ")";
        }
    }

    friend inline std::ostream & operator << (std::ostream & s, const poly & p)
    // Outputs p to s using print().
    {
        p.print(s);
        return s;
    }

    inline poly operator * (const poly & p) const
    // Multiplies the polynomial with another polynomial.
    {
        int d = p.degree() + degree();
        if (d < -1)
            d = -1;
        poly r(true, d + 1, get_allocator());
        if (!isZero() && !p.isZero())
            for (unsigned i = 0; i < r.d_value.size(); ++i)
                for (unsigned j = 0; j < d_value.size(); ++j)
                    if (i < j + p.d_value.size())
                        r[i] += d_value[j] * p[i - j];
        r.normalize();
        return r;
    }



    Alloc get_allocator() const
    {
        return d_value.get_allocator();
    }
};

template<class T, class Alloc>
inline poly<1, T, Alloc> Monomial(unsigned e)
// Creates a monomial in one indeterminate.
{
    poly<1, T, Alloc> p;
    p[e] = 1;
    return p;
}

template<class T, class Alloc>
inline poly<2, T, Alloc> Monomial(unsigned e, unsigned f)
// Creates a monomial in two indeterminates.
{
    poly<2, T, Alloc> p;
    p[e][f] = 1;
    return p;
}

template<class T, class Alloc>
inline poly<3, T, Alloc> Monomial(unsigned e, unsigned f, unsigned g)
// Creates a monomial in three indeterminates.
{
    poly<3, T, Alloc> p;
    p[e][f][g] = 1;
    return p;
}

template<class T, class Alloc>
inline poly<4, T, Alloc> Monomial(unsigned e, unsigned f, unsigned g, unsigned h)
// Creates a monomial in four indeterminates.
{
    poly<4, T, Alloc> p;
    p[e][f][g][h] = 1;
    return p;
}

template<class T, class Alloc>
inline poly<5, T, Alloc> Monomial(unsigned e, unsigned f, unsigned g, unsigned h,unsigned i)
// Creates a monomial in five indeterminates.
{
    poly<5, T, Alloc> p;
    p[e][f][g][h][i] = 1;
    return p;
}

template<class T>
inline poly<1, T, std::allocator<T> > Monomial(unsigned e)
// Creates a monomial in one indeterminate.
{
    poly<1, T, std::allocator<T> > p;
    p[e] = 1;
    return p;
}

template<class T>
inline poly<2, T, std::allocator<T> > Monomial(unsigned e, unsigned f)
// Creates a monomial in two indeterminates.
{
    poly<2, T, std::allocator<T> > p;
    p[e][f] = 1;
    return p;
}

template<class T>
inline poly<3, T, std::allocator<T> > Monomial(unsigned e, unsigned f, unsigned g)
// Creates a monomial in three indeterminates.
{
    poly<3, T, std::allocator<T> > p;
    p[e][f][g] = 1;
    return p;
}

template<class T>
inline poly<4, T, std::allocator<T> > Monomial(unsigned e, unsigned f, unsigned g, unsigned h)
// Creates a monomial in four indeterminates.
{
    poly<4, T, std::allocator<T> > p;
    p[e][f][g][h] = 1;
    return p;
}

template<class T>
inline poly<4, T, std::allocator<T> > Monomial_V(unsigned e, unsigned f, unsigned g, unsigned h)
// Creates a monomial in four indeterminates with vector input.
{
    poly<4, T, std::allocator<T> > p;
    p[e][f][g][h] = 1;
    return p;
}

template<class T>
inline poly<5, T, std::allocator<T> > Monomial(unsigned e, unsigned f, unsigned g, unsigned h, unsigned i)
// Creates a monomial in five indeterminates.
{
    poly<5, T, std::allocator<T> > p;
    p[e][f][g][h][i] = 1;
    return p;
}








template<int n, class T, class Alloc>
poly<n - 1, T, Alloc> poly<n, T, Alloc>::d_zero; // Declare the zero coefficient.


#endif // MULTIPOLY_H
