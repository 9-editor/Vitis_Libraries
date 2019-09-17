/***************************************************************************
Copyright (c) 2016, Xilinx, Inc.
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

#include "xf_canny_config.h"

extern "C" {
void edgetracing_accel(ap_uint<INPUT_PTR_WIDTH>* img_inp, ap_uint<OUTPUT_PTR_WIDTH>* img_out, int rows, int cols) {
    #pragma HLS INTERFACE m_axi     port=img_inp  offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi     port=img_out  offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=img_inp  bundle=control
    #pragma HLS INTERFACE s_axilite port=img_out  bundle=control

    #pragma HLS INTERFACE s_axilite port=rows     bundle=control
    #pragma HLS INTERFACE s_axilite port=cols     bundle=control
    #pragma HLS INTERFACE s_axilite port=return   bundle=control

    xf::cv::Mat<XF_2UC1, HEIGHT, WIDTH, XF_NPPC32> _dst1(rows, cols, img_inp);
    xf::cv::Mat<XF_8UC1, HEIGHT, WIDTH, XF_NPPC8> _dst2(rows, cols, img_out);

    xf::cv::EdgeTracing<XF_2UC1, XF_8UC1, HEIGHT, WIDTH, XF_NPPC32, XF_NPPC8, XF_USE_URAM>(_dst1, _dst2);
}
}
