/*
 * Copyright 2019 Xilinx, Inc.
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
#define XF_CV_DEPTH_IN 3
#define XF_CV_DEPTH_OUT_0 3
#define XF_CV_DEPTH_OUT_1 3

#define MPC 0 // Multiple Pixels per Clock operation
#define SPC 1 // Single Pixel per Clock operation

/*  Set Filter size  */

#define FILTER_SIZE_3 0
#define FILTER_SIZE_5 1
#define FILTER_SIZE_7 0

#define GRAY 0
#define RGBA 1

#define INPUT_PTR_WIDTH 256
#define OUTPUT_PTR_WIDTH 256

#define XF_USE_URAM true
