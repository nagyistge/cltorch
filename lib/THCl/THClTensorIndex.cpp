#include "THClTensorMath.h"
#include "THClGeneral.h"
//#include "THClBlas.h"
#include "THClTensorCopy.h"
//#include "THClTensorRandom.h"
#include "THClApply.h"
#include "THClReduce.h"

#include <string>
using namespace std;

#ifndef DIVUP
#define DIVUP(x, y) (((x) + (y) - 1) / (y))
#endif

std::string THClTensorIndex_getKernelTemplate();

void THClTensor_indexCopy(THClState *state, THClTensor *res_, int dim, THLongTensor *indices, THClTensor *src)
{
  THError("Not implemented");
//  THAssert(THClTensor_checkGPU(state, 2, res_, src));
//  THClTensor *indices_;
//  long *stride_;
//  long nIndex = indices->size[0];
//  long nRes;

//  THArgCheck(indices->nDimension == 1, 3, "expecting vector of indices");
//  THArgCheck(dim < src->nDimension, 4, "Indexing dim is out of bounds");
//  THArgCheck(src->nDimension > 0, 2, "Source tensor is empty");
//  THArgCheck(nIndex == src->size[dim], 4, "length of src.size[dim] is not equal to length of indices");

//  src = THClTensor_newContiguous(state, src);
//  indices_ = THClTensor_newWithSize1d(state, nIndex);
//  THClTensor_copyLong(state, indices_, indices);

//  nRes = THClTensor_nElement(state, res_);
//  dim3 nthreads(16, 16);
//  dim3 nblocks(ceil((float)nRes / nIndex / (16*16)));

//  THClCheck(cudaMalloc((void**)&stride_, res_->nDimension * sizeof(long)));
//  THClCheck(cudaMemcpy(stride_, res_->stride, res_->nDimension * sizeof(long), cudaMemcpyHostToDevice));

//  THClTensor_kernel_indexCopy<<<nblocks, nthreads, 0, THClState_getCurrentStream(state)>>>(
//    THClTensor_data(state, res_), THClTensor_data(state, src),
//    stride_, THClTensor_data(state, indices_),
//    res_->nDimension, dim, nIndex,
//    THClTensor_nElement(state, src), res_->size[dim]
//  );

//  THClCheck(cudaFree(stride_));
//  THClTensor_free(state, indices_);
//  THClTensor_free(state, src);
}


void THClTensor_indexFill(THClState *state, THClTensor *res_, int dim, THLongTensor *indices, float val)
{
  THAssert(THClTensor_checkGPU(state, 1, res_));
  THClTensor *indices_;
  int *stride_;
  long nIndex = indices->size[0];
  long nRes;

  THArgCheck(indices->nDimension == 1, 3, "Index is supposed to be a vector");
  THArgCheck(dim < res_->nDimension,4,"Indexing dim is out of bounds");
  THArgCheck(res_->nDimension > 0, 2, "Source tensor is empty");

  indices_ = THClTensor_newWithSize1d(state, nIndex);
  THClTensor_copyLong(state, indices_, indices);

  nRes = THClTensor_nElement(state, res_) / res_->size[dim] * nIndex;

  dim3 nthreads(16, 16);
  dim3 nblocks(ceil((float)nRes / nIndex / (16*16)));

  stride_ = new int[res_->nDimension];
  CLWrapper *strideWrapper = THClState_getCl(state)->wrap(res_->nDimension, stride_);
  for(int i = 0; i < res_->nDimension; i++ ) {
    stride_[i] = res_->stride[i];
  }
  strideWrapper->copyToDevice();

  // launch kernel here....
  TemplatedKernel kernelBuilder(THClState_getCl(state));

  std::string uniqueName = "THClTensorMathIndex";
  CLKernel *kernel = kernelBuilder.buildKernel(uniqueName, "THClTensorIndex.cl",
    THClTensorIndex_getKernelTemplate(), "THClTensor_kernel_indexFill");
  // calculate workgroup sizes and stuff
  dim3 global_ws;
  for( int i = 0; i < 3; i++ ) {
      global_ws.vec[i] = nblocks.vec[i] * nthreads.vec[i];
  }

  kernel->inout(THClTensor_wrapper(state, res_));
  kernel->in((int)THClTensor_storageOffset(state, res_));
  kernel->in(strideWrapper);
  kernel->in(THClTensor_wrapper(state, indices_)),
  kernel->in((int)THClTensor_storageOffset(state, indices_)),
  kernel->in((int)(res_->nDimension));
  kernel->in((int)dim);
  kernel->in((int)nIndex);
  kernel->in((int)nRes);
  kernel->in((int)(res_->size[dim]));
  kernel->in(val);

  kernel->run(3, global_ws.as_size_t(), nthreads.as_size_t());
  THClState_getCl(state)->finish();

  delete strideWrapper;
  delete[] stride_;
  THClTensor_free(state, indices_);
}

