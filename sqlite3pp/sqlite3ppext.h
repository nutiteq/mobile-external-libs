// sqlite3ppext.h
//
// The MIT License
//
// Copyright (c) 2012 Wongoo Lee (iwongu at gmail dot com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
#pragma once

#include "sqlite3pp.h"
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits.hpp>

namespace sqlite3pp
{
  namespace ext
  {

    class context : boost::noncopyable
    {
     public:
      explicit context(sqlite3_context* ctx, int nargs = 0, sqlite3_value** values = 0);

      int args_count() const;
      int args_bytes(int idx) const;
      int args_type(int idx) const;

      template <class T> T get(int idx) const {
        return get(idx, T());
      }

      void result(int value);
      void result(double value);
      void result(long long int value);
      void result(std::string const& value);
      void result(char const* value, bool fstatic = true);
      void result(void const* value, int n, bool fstatic = true);
      void result();
      void result(std::nullptr_t);
      void result_copy(int idx);
      void result_error(char const* msg);

      void* aggregate_data(int size);

     private:
      int get(int idx, int) const;
      double get(int idx, double) const;
      long long int get(int idx, long long int) const;
      char const* get(int idx, char const*) const;
      std::string get(int idx, std::string) const;
      void const* get(int idx, void const*) const;

     private:
      sqlite3_context* ctx_;
      int nargs_;
      sqlite3_value** values_;
    };

    namespace
    {
      template <class R>
      void function0_impl(sqlite3_context* ctx, int nargs, sqlite3_value** values)
      {
        boost::function<R ()>* f =
            static_cast<boost::function<R ()>*>(sqlite3_user_data(ctx));
        context c(ctx, nargs, values);
        c.result((*f)());
      }

      template <class R, class P1>
      void function1_impl(sqlite3_context* ctx, int nargs, sqlite3_value** values)
      {
        boost::function<R (P1)>* f =
            static_cast<boost::function<R (P1)>*>(sqlite3_user_data(ctx));
        context c(ctx, nargs, values);
        c.result((*f)(c.context::get<P1>(0)));
      }

      template <class R, class P1, class P2>
      void function2_impl(sqlite3_context* ctx, int nargs, sqlite3_value** values)
      {
        boost::function<R (P1, P2)>* f =
            static_cast<boost::function<R (P1, P2)>*>(sqlite3_user_data(ctx));
        context c(ctx, nargs, values);
        c.result((*f)(c.context::get<P1>(0), c.context::get<P2>(1)));
      }

      template <class R, class P1, class P2, class P3>
      void function3_impl(sqlite3_context* ctx, int nargs, sqlite3_value** values)
      {
        boost::function<R (P1, P2, P3)>* f =
            static_cast<boost::function<R (P1, P2, P3)>*>(sqlite3_user_data(ctx));
        context c(ctx, nargs, values);
        c.result((*f)(c.context::get<P1>(0), c.context::get<P2>(1), c.context::get<P3>(2)));
      }

      template <class R, class P1, class P2, class P3, class P4>
      void function4_impl(sqlite3_context* ctx, int nargs, sqlite3_value** values)
      {
        boost::function<R (P1, P2, P3, P4)>* f =
            static_cast<boost::function<R (P1, P2, P3, P4)>*>(sqlite3_user_data(ctx));
        context c(ctx, nargs, values);
        c.result((*f)(c.context::get<P1>(0), c.context::get<P2>(1), c.context::get<P3>(2), c.context::get<P4>(3)));
      }

      template <class R, class P1, class P2, class P3, class P4, class P5>
      void function5_impl(sqlite3_context* ctx, int nargs, sqlite3_value** values)
      {
        boost::function<R (P1, P2, P3, P4, P5)>* f =
            static_cast<boost::function<R (P1, P2, P3, P4, P5)>*>(sqlite3_user_data(ctx));
        context c(ctx, nargs, values);
        c.result((*f)(c.context::get<P1>(0), c.context::get<P2>(1), c.context::get<P3>(2), c.context::get<P4>(3), c.context::get<P5>(4)));
      }

    }


