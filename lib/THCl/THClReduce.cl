// Threads per thread block
#define THCL_NONCONTIG_REDUCE_BLOCK_SIZE 32 * 16

{{include_THClReduceApplyUtils}}

{{index_type}} getReduceNoncontigDimSliceIndex() {
  // Each thread handles one slice
  return getLinearBlockId<{{index_type}}>() * THCL_NONCONTIG_REDUCE_BLOCK_SIZE + threadIdx.x;
}

float modifyOp(float _in1) {
  float _out;
  float *in1 = &_in1;
  float *out = &_out;
  {{modify_operation}};
  return _out;
}

float reduceOp(float _in1, float _in2) {
  // I guess the compiler can sort this stuff out :-P
  float _out;
  float *in1 = &_in1;
  float *in2 = &_in2;
  float *out = &_out;
  {{reduce_operation}};
  return _out;
}

// Kernel that handles an entire reduction of a slice of a tensor per each thread
kernel void
THClTensor_reduceNoncontigDim(TensorInfoCl<{{index_type}}> out,
                              global float *out_data,
                              TensorInfoCl<{{index_type}}> in,
                              global float *in_data,
                              {{index_type}} reductionStride,
                              {{index_type}} reductionSize,
                              {{index_type}} totalSlices,
                              float init) {
  const {{index_type}} sliceIndex = getReduceNoncontigDimSliceIndex<{{index_type}}>();

  if (sliceIndex >= totalSlices) {
    return;
  }

  // Each thread picks a point in `out` and `in` for which it is
  // producing the reduction
  const {{index_type}} outOffset =
    IndexToOffset_{{1000 + dim1}}_get(sliceIndex, out);
  const {{index_type}} inBaseOffset =
    IndexToOffset_{{1000 + dim2}}_get(sliceIndex, in);

  // For each point in reductionSize, reduce into `r`
  {{index_type}} inOffset = inBaseOffset;

  float r = modifyOp(in_data[0]);
  inOffset += reductionStride;

  for ({{index_type}} i = 1; i < reductionSize; ++i) {
    r = reduceOp(r, modifyOp(in_data[inOffset]));
    inOffsets += reductionStride;
  }

  // Write out reduced value
  out.data[outOffset] = r;
}

{{index_type}} getReduceContigDimSliceIndex() {
  // Each block handles one slice
  return getLinearBlockId<{{index_type}}>();
}

// Kernel that handles an entire reduction of a slice of a tensor per
// each block
kernel void
THClTensor_reduceContigDim(TensorInfoCl<{{index_type}}> out,
                           global float *out_data,
                           TensorInfoCl<{{index_type}}> in,
                           global float *in_data,
                           {{index_type}} reductionSize,
                           {{index_type}} totalSlices,
                           float init,
                           local float *smem) {
  const {{index_type}} sliceIndex = getReduceContigDimSliceIndex<{{index_type}}>();

  if (sliceIndex >= totalSlices) {
    return;
  }

  // Get the offset in `out` for the reduction
  const {{index_type}} outOffset =
    IndexToOffset_{{1000 + dim1}}_get(sliceIndex, out);

  // Get the base offset in `in` for this block's reduction
  const {{index_type}} inBaseOffset =
    IndexToOffset_{{1000 + dim2}}_get(sliceIndex, in);

  // Each thread in the block will reduce some subset of elements in
  // the slice. The elements are guaranteed contiguous starting at
  // `inBaseOffset`.
  float r = init;
  for ({{index_type}} i = threadIdx.x; i < reductionSize; i += blockDim.x) {
    r = reduceOp(r, modifyOp(in_data[inBaseOffset + i]));
  }

  // Reduce within the block
//  extern __shared__ float smem[];
  smem[threadIdx.x] = r;

  // First warp will perform reductions across warps
  //__syncthreads();
  barrier(CLK_LOCAL_MEM_FENCE);
  if ((threadIdx.x / 32) == 0) {
    float r = {{init}};
    for (int i = 0; i < blockDim.x; i += 32) {
      r = reduceOp(r, smem[i + threadIdx.x]);
    }

    // Each lane participating writes out a value
    smem[threadIdx.x] = r;
  }

  // First thread will perform reductions across the block
//  __syncthreads();
  barrier(CLK_LOCAL_MEM_FENCE);
  if (threadIdx.x == 0) {
    r = init;
    #pragma unroll
    {% for i=0,31 do %}
      r = reduceOp(r, smem[i]);
    {% end %}

    // Write out reduced value
    out_data[outOffset] = r;
  }
}

