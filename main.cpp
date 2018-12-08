#include <chrono>
#include <string>
#include <iostream>
#include <algorithm>
#include <pngwriter.h>
#include <boost/program_options.hpp>
#include "mandelbrot.hpp"

namespace po = boost::program_options;

int main(int argc, char* argv[]) {
    std::string out_file;
    double scale, cx, cy;
    size_t max_n, width, height, n_colors, n_threads, nx, ny;
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "Print this message")
            ("out,o", po::value(&out_file)->default_value("result.png"), "Output file")
            ("scale,s", po::value(&scale)->default_value(1), "Scale")
            ("cx", po::value(&cx)->default_value(-0.28676842048), "X center coordinate")
            ("cy", po::value(&cy)->default_value(0), "Y center coordinate")
            ("iter,i", po::value(&max_n)->default_value(5000), "Maximum number of iterations")
            ("width", po::value(&width)->default_value(1920), "Image width")
            ("height", po::value(&height)->default_value(1080), "Image height")
            ("colors", po::value(&n_colors)->default_value(10), "Number of colors")
            ("threads,t", po::value(&n_threads)->default_value(1), "Number of threads")
            ("nx", po::value(&nx)->default_value(16), "Split into nx columns")
            ("ny", po::value(&ny)->default_value(16), "Split into ny rows");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }
    auto m = mtms::mandelbrot_set<double>(width, height, n_colors);
    const auto start = std::chrono::high_resolution_clock::now();
    m.run(n_threads, nx, ny, scale * std::max(3. / width, 2. / height), {cx, cy}, max_n);
    const auto run_end = std::chrono::high_resolution_clock::now();
    pngwriter png(static_cast<int>(width), static_cast<int>(height), 0, out_file.c_str());
    for (size_t i = 0; i < height; ++i)
        for (size_t j = 0; j < width; ++j) {
            const auto color = m.color(i, j);
            png.plot(static_cast<int>(j), static_cast<int>(i), color.r, color.g, color.b);
        }
    png.close();
    const auto write_end = std::chrono::high_resolution_clock::now();
    std::cout << "Run time: "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(run_end - start).count() << " ns\n"
              << "Write time : "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(write_end - run_end).count() << " ns\n"
              << "All time: "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(write_end - start).count() << " ns" << std::endl;
    return 0;
}