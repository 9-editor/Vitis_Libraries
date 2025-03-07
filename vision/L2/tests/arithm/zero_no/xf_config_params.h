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

#define T_8U 1  // Input type of 8U
#define T_16S 0 // Input type of 16S

/*  set the optimisation type  */
#define SPC 1 // Single Pixel per Clock operation
#define MPC 0 // Multiple Pixels per Clock operation

#define GRAY 1

#define ARRAY 1
#define SCALAR 0
// macros for accel
#define FUNCT_NUM 14
//#define EXTRA_ARG 0.05
#define EXTRA_PARM XF_CONVERT_POLICY_SATURATE

// OpenCV reference macros
#define CV_FUNCT_NAME zero
//#define CV_EXTRA_ARG 0.05

#define XF_CV_DEPTH_IN_1 1
#define XF_CV_DEPTH_IN_2 2
#define XF_CV_DEPTH_OUT_1 3
