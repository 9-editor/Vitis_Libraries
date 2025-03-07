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

#define MPC 1 // Multiple Pixels per Clock operation
#define SPC 0 // Single Pixel per Clock operation

#define INPUT_PTR_WIDTH 64
#define OUTPUT_PTR_WIDTH 64

#define GRAY 1

#define FILTER_SIZE 5

#define KERNEL_SHAPE XF_SHAPE_CROSS

#define ITERATIONS 1

#define XF_CV_DEPTH_IN_1 1
#define XF_CV_DEPTH_OUT_1 3
