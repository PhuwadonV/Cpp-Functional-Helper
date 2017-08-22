#pragma once
#include "Functional.hpp"

namespace TypeClass {
	template<typename T>
	class Data {
	public:
		T data;

		Data() = default;
		Data(T data) : data(data) {}
		Data(const Data &rhs) : Data(rhs.data) {}
		Data(Data &&rhs) : Data(static_cast<T&&>(rhs.data)) {}

		Data& operator=(const Data &rhs) {
			data = rhs.data;
		}

		Data& operator=(Data &&rhs) {
			data = static_cast<T&&>(rhs.data);
		}

		operator T() const {
			return data;
		}

		operator T&() {
			return data;
		}
	};

	template<typename T>
	class Maybe : public Data<T> {
		using Base = Data<T>;
	public:
		bool has_data;

		Maybe(bool has_data) : Base(), has_data(has_data) {}
		Maybe(bool has_data, T data) : Base(data), has_data(has_data) {}
		Maybe(const Maybe &rhs) : Base(rhs), has_data(rhs.has_data) {}
		Maybe(Maybe &&rhs) : Base(static_cast<Maybe&&>(rhs)), has_data(rhs.has_data) {}

		Maybe& operator=(const Maybe &rhs) {
			has_data = rhs.has_data;
			Base::operator=(rhs);
			return *this;
		}

		Maybe& operator=(Maybe &&rhs) {
			has_data = rhs.has_data;
			Base::operator=(static_cast<Maybe&&>(rhs));
			return *this;
		}
	};

	template<typename T>
	class IO : public Data<T> {
		using Base = Data<T>;
	public:
		IO(T data) : Base(data) {}
		IO(const IO &rhs) : Base(rhs) {}
		IO(IO &&rhs) : Base(static_cast<IO&&>(rhs)) {}

		IO& operator=(const IO &rhs) {
			Base::operator=(rhs);
			return *this;
		}

		IO& operator=(IO &&rhs) {
			Base::operator=(static_cast<IO&&>(rhs));
			return *this;
		}
	};

	namespace Make {
		template<typename Arg>
		inline Maybe<Arg> maybe(Arg arg) {
			return Maybe<Arg>(true, arg);
		}

		template<typename Arg>
		inline Maybe<Arg> maybe() {
			return Maybe<Arg>(false);
		}

		template<typename Arg>
		inline IO<Arg> io(Arg arg) {
			return IO<Arg>(arg);
		}
	}

	template<typename>
	class Default;

	template<template<typename>typename>
	class Functor;

	template<>
	class Functor<Maybe> {
	public:
		template<typename R, typename Arg>
		inline static R fmap(const Functional::Applyable<R, Arg> *f, Maybe<Arg> arg) {
			if (arg.has_data) return Make::maybe(f->apply(arg.data));
			return Make::maybe();
			
		}
	};

	template<>
	class Functor<IO> {
	public:
		template<typename R, typename Arg>
		inline static R fmap(const Functional::Applyable<R, Arg> *f, IO<Arg> arg) {
			return Make::io(f->apply(arg.data));
		}
	};

	template<template<typename>typename>
	class Applicative;

	template<template<typename>typename M>
	class Default<Applicative<M>> : public Functor<M> {};

	template<>
	class Applicative<Maybe> : public Default<Applicative<Maybe>> {
	public:
		template<typename Arg>
		inline static Maybe<Arg> pure(Arg arg) {
			return Make::maybe(arg);
		}

		template<typename R, typename Arg>
		inline static Maybe<R> applicative(Maybe<const Functional::Applyable<R, Arg>*> f, Maybe<Arg> arg) {
			if (f.has_data) return fmap(f.data, arg);
			return Make::maybe();
		}
	};

	template<>
	class Applicative<IO> : public Default<Applicative<IO>> {
	public:
		template<typename Arg>
		inline static IO<Arg> pure(Arg arg) {
			return Make::io(arg);
		}

		template<typename R, typename Arg>
		inline static IO<R> applicative(IO<const Functional::Applyable<R, Arg>*> f, IO<Arg> arg) {
			return fmap(f.data, arg);
		}
	};

	template<template<typename>typename>
	class Monad;

	template<template<typename>typename M>
	class Default<Monad<M>> : public Applicative<M> {
	public:
		template<typename A, typename B>
		inline static M<B> bindVal(M<B> b, M<A> a) {
			auto buff = Functional::curry([b](A a) { return b; });
			return Monad<M>::bind(static_cast<Functional::Applyable<M<B>, A>*>(&buff), a);
		}

		template<typename B, typename A>
		inline static M<B> bindFunc(const Functional::Applyable<B, A> *f, M<A> a) {
			auto buff = Functional::curry([f](A arg) { return  Monad<M>::unit(f->apply(arg)); });
			return Monad<M>::bind(static_cast<Functional::Applyable<M<B>, A>*>(&buff), a);
		}
	};

	template<>
	class Monad<Maybe> : public Default<Monad<Maybe>> {
	public:
		template<typename Arg>
		inline static Maybe<Arg> unit(Arg arg) {
			return pure(arg);
		}

		template<typename A, typename B>
		inline static Maybe<B> bind(const Functional::Applyable<Maybe<B>, A> *f, Maybe<A> a) {
			if (!a.has_data) return Make::maybe<B>();
			return f->apply(a.data);
		}
	};

	template<>
	class Monad<IO> : public Default<Monad<IO>> {
	public:
		template<typename Arg>
		inline static IO<Arg> unit(Arg arg) {
			return pure(arg);
		}

		template<typename A, typename B>
		inline static IO<B> bind(const Functional::Applyable<IO<B>, A> *f, IO<A> a) {
			return f->apply(a.data);
		}
	};
}