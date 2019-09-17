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

#include "xf_scharr_config.h"

extern "C" {
void scharr_accel(ap_uint<INPUT_PTR_WIDTH>* img_inp,
                  ap_uint<OUTPUT_PTR_WIDTH>* img_out1,
                  ap_uint<OUTPUT_PTR_WIDTH>* img_out2,
                  int rows,
                  int cols) {
    #pragma HLS INTERFACE m_axi     port=img_inp  offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi     port=img_out1  offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi     port=img_out2  offset=slave bundle=gmem3

    #pragma HLS INTERFACE s_axilite port=img_inp  bundle=control
    #pragma HLS INTERFACE s_axilite port=img_out1  bundle=control
    #pragma HLS INTERFACE s_axilite port=img_out2  bundle=control
    #pragma HLS INTERFACE s_axilite port=rows     bundle=control
    #pragma HLS INTERFACE s_axilite port=cols     bundle=control
    #pragma HLS INTERFACE s_axilite port=return   bundle=control

    const int pROWS = HEIGHT;
    const int pCOLS = WIDTH;
    const int pNPC1 = NPC1;

    xf::cv::Mat<IN_TYPE, HEIGHT, WIDTH, NPC1> in_mat;
    #pragma HLS stream variable=in_mat.data depth=pCOLS/pNPC1
    in_mat.rows = rows;
    in_mat.cols = cols;

    xf::cv::Mat<OUT_TYPE, HEIGHT, WIDTH, NPC1> _dstgx;
    #pragma HLS stream variable=_dstgx.data depth=pCOLS/pNPC1
    _dstgx.rows = rows;
    _dstgx.cols = cols;

    xf::cv::Mat<OUT_TYPE, HEIGHT, WIDTH, NPC1> _dstgy;
    #pragma HLS stream variable=_dstgy.data depth=pCOLS/pNPC1
    _dstgy.rows = rows;
    _dstgy.cols = cols;

    #pragma HLS DATAFLOW

    xf::cv::Array2xfMat<INPUT_PTR_WIDTH, IN_TYPE, HEIGHT, WIDTH, NPC1>(img_inp, in_mat);

    xf::cv::Scharr<XF_BORDER_CONSTANT, IN_TYPE, OUT_TYPE, HEIGHT, WIDTH, NPC1>(in_mat, _dstgx, _dstgy);

    xf::cv::xfMat2Array<OUTPUT_PTR_WIDTH, OUT_TYPE, HEIGHT, WIDTH, NPC1>(_dstgx, img_out1);
    xf::cv::xfMat2Array<OUTPUT_PTR_WIDTH, OUT_TYPE, HEIGHT, WIDTH, NPC1>(_dstgy, img_out2);
}
}