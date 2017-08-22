#pragma once
#include "TypeClass.hpp"
#include "Print.hpp"

namespace Test_TypeClass {
	using Functional::Applyable;
	using Functional::curry;

	using namespace TypeClass;

	template<typename T>
	T add(T x, T y) {
		return x + y;
	}

	Maybe<float> devide(float n, float d) {
		if (d == 0) return Make::maybe<float>();
		else return Make::maybe(n / d);
	}

	size_t counter = 1;

	IO<size_t> getCounter() {
		return counter++;
	}

	void test() {
		auto buff1 = curry([](float x) { return unit<Maybe>(add(5.0f, x)); });
		Applyable<Maybe<float>, float> *a1 = &buff1;
		Maybe<float> res1 = bind(a1, devide(1.0f, 2.0f));
		if (res1.has_data) println(res1); else println("Nothing");

		auto buff2 = curry([](float x) { return add(5.0f, x); });
		Applyable<float, float> *a2 = &buff2;
		Maybe<float> res2 = bindFunc(a2, devide(1.0f, 2.0f));
		if (res2.has_data) println(res2); else println("Nothing");

		Maybe<float> res3 = bindVal(Make::maybe(5.0f), devide(1, 2));
		if (res3.has_data) println(res3); else println("Nothing");
		
		auto buff4 = curry([](size_t x) { 
			auto buff4 = curry([](size_t x) {
				auto buff4 = curry([](size_t x) {
					return add<size_t>(0, x);
				});
				Applyable<size_t, size_t> *a4 = &buff4;
				return add(x, bindFunc(a4, getCounter()).data);
			});
			Applyable<size_t, size_t> *a4 = &buff4;
			return add(x, bindFunc(a4, getCounter()).data);
		});
		Applyable<size_t, size_t> *a4 = &buff4;
		println(bindFunc(a4, getCounter()));
	}
}  