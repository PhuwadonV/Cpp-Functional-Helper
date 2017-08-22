#pragma once

namespace Functional {
	namespace TMP {
		template<typename Result>
		class Evalable {
		public:
			using Eval = Result;
		};

		template<typename From, typename To>
		class Convert : public Evalable<To> {};
	}

	template<typename...>
	class Tuple {
	public:
		Tuple() = default;
		Tuple(const Tuple&) = default;
		Tuple(Tuple&&) = default;

		Tuple& operator=(const Tuple&) = delete;
		Tuple& operator=(Tuple&&) = delete;
	};

	template<typename T>
	class Tuple<T> {
	public:
		T data;

		Tuple(T data) : data(data) {}
		Tuple(const Tuple &rhs) : data(rhs.data) {}
		Tuple(Tuple &&rhs) : Tuple(rhs) {}

		Tuple& operator=(const Tuple&) = delete;
		Tuple& operator=(Tuple&&) = delete;

		template<size_t>
		inline auto& get();

		template<>
		inline auto& get<0>() {
			return data;
		}
	};

	template<typename T, typename... Ts>
	class Tuple<T, Ts...> {
	public:
		T data;
		Tuple<Ts...> next;

		Tuple(T data, Tuple<Ts...> next) : data(data), next(next) {}
		Tuple(const Tuple &rhs) : data(rhs.data), next(rhs.next) {}
		Tuple(Tuple &&rhs) : Tuple(rhs) {}

		Tuple& operator=(const Tuple&) = delete;
		Tuple& operator=(Tuple&&) = delete;

		template<size_t n>
		inline auto& get() {
			return next.get<n - 1>();
		}

		template<>
		inline auto& get<0>() {
			return data;
		}
	};

	namespace Make {
		auto tuple() {
			return Tuple<>();
		}

		template<typename Arg>
		auto tuple(Arg arg) {
			return Tuple<Arg>(arg);
		}

		template<typename Arg, typename... Args>
		auto tuple(Arg arg, Args... args) {
			return Tuple<Arg, Args...>(arg, tuple(args...));
		}
	}

	template<typename R, typename... Args>
	class Applyable {
	public:
		virtual R apply(Args...) const = 0;
	};

	template<typename, typename, typename...>
	class CurryApplyable;

	template<typename R, typename Arg>
	class CurryApplyable<R, Arg> {
	public:
		virtual R apply(Arg) const = 0;
	};

	template<typename R, typename Arg1, typename Arg2, typename... Args>
	class CurryApplyable<R, Arg1, Arg2, Args...> {
	public:
		virtual CurryApplyable<R, Arg2, Args...>* newCurryApplyable(Arg1) const = 0;
	};

	template<typename, bool, typename, typename, typename, typename...>
	class FunctionHolder;

	template<typename F, typename R, typename Arg1, typename Arg2, typename... Args, typename T, typename... Ts>
	class FunctionHolder<F, true, Tuple<T, Ts...>, R, Arg1, Arg2, Args...> : public Applyable<R, Arg1, Arg2, Args...>, public CurryApplyable<R, Arg1, Arg2, Args...> {
		F m_f;
		Tuple<T, Ts...> m_ds;

		template<typename T, typename... Ts, typename... Args>
		auto call(Tuple<T, Ts...> ds, Args... args) const {
			return call(ds.next, ds.data, args...);
		}

		template<typename T, typename... Args>
		auto call(Tuple<T> ds, Args... args) const {
			return m_f(ds.data, args...);
		}
	public:
		FunctionHolder(F f, Tuple<T, Ts...> ds) : m_f(f), m_ds(ds) {}
		FunctionHolder(const FunctionHolder &rhs) : m_f(rhs.m_f) {}
		FunctionHolder(FunctionHolder &&rhs) : m_f(rhs.m_f) {}

		FunctionHolder& operator=(const FunctionHolder &rhs) {
			m_f = rhs.m_f;
			m_ds = rhs.m_ds;
		}

		FunctionHolder& operator=(FunctionHolder &&rhs) {
			m_f = static_cast<F&&>(rhs.m_f);
			m_ds = static_cast<F&&>(rhs.m_ds);
		}

		auto operator()(Arg1 arg) const {
			return FunctionHolder<F, true, Tuple<Arg1, T, Ts...>, R, Arg2, Args...>(m_f, Tuple<Arg1, T, Ts...>(arg, m_ds));
		}

		R apply(Arg1 arg1, Arg2 arg2, Args... args) const {
			return call(m_ds, arg1, arg2, args...);
		}

		F getFunction() {
			return m_f;
		}

		Tuple<T, Ts...> getData() {
			return m_ds;
		}

