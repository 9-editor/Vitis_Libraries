/***************************************************************************
Copyright (c) 2018, Xilinx, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/
#include "common/xf_headers.h"
#include "xcl2.hpp"

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <XCLBIN File> <INPUT IMAGE PATH 1>" << std::endl;
        return EXIT_FAILURE;
    }

    cv::Mat in_img, out_img, ocv_ref, diff, filter;

    // Reading in the gray image:
    in_img = cv::imread(argv[2], 0);

    if (in_img.data == NULL) {
        std::cout << "ERROR: Cannot open image " << argv[2] << std::endl;
        return EXIT_FAILURE;
    }

    unsigned char shift = SHIFT;

    // Creating the kernel:
    filter.create(FILTER_HEIGHT, FILTER_WIDTH, CV_32F);

    // Filling the Filter coefficients:
    for (int i = 0; i < FILTER_HEIGHT; i++) {
        for (int j = 0; j < FILTER_WIDTH; j++) {
            filter.at<float>(i, j) = (float)0.1111;
        }
    }

    // OpenCV reference:
#if OUT_8U
    out_img.create(in_img.rows, in_img.cols, CV_8U); // create memory for output image
    diff.create(in_img.rows, in_img.cols, CV_8U);    // create memory for difference image
#elif OUT_16S
    out_img.create(in_img.rows, in_img.cols, CV_16S); // create memory for output image
    diff.create(in_img.rows, in_img.cols, CV_16S);    // create memory for difference image
#endif

    cv::Point anchor = cv::Point(-1, -1);

#if OUT_8U
    cv::filter2D(in_img, ocv_ref, CV_8U, filter, anchor, 0, cv::BORDER_CONSTANT);
#elif OUT_16S
    cv::filter2D(in_img, ocv_ref, CV_16S, filter, anchor, 0, cv::BORDER_CONSTANT);
#endif

    cv::imwrite("ref_img.jpg", ocv_ref); // reference image

    std::vector<short int> filter_vec(FILTER_WIDTH * FILTER_HEIGHT);

    for (int i = 0; i < FILTER_HEIGHT; i++) {
        for (int j = 0; j < FILTER_WIDTH; j++) {
            filter_vec[i * FILTER_WIDTH + j] = 3640;
        }
    }

    // OpenCL section:
    size_t image_in_size_bytes = in_img.rows * in_img.cols * sizeof(unsigned char);
    size_t vec_in_size_bytes = filter_vec.size() * sizeof(short int);
#if OUT_8U == 1
    size_t image_out_size_bytes = image_in_size_bytes;
#else
    size_t image_out_size_bytes = in_img.rows * in_img.cols * sizeof(short int);
#endif

    cl_int err;
    std::cout << "INFO: Running OpenCL section." << std::endl;

    // Get the device:
    std::vector<cl::Device> devices = xcl::get_xil_devices();
    cl::Device device = devices[0];

    // Context, command queue and device name:
    OCL_CHECK(err, cl::Context context(device, NULL, NULL, NULL, &err));
    OCL_CHECK(err, cl::CommandQueue queue(context, device, CL_QUEUE_PROFILING_ENABLE, &err));
    OCL_CHECK(err, std::string device_name = device.getInfo<CL_DEVICE_NAME>(&err));

    std::cout << "INFO: Device found - " << device_name << std::endl;

    // Load binary:
    unsigned fileBufSize;
    std::string binaryFile = argv[1];
    char* fileBuf = xcl::read_binary_file(binaryFile, fileBufSize);
    cl::Program::Binaries bins{{fileBuf, fileBufSize}};
    devices.resize(1);
    OCL_CHECK(err, cl::Program program(context, devices, bins, NULL, &err));

    // Create a kernel:
    OCL_CHECK(err, cl::Kernel kernel(program, "customconv", &err));

    // Allocate the buffers:
    OCL_CHECK(err, cl::Buffer buffer_inImage(context, CL_MEM_READ_ONLY, image_in_size_bytes, NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_inFilter(context, CL_MEM_READ_ONLY, vec_in_size_bytes, NULL, &err));
    OCL_CHECK(err, cl::Buffer buffer_outImage(context, CL_MEM_WRITE_ONLY, image_out_size_bytes, NULL, &err));

    // Set kernel arguments:
    OCL_CHECK(err, err = kernel.setArg(0, buffer_inImage));
    OCL_CHECK(err, err = kernel.setArg(1, buffer_inFilter));
    OCL_CHECK(err, err = kernel.setArg(2, shift));
    OCL_CHECK(err, err = kernel.setArg(3, buffer_outImage));

    // Initialize the buffers:
    cl::Event event;

    OCL_CHECK(err, queue.enqueueWriteBuffer(buffer_inImage,      // buffer on the FPGA
                                            CL_TRUE,             // blocking call
                                            0,                   // buffer offset in bytes
                                            image_in_size_bytes, // Size in bytes
                                            in_img.data,         // Pointer to the data to copy
                                            nullptr, &event));

    OCL_CHECK(err, queue.enqueueWriteBuffer(buffer_inFilter,   // buffer on the FPGA
                                            CL_TRUE,           // blocking call
                                            0,                 // buffer offset in bytes
                                            vec_in_size_bytes, // Size in bytes
                                            filter_vec.data(), // Pointer to the data to copy
                                            nullptr, &event));

    // Execute the kernel:
    OCL_CHECK(err, err = queue.enqueueTask(kernel));

    // Copy Result from Device Global Memory to Host Local Memory
    queue.enqueueReadBuffer(buffer_outImage, // This buffers data will be read
                            CL_TRUE,         // blocking call
                            0,               // offset
                            image_out_size_bytes,
                            out_img.data, // Data will be stored here
                            nullptr, &event);

    // Clean up:
    queue.finish();

    // Save the kernel result:
    cv::imwrite("out_img.jpg", out_img);

    // Results verification:
    cv::absdiff(ocv_ref, out_img, diff); // Compute absolute difference image
    cv::imwrite("diff_img.jpg", diff);   // Save the difference image for debugging purpose

    double minval = 256, maxval = 0;
    int cnt = 0;
    for (int i = 0; i < in_img.rows; i++) {
        for (int j = 0; j < in_img.cols; j++) {
#if OUT_8U == 1
            unsigned char v = diff.at<unsigned char>(i, j);
#elif OUT_16S == 1
            short int v = diff.at<short int>(i, j);
#endif
            if (v > 2) cnt++;
            if (minval > v) minval = v;
            if (maxval < v) maxval = v;
        }
    }

    float err_per = 100.0 * (float)cnt / (in_img.rows * in_img.cols);

    std::cout << "INFO: Verification results:" << std::endl;
    std::cout << "\tMinimum error in intensity = " << minval << std::endl;
    std::cout << "\tMaximum error in intensity = " << maxval << std::endl;
    std::cout << "\tPercentage of pixels above error threshold = " << err_per << std::endl;

    if (err_per > 0.0f) {
        std::cout << "ERROR: Test Failed." << std::endl;
        return EXIT_FAILURE;
    }

    return 0;
}
