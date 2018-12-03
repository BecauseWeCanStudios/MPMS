#pragma once
#include <cmath>
#include <array>
#include <queue>
#include <mutex>
#include <vector>
#include <thread>
#include <utility>
#include <cstddef>
#include <iostream>
#include <functional>

namespace mtms {

	template<typename T>
	class mandelbrot_set {

	public:

		struct color_t {

			T r, g, b;

		};

		using vector1d_t = std::vector<color_t>;
		using vector2d_t = std::vector<vector1d_t>;

		mandelbrot_set(const size_t width, const size_t height) : 
			_width(width), _height(height), _data(vector2d_t(height, vector1d_t(width))) {}

		const auto& data() const {
			return _data;
		}

		void run(const size_t n_workers, const size_t n_width, const size_t n_height,
		         const T& scale, const std::pair<T, T>& shift, const size_t max_n) {
			const auto w = mesh(n_width + 1, _width);
			const auto h = mesh(n_height + 1, _height);
			for (size_t i = 0; i < n_height; ++i)
				for (size_t j = 0; j < n_width; ++j)
					_queue.push({w[j], h[i], w[j + 1], h[i + 1]});
			const auto w_func = [hw=T(_width) / 2, &scale, &shift](const T& value) { return shift.first + (value - hw) * scale; };
			const auto h_func = [hh=T(_height) / 2, &scale, &shift](const T& value) { return shift.second + (value - hh) * scale; };
			std::vector<std::thread> threads; 
			threads.reserve(n_workers);
			for (size_t i = 0; i < n_workers; ++i)
				threads.emplace_back([this, &w_func, &h_func, &max_n]() { worker(w_func, h_func, max_n); });
			for (auto& it : threads)			
				it.join();
		}

		static auto mesh(const size_t n, const size_t last) {
			const auto d = last / (n - 1);
			std::vector<size_t> result(n);
			for (size_t i = 0; i < n; ++i)
				result[i] = i * d;
			result.back() = last;
			return result;
		}

		template<typename FW, typename FH>
		void worker(const FW& w_func, const FH& h_func, const size_t max_n) {
			while (true) {
				_mutex.lock();
				if (_queue.empty()) {
					_mutex.unlock();
					return;
				}
				const auto p = _queue.front(); _queue.pop();
				_mutex.unlock();
				for (size_t i = p.top.y; i < p.bottom.y; ++i)
					for (size_t j = p.top.x; j < p.bottom.x; ++j) {
						auto pair = std::make_pair(w_func(j), h_func(i));
						const auto c = pair;
						size_t n = 0;
						while (modulus(pair) < 4 && n <= max_n) {
                            pair = product(pair, c);
                            ++n;
                        }
						T value = 0;
						if (n <= max_n) {
                            auto log = std::log(modulus(pair)) / 2;
                            value = std::log(log / std::log(2)) / std::log(2) + n - 1;
                        }
						_data[i][j] = palette(value, max_n);
                    }
			}
		}

		template<typename P>
		static auto product(const P& pair, const P& c) {
			return std::make_pair(
				std::pow(pair.first, 2) - std::pow(pair.second, 2) + c.first, 
				2 * pair.first * pair.second + c.second
			);
		}

		template<typename P>
		static auto modulus(const P& pair){
			return std::pow(pair.first, 2) + std::pow(pair.second, 2);
		}

		static color_t palette(T n, const size_t max_n) {
			n = 380 + n * 400 / max_n;
			color_t color{0, 0, 0};
			if (n >= 380 && n <= 440) {
				color.r = (440 - n) / 60;
				color.b = 1;
			} else if (n >= 440 && n <= 490) {
				color.g = (n - 440) / 50;
				color.b = 1;
			} else if (n >= 490 && n <= 510) {
				color.g = 1;
				color.b = (510 - n) / 20;
			} else if (n >= 510 && n <= 580) {
				color.r = (n - 510) / 70;
				color.g = 1;
			} else if (n >= 580 && n <= 645) {
				color.r = 1;
				color.g = (645 - n) / 65;
			} else if (n >= 645 && n <= 780)
				color.r = 1;
			T s = 1;
			if (n >= 700)
				s = 0.3 + 0.7 * (780 - n) / 80;
			else if (n <= 420 && n >= 380)
				s = 0.3 + 0.7 * (n - 380) / 40;
			return {std::pow(color.r * s, 0.8), std::pow(color.g * s, 0.8), std::pow(color.b * s, 0.8)};
		}

	private:

		union part {

			std::array<size_t, 4> data;

			struct {

				size_t x1, y1, x2, y2;

			};

			struct {

                struct {

                    size_t x, y;

                } top, bottom;
            };

		};

		const size_t _width, _height;
		vector2d_t _data;
		std::mutex _mutex;
		std::queue<part> _queue;

	};

}// namespace mtms
