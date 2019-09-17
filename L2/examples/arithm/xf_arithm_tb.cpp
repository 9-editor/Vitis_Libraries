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
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <XCLBIN File> <INPUT IMAGE PATH 1> <INPUT IMAGE PATH 2>" << std::endl;
        return EXIT_FAILURE;
    }

    cv::Mat in_img1, in_img2, in_gray1, in_gray2, out_img, ocv_ref, diff;

    // Reading in the image:
    in_gray1 = cv::imread(argv[2], 0);

    if (in_gray1.data == NULL) {
        std::cout << "ERROR: Cannot open image " << argv[2] << std::endl;
        return EXIT_FAILURE;
    }

#if ARRAY
    in_gray2 = cv::imread(argv[3], 0);

    if (in_gray2.data == NULL) {
        std::cout << "ERROR: Cannot open image " << argv[3] << std::endl;
        return EXIT_FAILURE;
    }
#endif

#if T_16S
    /*  convert to 16S type  */
    in_gray1.convertTo(in_gray1, CV_16SC1);
    in_gray2.convertTo(in_gray2, CV_16SC1);
#endif

    out_img.create(in_gray1.rows, in_gray1.cols, in_gray1.depth());
    ocv_ref.create(in_gray2.rows, in_gray1.cols, in_gray1.depth());
    diff.create(in_gray1.rows, in_gray1.cols, in_gray1.depth());

#ifdef FUNCT_MULTIPLY
    float scale = 0.05;
#endif

// OpenCL section:
#if T_16S
    size_t image_in_size_bytes = in_gray1.rows * in_gray1.cols * sizeof(short int);
    size_t image_out_size_bytes = in_gray1.rows * in_gray1.cols * sizeof(short int);
#else
    size_t image_in_size_bytes = in_gray1.rows * in_gray1.cols * sizeof(unsigned char);
    size_t image_out_size_bytes = in_gray1.rows * in_gray1.cols * sizeof(unsigned char);
#endif

#if SCALAR
    std::vector<unsigned char> scalar(in_gray1.channels());

    for (int i = 0; i < in_gray1.channels(); ++i) scalar[i] = 150;

    size_t vec_in_size_bytes = in_gray1.channels() * sizeof(unsigned char);
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
    OCL_CHECK(err, cl::Kernel kernel(program, "arithm", &err));

    // Allocate the buffers:
    OCL_CHECK(err, cl::Buffer buffer_inImage1(context, CL_MEM_READ_ONLY, image_in_size_bytes, NULL, &err));
#if SCALAR
    OCL_CHECK(err, cl::Buffer buffer_inVec(context, CL_MEM_READ_ONLY, vec_in_size_bytes, NULL, &err));
#else
    OCL_CHECK(err, cl::Buffer buffer_inImage2(context, CL_MEM_READ_ONLY, image_in_size_bytes, NULL, &err));
#endif
    OCL_CHECK(err, cl::Buffer buffer_outImage(context, CL_MEM_WRITE_ONLY, image_out_size_bytes, NULL, &err));

    // Set kernel arguments:
    OCL_CHECK(err, err = kernel.setArg(0, buffer_inImage1));
#if SCALAR
    OCL_CHECK(err, err = kernel.setArg(1, buffer_inVec));
#else
    OCL_CHECK(err, err = kernel.setArg(1, buffer_inImage2));
#endif
#ifdef FUNCT_MULTIPLY
    OCL_CHECK(err, err = kernel.setArg(2, scale));
    OCL_CHECK(err, err = kernel.setArg(3, buffer_outImage));
#else
    OCL_CHECK(err, err = kernel.setArg(2, buffer_outImage));
#endif

    // Initilize buffers:
    cl::Event event;

    OCL_CHECK(err, queue.enqueueWriteBuffer(buffer_inImage1,     // buffer on the FPGA
                                            CL_TRUE,             // blocking call
                                            0,                   // buffer offset in bytes
                                            image_in_size_bytes, // Size in bytes
                                            in_gray1.data,       // Pointer to the data to copy
                                            nullptr, &event));

#if SCALAR
    OCL_CHECK(err, queue.enqueueWriteBuffer(buffer_inVec,      // buffer on the FPGA
                                            CL_TRUE,           // blocking call
                                            0,                 // buffer offset in bytes
                                            vec_in_size_bytes, // Size in bytes
                                            scalar.data(),     // Pointer to the data to copy
                                            nullptr, &event));
#else
    OCL_CHECK(err, queue.enqueueWriteBuffer(buffer_inImage2,     // buffer on the FPGA
                                            CL_TRUE,             // blocking call
                                            0,                   // buffer offset in bytes
                                            image_in_size_bytes, // Size in bytes
                                            in_gray2.data,       // Pointer to the data to copy
                                            nullptr, &event));
#endif

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

    // Write down the kernel result:
    cv::imwrite("hls_out.jpg", out_img);

/* OpenCV reference function */
#if ARRAY
#if defined(FUNCT_BITWISENOT)
    cv::CV_FUNCT_NAME(in_gray1, ocv_ref);
#elif defined(FUNCT_ZERO)
    ocv_ref = cv::Mat::zeros(in_gray1.rows, in_gray1.cols, in_gray1.depth());
#else
    cv::CV_FUNCT_NAME(in_gray1, in_gray2, ocv_ref
#ifdef FUNCT_MULTIPLY
                      ,
                      scale
#endif
#ifdef FUNCT_COMPARE
                      ,
                      CV_EXTRA_PARM
#endif
    );
#endif
#endif

#if SCALAR
#if defined(FUNCT_SET)
    ocv_ref.setTo(cv::Scalar(scalar[0]));
#else
#ifdef FUNCT_SUBRS
    cv::CV_FUNCT_NAME(scalar[0], in_gray1, ocv_ref);
#else
    cv::CV_FUNCT_NAME(in_gray1, scalar[0], ocv_ref
#ifdef FUNCT_COMPARE
                      ,
                      CV_EXTRA_PARM
#endif
    );
#endif
#endif
#endif

    // Write down the OpenCV outputs:
    cv::imwrite("ref_img.jpg", ocv_ref);

    /* Results verification */
    // Do the diff and save it:
    cv::absdiff(ocv_ref, out_img, diff);
    cv::imwrite("diff_img.jpg", diff);

    // Find the percentage of pixels above error threshold:
    double minval = 256, maxval = 0;
    int cnt = 0;
    for (int i = 0; i < in_gray1.rows; i++) {
        for (int j = 0; j < in_gray1.cols; j++) {
            uchar v = diff.at<uchar>(i, j);

            if (v > 2) cnt++;
            if (minval > v) minval = v;
            if (maxval < v) maxval = v;
        }
    }

    float err_per = 100.0 * (float)cnt / (in_gray1.rows * in_gray1.cols);

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
