/**
 * Mandlebort implementation using OpenCL
 *
 * Copyright (c) 2012, Moritz Wundke
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the owner nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Moritz Wundke BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
 
__kernel void mandel_kernel (
  __global char * mandleset,
  const int width,
  const int height,
  const float scale,
  const float offsetX,
  const float offsetY,
  const int iterations
  )
{
    int tid = get_global_id(0);
   
    int i = tid%width;
    int j = tid/height;
   
    float x0 = ((i*scale) - ((scale/2)*width))/width + offsetX;
    float y0 = ((j*scale) - ((scale/2)*height))/height + offsetY;
   
    float x = x0;
    float y = y0;
   
    float x2 = x*x;
    float y2 = y*y;
   
    float scaleSquare = scale * scale;
   
    uint iter=0;
    for(iter=0; (x2+y2 <= scaleSquare) && (iter < iterations); ++iter)
    {
        y = 2 * x * y + y0;
        x = x2 - y2   + x0;
       
        x2 = x*x;
        y2 = y*y;
    }
    if ( iter == iterations)
  mandleset[tid] = 1;
    else
      mandleset[tid] = 0; 
}
