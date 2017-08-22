#pragma once

template<typename T, typename... Ts>
void print(T t, Ts... ts) {
	std::cout << t;
	print(ts...);
}

template<typename T = char*>
void print(T t = "") {
	std::cout << t;
}

template<typename T, typename... Ts>
void println(T t, Ts... ts) {
	std::cout << t;
	println(ts...);
}

template<typename T = char*>
void println(T t = "") {
	std::cout << t << std::endl;
}