void THClTensor_indexSelect(THClState *state, THClTensor *res_, THClTensor *src, int dim, THLongTensor *indices)
{
  THError("Not implemented");
//  THAssert(THClTensor_checkGPU(state, 2, res_, src));
//  THLongStorage *newSize;
//  THClTensor *indices_;
//  long *stride_;
//  long nIndex = indices->size[0];
//  long nRes;

//  THArgCheck(indices->nDimension == 1, 3, "expecting vector of indices");
//  THArgCheck(dim < src->nDimension, 4, "Indexing dim is out of bounds");
//  THArgCheck(src->nDimension > 0, 2, "Source tensor is empty");

//  newSize = THLongStorage_newWithSize(src->nDimension);
//  THLongStorage_rawCopy(newSize, src->size);
//  newSize->data[dim] = nIndex;
//  THClTensor_resize(state, res_, newSize, NULL);
//  THLongStorage_free(newSize);

//  indices_ = THClTensor_newWithSize1d(state, nIndex);
//  THClTensor_copyLong(state, indices_, indices);

//  nRes = THClTensor_nElement(state, res_);
//  dim3 nthreads(16, 16);
//  dim3 nblocks(ceil((float)nRes / nIndex / (16*16)));

//  THClCheck(cudaMalloc((void**)&stride_, src->nDimension * sizeof(long)));
//  THClCheck(cudaMemcpy(stride_, src->stride, src->nDimension * sizeof(long), cudaMemcpyHostToDevice));

//  THClTensor_kernel_indexSelect<<<nblocks, nthreads, 0, THClState_getCurrentStream(state)>>>(
//    THClTensor_data(state, res_), THClTensor_data(state, src),
//    stride_, THClTensor_data(state, indices_),
//    src->nDimension, dim, indices->size[0], nRes, src->size[dim]
//  );

//  THClCheck(cudaFree(stride_));
//  THClTensor_free(state, indices_);
}

