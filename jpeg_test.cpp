#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>

#include <jpeglib.h>
#include <opencv2/opencv.hpp>
#include <turbojpeg.h>


void ppm_save(const std::vector<uint8_t> &img_buffer, const int width, const int height, const std::string &path) {
    std::ofstream f;
    f.open(path.c_str(), std::ofstream::binary);
    // header
    f << "P6" << std::endl << width << " " << height << std::endl << 0xFF << std::endl;
    // data
    f.write((char*)img_buffer.data(), img_buffer.size());
    f.close();
}


int main(int argc, char **argv) {
    if(argc != 2) {
        std::cout << "provide jpeg image path" << std::endl;
        return EXIT_FAILURE;
    }

    std::ifstream file;
    file.open(argv[1], std::ios::binary);
    const std::vector<uint8_t> data(std::istreambuf_iterator<char>(file), {});
    file.close();

    std::cout << "file size: " << data.size() << std::endl;

    // jpeg turbo
    {
    const auto tstart = std::chrono::high_resolution_clock::now();
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    jpeg_mem_src(&cinfo, data.data(), data.size());

    int ret = jpeg_read_header(&cinfo, TRUE);
    if(ret!=1) {
        std::cerr<<"not a jpeg"<<std::endl;
        return EXIT_FAILURE;
    }

    jpeg_start_decompress(&cinfo);

    std::vector<uint8_t> img_buffer(cinfo.output_width*cinfo.output_height*cinfo.output_components);
    while(cinfo.output_scanline < cinfo.output_height) {
        unsigned char *buffer_array[1];
        buffer_array[0] = img_buffer.data() + (cinfo.output_scanline*cinfo.output_width*cinfo.output_components);
        jpeg_read_scanlines(&cinfo, buffer_array, 1);
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    const auto tend = std::chrono::high_resolution_clock::now();

    std::cout << "img size: " << img_buffer.size() << std::endl;
    std::cout << "img dim: " << cinfo.output_width << " x " << cinfo.output_height << " x " << cinfo.output_components << std::endl;
    std::cout << "JPEG time (ms): " << std::chrono::duration<float, std::milli>(tend - tstart).count() << std::endl;

    ppm_save(img_buffer, cinfo.output_width, cinfo.output_height, "/tmp/a.ppm");
    }

    // OpenCV
    {
    const auto tstart = std::chrono::high_resolution_clock::now();
    const cv::Mat img = cv::imdecode(data, cv::IMREAD_UNCHANGED);
    const auto tend = std::chrono::high_resolution_clock::now();
    std::cout << "img size: " << std::distance(img.datastart, img.dataend) << std::endl;
    std::cout << "img dim: " << img.size() << " x " << img.channels() << std::endl;
    std::cout << "OpenCV time (ms): " << std::chrono::duration<float, std::milli>(tend - tstart).count() << std::endl;

    const std::vector<uint8_t> data(img.begin<uint8_t>(), img.end<uint8_t>());
    ppm_save(data, img.cols, img.rows, "/tmp/b.ppm");
    }

    // TurboJPEG
    {
    const auto tstart = std::chrono::high_resolution_clock::now();

    tjhandle _jpegDecompressor = tjInitDecompress();

    int width, height;
    tjDecompressHeader(_jpegDecompressor, const_cast<uint8_t *>(data.data()), data.size(), &width, &height);

    std::vector<uint8_t> img_buffer(width*height*3);
    tjDecompress2(_jpegDecompressor, data.data(), data.size(), img_buffer.data(), width, 0, height, TJPF_RGB, TJFLAG_FASTDCT);
    tjDestroy(_jpegDecompressor);

    const auto tend = std::chrono::high_resolution_clock::now();

    std::cout << "img size: " << img_buffer.size() << std::endl;
    std::cout << "img dim: " << width << " x " << height << " x " << 3 << std::endl;
    std::cout << "TurboJPEG time (ms): " << std::chrono::duration<float, std::milli>(tend - tstart).count() << std::endl;

    ppm_save(img_buffer, width, height, "/tmp/c.ppm");
    }

    return EXIT_SUCCESS;
}