		Applyable<R, Arg1, Arg2, Args...>* newApplyable() {
			return new FunctionHolder(m_f, m_ds);
		}

		CurryApplyable<R, Arg1, Arg2, Args...>* newCurryApplyable() {
			return new FunctionHolder(m_f, m_ds);
		}

		CurryApplyable<R, Arg2, Args...>* newCurryApplyable(Arg1 arg) const {
			return new FunctionHolder<F, true, Tuple<Arg1, T, Ts...>, R, Arg2, Args...>(m_f, Tuple<Arg1, T, Ts...>(arg, m_ds));
		}
	};

	template<typename F, typename R, typename Arg1, typename Arg2, typename... Args>
	class FunctionHolder<F, false, void, R, Arg1, Arg2, Args...> : public Applyable<R, Arg1, Arg2, Args...>, public CurryApplyable<R, Arg1, Arg2, Args...> {
		F m_f;
	public:
		FunctionHolder(F f) : m_f(f) {}
		FunctionHolder(const FunctionHolder &rhs) : m_f(rhs.m_f) {}
		FunctionHolder(FunctionHolder &&rhs) : m_f(rhs.m_f) {}

		FunctionHolder& operator=(const FunctionHolder &rhs) {
			m_f = rhs.m_f;
		}

		FunctionHolder& operator=(FunctionHolder &&rhs) {
			m_f = static_cast<F&&>(rhs.m_f);
		}

		auto operator()(Arg1 arg) const {
			return FunctionHolder<F, true, Tuple<Arg1>, R, Arg2, Args...>(m_f, Tuple<Arg1>(arg));
		}

		R apply(Arg1 arg1, Arg2 arg2, Args... args) const {
			return m_f(arg1, arg2, args...);
		}

		F getFunction() {
			return m_f;
		}

		Applyable<R, Arg1, Arg2, Args...>* newApplyable() {
			return new FunctionHolder(m_f);
		}

		CurryApplyable<R, Arg1, Arg2, Args...>* newCurryApplyable() {
			return new FunctionHolder(m_f);
		}

		CurryApplyable<R, Arg2, Args...>* newCurryApplyable(Arg1 arg) const {
			return new FunctionHolder<F, true, Tuple<Arg1>, R, Arg2, Args...>(m_f, Tuple<Arg1>(arg));
		}
	};

	template<typename F, typename R, typename Arg, typename T, typename... Ts>
	class FunctionHolder<F, true, Tuple<T, Ts...>, R, Arg> : public Applyable<R, Arg>, public CurryApplyable<R, Arg> {
		F m_f;
		Tuple<T, Ts...> m_ds;

		template<typename T, typename... Ts, typename... Args>
		auto call(Tuple<T, Ts...> ds, Args... args) const {
			return call(ds.next, ds.data, args...);
		}

		template<typename T, typename... Args>
		auto call(Tuple<T> ds, Args... args) const {
			return m_f(ds.data, args...);
		}
	public:
		FunctionHolder(F f, Tuple<T, Ts...> ds) : m_f(f), m_ds(ds) {}
		FunctionHolder(const FunctionHolder &rhs) : m_f(rhs.m_f) {}
		FunctionHolder(FunctionHolder &&rhs) : m_f(rhs.m_f) {}

		FunctionHolder& operator=(const FunctionHolder &rhs) {
			m_f = rhs.m_f;
			m_ds = rhs.m_ds;
		}

		FunctionHolder& operator=(FunctionHolder &&rhs) {
			m_f = static_cast<F&&>(rhs.m_f);
			m_ds = static_cast<F&&>(rhs.m_ds);
		}

		R operator()(Arg arg) const {
			return call(m_ds, arg);
		}

		R apply(Arg arg) const {
			return call(m_ds, arg);
		}

		F getFunction() {
			return m_f;
		}

		Tuple<T, Ts...> getData() {
			return m_ds;
		}

		Applyable<R, Arg>* newApplyable() {
			return new FunctionHolder(m_f, m_ds);
		}

		CurryApplyable<R, Arg>* newCurryApplyable() {
			return new FunctionHolder(m_f, m_ds);
		}
	};

	template<typename F, typename R, typename Arg>
	class FunctionHolder<F, false, void, R, Arg> : public Applyable<R, Arg>, public CurryApplyable<R, Arg> {
		F m_f;
	public:
		FunctionHolder(F f) : m_f(f) {}
		FunctionHolder(const FunctionHolder &rhs) : m_f(rhs.m_f) {}
		FunctionHolder(FunctionHolder &&rhs) : m_f(rhs.m_f) {}

		FunctionHolder& operator=(const FunctionHolder &rhs) {
			m_f = rhs.m_f;
		}