std::string THClTensorIndex_getKernelTemplate() {
  // [[[cog
  // import stringify
  // stringify.write_kernel( "kernel", "THClTensorIndex.cl" )
  // ]]]
  // generated using cog, from THClTensorIndex.cl:
  const char * kernelSource =  
  "// from lib/THC/THCTensorIndex.cu:\n" 
  "\n" 
  "kernel void THClTensor_kernel_indexFill(\n" 
  "   global float *tensor_data, int tensor_offset,\n" 
  "  global int* stride,\n" 
  "  global float *index_data, int index_offset,\n" 
  "  int src_nDim,\n" 
  "   int dim, int idx_size, int tensor_size, int size_dim, float val\n" 
  ")\n" 
  "{\n" 
  "  int thread_idx = get_group_id(0) * get_local_size(0) * get_local_size(1) + get_local_id(1) * get_local_size(0) + get_local_id(0);\n" 
  "\n" 
  "  long flat_size = tensor_size / idx_size;\n" 
  "\n" 
  "  if (thread_idx < flat_size)\n" 
  "  {\n" 
  "    long coeff = 0;\n" 
  "    for (int i=0; i<idx_size; i++)\n" 
  "    {\n" 
  "      int leftover = thread_idx;\n" 
  "      int srcIdx = 0;\n" 
  "      for (int d=0; d<src_nDim; d++)\n" 
  "      {\n" 
  "        if (d < dim)\n" 
  "        {\n" 
  "          coeff = leftover / (stride[d] / size_dim);\n" 
  "          leftover -= coeff * (stride[d] / size_dim);\n" 
  "          srcIdx += coeff * stride[d];\n" 
  "        }\n" 
  "        else if (d > dim)\n" 
  "        {\n" 
  "          coeff = leftover / stride[d];\n" 
  "          leftover -= coeff * stride[d];\n" 
  "          srcIdx += coeff * stride[d];\n" 
  "        }\n" 
  "      }\n" 
  "      tensor_data[tensor_offset + srcIdx + (int)((index_data[index_offset + i])-1)*stride[dim] ] = val;\n" 
  "    }\n" 
  "  }\n" 
  "}\n" 
  "\n" 
  "kernel void THClTensor_kernel_indexCopy(\n" 
  "   global float *res_data, long res_offset,\n" 
  "   global float *src_data, long src_offset,\n" 
  "   global long* res_stride, global float *index_data, long index_offset,\n" 
  "   long res_nDim, int dim, long idx_size, long src_size, long size_dim\n" 
  ")\n" 
  "{\n" 
  "  int thread_idx = get_group_id(0) * get_local_size(0) * get_local_size(1) + get_local_id(1) * get_local_size(0) + get_local_id(0);\n" 
  "\n" 
  "  long flat_size = src_size / idx_size;\n" 
  "\n" 
  "  if (thread_idx < flat_size)\n" 
  "  {\n" 
  "    long coeff = 0;\n" 
  "    for (int i=0; i<idx_size; i++)\n" 
  "    {\n" 
  "      int leftover = thread_idx;\n" 
  "      int targetIdx = 0;\n" 
  "      int resIdx = 0;\n" 
  "      for (int d=0; d<res_nDim; d++)\n" 
  "      {\n" 
  "        if (d < dim)\n" 
  "        {\n" 
  "          long stride_d = res_stride[d] / size_dim;\n" 
  "          coeff = leftover / stride_d;\n" 
  "          leftover -= coeff * stride_d;\n" 
  "          targetIdx += coeff * stride_d * idx_size;\n" 
  "          resIdx += coeff * res_stride[d];\n" 
  "        }\n" 
  "        else if (d > dim)\n" 
  "        {\n" 
  "          coeff = leftover / res_stride[d];\n" 
  "          leftover -= coeff * res_stride[d];\n" 
  "          targetIdx += coeff * res_stride[d];\n" 
  "          resIdx += coeff * res_stride[d];\n" 
  "        }\n" 
  "      }\n" 
  "      res_data[res_offset + resIdx + ((int)(index_data[index_offset + i])-1)*res_stride[dim] ] = src_data[src_offset + targetIdx + i*res_stride[dim] ];\n" 
  "    }\n" 
  "  }\n" 
  "}\n" 
  "\n" 
  "kernel void THClTensor_kernel_indexSelect(\n" 
  "   global float *tensor_data, long tensor_offset, global float *src_data, long src_offset,\n" 
  "  global long* src_stride, global float *index_data, long index_offset,\n" 
  "   long src_nDim, int dim, long idx_size, long tensor_size, long size_dim\n" 
  ")\n" 
  "{\n" 
  "  int thread_idx = get_group_id(0) * get_local_size(0) * get_local_size(1) + get_local_id(1) * get_local_size(0) + get_local_id(0);\n" 
  "\n" 
  "  long flat_size = tensor_size / idx_size;\n" 
  "\n" 
  "  if (thread_idx < flat_size)\n" 
  "  {\n" 
  "    long coeff = 0;\n" 
  "    for (int i=0; i<idx_size; i++)\n" 
  "    {\n" 
  "      int leftover = thread_idx;\n" 
  "      int targetIdx = 0;\n" 
  "      int srcIdx = 0;\n" 
  "      for (int d=0; d<src_nDim; d++)\n" 
  "      {\n" 
  "        if (d < dim)\n" 
  "        {\n" 
  "          long stride_d = src_stride[d] / size_dim;\n" 
  "          coeff = leftover / stride_d;\n" 
  "          leftover -= coeff * stride_d;\n" 
  "          targetIdx += coeff * stride_d * idx_size;\n" 
  "          srcIdx += coeff * src_stride[d];\n" 
  "        }\n" 
  "        else if (d > dim)\n" 
  "        {\n" 
  "          coeff = leftover / src_stride[d];\n" 
  "          leftover -= coeff * src_stride[d];\n" 
  "          targetIdx += coeff * src_stride[d];\n" 
  "          srcIdx += coeff * src_stride[d];\n" 
  "        }\n" 
  "      }\n" 
  "      tensor_data[tensor_offset + targetIdx + i*src_stride[dim] ] = src_data[src_offset + srcIdx + ((int)(index_data[index_offset + i])-1)*src_stride[dim] ];\n" 
  "    }\n" 
  "  }\n" 
  "}\n" 
  "\n" 
  "";
  // [[[end]]]
  return kernelSource;
}

