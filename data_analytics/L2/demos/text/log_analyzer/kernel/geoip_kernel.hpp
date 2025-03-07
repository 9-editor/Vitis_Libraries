/*
 * Copyright 2022 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _XF_GEOIP_PLUS_KERNEL_HPP_
#define _XF_GEOIP_PLUS_KERNEL_HPP_

#include "xf_data_analytics/text/geoip.hpp"
#include <ap_int.h>
#include <hls_stream.h>

#define TH1 16
#define TH2 4
#define Bank1 32
#define Bank2 24
#define NIP 1024 * 128

typedef ap_uint<16> uint16;
typedef ap_uint<32> uint32;
typedef ap_uint<64> uint64;
typedef ap_uint<512> uint512;

extern "C" void GeoIP_kernel(unsigned int ipPos,
                             uint64* msg,
                             uint16* msgLen,
                             uint32* offsetIP,
                             uint64* netHigh16,
                             uint512* netLow21,
                             uint512* net2Low21,
                             uint32* netID);
#endif