		FunctionHolder& operator=(FunctionHolder &&rhs) {
			m_f = static_cast<F&&>(rhs.m_f);
		}

		R operator()(Arg arg) const {
			return m_f(arg);
		}

		R apply(Arg arg) const {
			return m_f(arg);
		}

		F getFunction() {
			return m_f;
		}

		Applyable<R, Arg>* newApplyable() {
			return new FunctionHolder(m_f);
		}

		CurryApplyable<R, Arg>* newCurryApplyable() {
			return new FunctionHolder(m_f);
		}
	};

	namespace Hidden {
		template<typename V, typename = void>
		class CurryType {
		public:
			inline static auto apply(V v) {
				return v;
			}
		};

		template<typename C>
		class CurryType<C, typename TMP::Convert<decltype(&C::operator()), void>::Eval> {
		public:
			inline static auto apply(C c) {
				return apply(c, &C::operator());
			}

			template<typename R>
			inline static auto apply(C c, R(C::*)() const) {
				return c();
			}

			template<typename R, typename Arg1, typename... Args>
			inline static auto apply(C c, R(C::*)(Arg1, Args...) const) {
				return FunctionHolder<C, false, void, R, Arg1, Args...>(c);
			}
		};

		template<typename R>
		class CurryType<const Applyable<R>*, void> {
		public:
			inline static auto apply(const Applyable<R> *f) {
				return f->apply();
			}
		};

		template<typename R, typename Arg1, typename... Args>
		class CurryType<const Applyable<R, Arg1, Args...>*, void> {
			inline static R apply_medthod(const Applyable<R, Arg1, Args...> *f, Arg1 arg1, Args... args) {
				return f->apply(arg1, args...);
			}
		public:
			inline static auto apply(const Applyable<R, Arg1, Args...> *f) {
				return FunctionHolder<R(*)(const Applyable<R, Arg1, Args...>*, Arg1, Args...), false, void, R, const Applyable<R, Arg1, Args...>*, Arg1, Args...>(apply_medthod)(f);
			}
		};

		template<typename R, typename... Args>
		class CurryType<Applyable<R, Args...>*, void> {
		public:
			inline static auto apply(Applyable<R, Args...> *f) {
				return CurryType<const Applyable<R, Args...>*>::apply(f);
			}
		};

		template<typename, typename>
		class CurryMedthod;

		template<typename C, typename R>
		class CurryMedthod<C, R(C::*)()> {
		public:
			inline static auto apply(C c, R(C::*f)()) {
				return (c.*f)();
			}
		};

		template<typename C, typename R, typename Arg1, typename... Args>
		class CurryMedthod<C, R(C::*)(Arg1, Args...)> {
			inline static R apply_medthod(C c, R(C::*f)(Arg1, Args...), Arg1 arg1, Args... args) {
				return (c.*f)(arg1, args...);
			}
		public:
			inline static auto apply(C c, R(C::*f)(Arg1, Args...)) {
				return FunctionHolder<R(*)(C, R(C::*)(Arg1, Args...), Arg1, Args...), false, void, R, C, R(C::*)(Arg1, Args...), Arg1, Args...>(apply_medthod)(c)(f);
			}
		};

		template<typename C, typename R>
		class CurryMedthod<C*, R(C::*)()> {
		public:
			inline static auto apply(C *c, R(C::*f)()) {
				return (c->*f)();
			}
		};

		template<typename C, typename R, typename Arg1, typename... Args>
		class CurryMedthod<C*, R(C::*)(Arg1, Args...)> {
			inline static R apply_medthod(C *c, R(C::*f)(Arg1, Args...), Arg1 arg1, Args... args) {
				return (c->*f)(arg1, args...);
			}
		public:
			inline static auto apply(C *c, R(C::*f)(Arg1, Args...)) {
				return FunctionHolder<R(*)(C*, R(C::*)(Arg1, Args...), Arg1, Args...), false, void, R, C*, R(C::*)(Arg1, Args...), Arg1, Args...>(apply_medthod)(c)(f);
			}
		};

		template<typename C, typename R>
		class CurryMedthod<C, R(C::*)() const> {
		public:
			inline static auto apply(C c, R(C::*f)() const) {
				return (c.*f)();
			}
		};

		template<typename C, typename R, typename Arg1, typename... Args>
		class CurryMedthod<C, R(C::*)(Arg1, Args...) const> {
			inline static R apply_medthod(C c, R(C::*f)(Arg1, Args...) const, Arg1 arg1, Args... args) {
				return (c.*f)(arg1, args...);
			}
		public:
			inline static auto apply(C c, R(C::*f)(Arg1, Args...) const) {
				return FunctionHolder<R(*)(C, R(C::*)(Arg1, Args...) const, Arg1, Args...), false, void, R, C, R(C::*)(Arg1, Args...) const, Arg1, Args...>(apply_medthod)(c)(f);
			}
		};

