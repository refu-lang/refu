###########################
Refu Language Specification
###########################

*******************
Built-in data types
*******************

The following data types are built-in. Some of them correspond to the data types
defined by the C99 standard in ``<stdint.h>`` but they follow the same naming
scheme as in rust.

- **Unsigned numbers**

  - ``uint``: compiler will decide the size depending on context
  - ``u8``: 8 bit unsigned integer, corresponding to ``uint8_t``
  - ``u16``: 16 bit unsigned integer, corresponding to ``uint16_t``
  - ``u32``: 32 bit unsigned integer, corresponding to ``uint32_t``
  - ``u64``: 64 bit unsigned integer, corresponding to ``uint64_t``

- **Signed numbers**

  - ``int``: compiler will decide the size depending on context
  - ``i8``: 8 bit signed integer, corresponding to ``int8_t``
  - ``i16``: 16 bit signed integer, corresponding to ``int16_t``
  - ``i32``: 32 bit signed integer, corresponding to ``int32_t``
  - ``i64``: 64 bit signed integer, corresponding to ``int64_t``

- **Real numbers**

  - ``f32``: corresponds to binary32, single precision floating point, as
    defined by `IEE 754-2008 <http://en.wikipedia.org/wiki/IEEE_754-2008>`_
  - ``f64``: corresponds to binary64, double precision floating point, as
    defined by `IEE 754-2008 <http://en.wikipedia.org/wiki/IEEE_754-2008>`_

- **Strings**

  - ``string``: UTF-8 encoded unicode string.
  - ``string8``: Ascii encoded string

- **Other**

  - ``bool``: A boolean true or false value
  - ``nil``: the unit type, also known as NULL

*******************
Alebraic Data Types
*******************

More complex data types can be defined by the user as Algrebraic data types. This is
achieved with the ``type`` keyword.::

    type person {
        name:string, age:int |
        name:string, age:int, surname:string
    }

    type list {
         nil | (load:int, tail:list)
    }

    type foo {
        a:int,
        b:(string|float)
    }

    type foo {
        a:int,
        b:(string | (i:int, f:f32))
    }

Above we have the definition of a person and a list. A person has a name
and an age and optionally a surname. And a list is either empty (denoted
by the ``nil`` keyword or it has a load of an ``int`` and a tail which is another
list.

Recursive Data Types
====================

TODO: Describe recursive ADTs

Anonymous Data Types
====================

An algebraic data type can be considered as the equivalent of a
tagged union type in C. Refu also supports anonymous ADTs. That means,
you can encounter the ADT syntax without it having been defined.
For example, a function's argument can be an anonymous ADT.::

    fn print_me(a:(string | b:int, c:int))
    {
        //do some initialization stuff
        ...
        //and now do the pattern matching
        match a {
            string   => print(a)
            int, int => print(a.b, a.c)
        }
    }

::

    fn print_me(a:string | (b:int, c:int)) -> int
    _    => {
        print("%s", a)
        print("one argument")
    }
    _, _ => {
        print("%d %d", b, c)
        print("two arguments")
    }



Above we have one function with an anomymous ADT.
If such a  function exists then it must have a match expression somewhere
inside its body in order to distinguish what kind of input it is having
before this input is used. The most explicit way to achieve this is to
write the match expression explicitly as in example 1. To do that we match
the keyword fn inside the function's body against the various cases.

In another case if the function body consists only of different branches
depending on the input we can omit the function's body block completely
and go with the way that example 2 is defined, which resembles a lot the
way functions are defined in haskell. It is just syntactic sugar for
achieving the same thing as in example 1.

Instantiating objects
=====================

In order to construct an instance of a data type you have to use one of its
constructors. A constructor of an object is simply defined as any of its
sum type operands.::

    a:person = person("steven", 23)
    b:person = person("celina", 22, "wojtowicz")

For ease of use arguments can also be given to a
constructor as keyword arguments. If one keyword argument is passed to a
constructor then all arguments should be keyword arguments. Finally when
passing keyword arguments the order of the arguments does not matter as
opposed to when calling a constructor normally.::

    a:person = person(name="steven", age=23)
    b:person = person(name="celina", surname="wojtowicz", age=23)

.. note::

   Keyword arguments are currently not implemented

Instantiating Recursive data types
----------------------------------

.. note::

   Recursive data types are currently not implemented

Data types can also be recursive. This is how we can define collections in Refu. But how do you construct a collection?::

    a:list = nil
    b:list = list(1, 2, 3, 4, 5)
    c:list = list(1, list( 2, list(3, list(4, list(5, nil)))))


In the above examples list ``b`` and list ``c`` are equal. The canonical way to
define a list would be exactly like list ``c`` is defined, having /1/ as its
first element and using ``nil`` after ``5`` to denote the list's end.

As we can see above to construct a recursive data type we still use a
constructor but we can take advantage of the fact that the type is recursive
in order to construct it.

In the case of ``b`` 's construction Refu knows that a list's constructor can
only accept an int and a next list pointer. Using that knowledge it can
expand the ``list(1, 2, 3, 4, 5)`` to ``list(1, list(2, list(3, list(4, list(5, nil)))))``.

Same thing can work for more complex recursive data types such as a binary
tree. Look below for an example.::

    type binary_tree {
        nil | load:int, left:binary_tree, right:binary_tree
    }

    a:binary_tree = nil
    b:binary_tree = binary_tree(8, (4, (1, 7)), (12, (10, 19)))
    c:binary_tree = binary_tree(
        8,
        binary_tree(4,
                        binary_tree(1, nil, nil), binary_tree(7, nil, nil)),
        binary_tree(12,
                        binary_tree(10, nil, nil), binary_tree(19, nil, nil)))