    class function : boost::noncopyable
    {
     public:
      typedef boost::function<void (context&)> function_handler;
      typedef boost::shared_ptr<boost::function_base> pfunction_base;

      explicit function(database& db);

      int create(char const* name, function_handler h, int nargs = 0);

      template <class F> int create(char const* name, boost::function<F> h) {
        fh_[name] = boost::shared_ptr<boost::function_base>(new boost::function<F>(h));
        return create_function_impl<boost::function<F>::arity, F>()(db_, fh_[name].get(), name);
      }

     private:
      template <int N, class F>
      struct create_function_impl;

      template <class F>
      struct create_function_impl<0, F> {
        int operator()(sqlite3* db, boost::function_base* fh, char const* name) {
          typedef boost::function_traits<F> FT;
          typedef typename FT::result_type R;

          return sqlite3_create_function(db, name, 0, SQLITE_UTF8, fh,
                                         function0_impl<R>,
                                         0, 0);
        }
      };

      template <class F>
      struct create_function_impl<1, F> {
        int operator()(sqlite3* db, boost::function_base* fh, char const* name) {
          typedef boost::function_traits<F> FT;
          typedef typename FT::result_type R;
          typedef typename FT::arg1_type P1;

          return sqlite3_create_function(db, name, 1, SQLITE_UTF8, fh,
                                         function1_impl<R, P1>,
                                         0, 0);
        }
      };

      template <class F>
      struct create_function_impl<2, F> {
        int operator()(sqlite3* db, boost::function_base* fh, char const* name) {
          typedef boost::function_traits<F> FT;
          typedef typename FT::result_type R;
          typedef typename FT::arg1_type P1;
          typedef typename FT::arg2_type P2;

          return sqlite3_create_function(db, name, 2, SQLITE_UTF8, fh,
                                         function2_impl<R, P1, P2>,
                                         0, 0);
        }
      };

      template <class F>
      struct create_function_impl<3, F> {
        int operator()(sqlite3* db, boost::function_base* fh, char const* name) {
          typedef boost::function_traits<F> FT;
          typedef typename FT::result_type R;
          typedef typename FT::arg1_type P1;
          typedef typename FT::arg2_type P2;
          typedef typename FT::arg3_type P3;

          return sqlite3_create_function(db, name, 3, SQLITE_UTF8, fh,
                                         function3_impl<R, P1, P2, P3>,
                                         0, 0);
        }
      };

      template <class F>
      struct create_function_impl<4, F> {
        int operator()(sqlite3* db, boost::function_base* fh, char const* name) {
          typedef boost::function_traits<F> FT;
          typedef typename FT::result_type R;
          typedef typename FT::arg1_type P1;
          typedef typename FT::arg2_type P2;
          typedef typename FT::arg3_type P3;
          typedef typename FT::arg4_type P4;

          return sqlite3_create_function(db, name, 4, SQLITE_UTF8, fh,
                                         function4_impl<R, P1, P2, P3, P4>,
                                         0, 0);
        }
      };

      template <class F>
      struct create_function_impl<5, F> {
        int operator()(sqlite3* db, boost::function_base* fh, char const* name) {
          typedef boost::function_traits<F> FT;
          typedef typename FT::result_type R;
          typedef typename FT::arg1_type P1;
          typedef typename FT::arg2_type P2;
          typedef typename FT::arg3_type P3;
          typedef typename FT::arg4_type P4;
          typedef typename FT::arg5_type P5;

          return sqlite3_create_function(db, name, 5, SQLITE_UTF8, fh,
                                         function5_impl<R, P1, P2, P3, P4, P5>,
                                         0, 0);
        }
      };

     private:
      sqlite3* db_;

      std::map<std::string, pfunction_base> fh_;
    };
  } // namespace ext

} // namespace sqlite3pp