		template<typename C, typename R>
		class CurryMedthod<C*, R(C::*)() const> {
		public:
			inline static auto apply(C *c, R(C::*f)() const) {
				return (c->*f)();
			}
		};

		template<typename C, typename R, typename Arg1, typename... Args>
		class CurryMedthod<C*, R(C::*)(Arg1, Args...) const> {
			inline static R apply_medthod(C *c, R(C::*f)(Arg1, Args...) const, Arg1 arg1, Args... args) {
				return (c->*f)(arg1, args...);
			}
		public:
			inline static auto apply(C *c, R(C::*f)(Arg1, Args...) const) {
				return FunctionHolder<R(*)(C*, R(C::*)(Arg1, Args...) const, Arg1, Args...), false, void, R, C*, R(C::*)(Arg1, Args...) const, Arg1, Args...>(apply_medthod)(c)(f);
			}
		};

		template<typename C, typename R>
		class CurryMedthod<const C, R(C::*)() const> {
		public:
			inline static auto apply(C c, R(C::*f)() const) {
				return (c.*f)();
			}
		};

		template<typename C, typename R, typename Arg1, typename... Args>
		class CurryMedthod<const C, R(C::*)(Arg1, Args...) const> {
			inline static R apply_medthod(C c, R(C::*f)(Arg1, Args...) const, Arg1 arg1, Args... args) {
				return (c.*f)(arg1, args...);
			}
		public:
			inline static auto apply(C c, R(C::*f)(Arg1, Args...) const) {
				return FunctionHolder<R(*)(C, R(C::*)(Arg1, Args...) const, Arg1, Args...), false, void, R, C, R(C::*)(Arg1, Args...) const, Arg1, Args...>(apply_medthod)(c)(f);
			}
		};

		template<typename C, typename R>
		class CurryMedthod<const C*, R(C::*)() const> {
		public:
			inline static auto apply(const C *c, R(C::*f)() const) {
				return (c->*f)();
			}
		};

		template<typename C, typename R, typename Arg1, typename... Args>
		class CurryMedthod<const C*, R(C::*)(Arg1, Args...) const> {
			inline static R apply_medthod(const C *c, R(C::*f)(Arg1, Args...) const, Arg1 arg1, Args... args) {
				return (c->*f)(arg1, args...);
			}
		public:
			inline static auto apply(const C *c, R(C::*f)(Arg1, Args...) const) {
				return FunctionHolder<R(*)(const C*, R(C::*)(Arg1, Args...) const, Arg1, Args...), false, void, R, const C*, R(C::*)(Arg1, Args...) const, Arg1, Args...>(apply_medthod)(c)(f);
			}
		};
	}

	template<typename R>
	inline auto curry(R(*f)()) {
		return f();
	}

	template<typename R, typename Arg1, typename... Args>
	inline auto curry(R(*f)(Arg1, Args...)) {
		return FunctionHolder<R(*)(Arg1, Args...), false, void, R, Arg1, Args...>(f);
	}
	
	template<typename T>
	inline auto curry(T c) {
		return Hidden::CurryType<T>::apply(c);
	}

	template<typename F, bool B, typename D, typename R, typename... Args>
	inline auto curry(const FunctionHolder<F, B, D, R, Args...> f) {
		return f;
	}

	template<typename C, typename F>
	inline auto curry(C c, F f) {
		return Hidden::CurryMedthod<C, F>::apply(c, f);
	}

	template<typename A, typename B>
	inline auto compose(A f, B g) {
		return [f, g](auto x) { return f(g(x)); };
	}

	namespace Combinator {
		template<typename F, typename R, typename... Args>
		class Y : public Applyable<R, Args...> {
			F m_f;
		public:
			Y(F f) : m_f(f) {}
			Y(const Y &rhs) : m_f(rhs.m_f) {}
			Y(Y &&rhs) : Y(rhs) {}

			Y& operator=(const Y &rhs) {
				m_f = rhs.m_f;
			}

			Y& operator=(Y &&rhs) {
				m_f = static_cast<F&&>(rhs.m_f);
			}

			R operator()(Args... args) const {
				return m_f(this, args...);
			}

			R apply(Args... args) const {
				return m_f(this, args...);
			}
		};

		template<typename R, typename... Args>
		auto y(R(*f)(const Applyable<R, Args...>*, Args...)) {
			return Y<R(*)(const Applyable<R, Args...>*, Args...), R, Args...>(f);
		}
	}
}