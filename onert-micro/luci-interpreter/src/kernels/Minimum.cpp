/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd. All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Builders.h"
#include "kernels/Utils.h"

#include "kernels/BinaryOpCommon.h"

#include "PALMinimum.h"

namespace luci_interpreter
{

void configure_kernel_CircleMinimum(const circle::Operator *cur_op, BaseRuntimeGraph *runtime_graph)
{
  kernels::TISOKernel kernel(cur_op, runtime_graph);

  LUCI_INTERPRETER_CHECK(Tensor::element_type(kernel.input1()) ==
                         Tensor::element_type(kernel.input2()));
  LUCI_INTERPRETER_CHECK(Tensor::element_type(kernel.input1()) ==
                         Tensor::element_type(kernel.output()));
}

void execute_kernel_CircleMinimum(const circle::Operator *cur_op, BaseRuntimeGraph *runtime_graph)
{
  kernels::TISOKernel kernel(cur_op, runtime_graph);
  luci_interpreter::RuntimeShape input_shape1 =
    kernels::getTensorRuntimeShape(kernel.input1(), runtime_graph);
  luci_interpreter::RuntimeShape input_shape2 =
    kernels::getTensorRuntimeShape(kernel.input2(), runtime_graph);
  luci_interpreter::RuntimeShape output_shape =
    kernels::getTensorRuntimeShape(kernel.output(), runtime_graph);

  const uint8_t *input_data1 = runtime_graph->getDataByTensor(kernel.input1());
  const uint8_t *input_data2 = runtime_graph->getDataByTensor(kernel.input2());
  uint8_t *output_data = runtime_graph->getDataByTensor(kernel.output());

  assert(input_data1 != nullptr);
  assert(input_data2 != nullptr);
  assert(output_data != nullptr);

  switch (Tensor::element_type(kernel.input1()))
  {
#ifndef DIS_FLOAT
    case DataType::FLOAT32:
    {
      // check that input and output dimensions are equal
      auto AreShapesEqual = [](const luci_interpreter::RuntimeShape &input_shape1,
                               const luci_interpreter::RuntimeShape &input_shape2) -> bool {
        if (input_shape1.dimensionsCount() == input_shape2.dimensionsCount())
        {
          int N = input_shape1.dimensionsCount();
          for (int i = 0; i < N; ++i)
          {
            if (input_shape1.dims(i) != input_shape2.dims(i))
              return false;
          }
          return true;
        }
        return false;
      };
      if (AreShapesEqual(input_shape1, input_shape2))
      {
        const int flat_size = input_shape1.flatSize();
        luci_interpreter_pal::Minimum(flat_size, kernels::getTensorData<float>(input_data1),
                                      kernels::getTensorData<float>(input_data2),
                                      kernels::getTensorData<float>(output_data));
      }
      else
      {
        luci_interpreter_pal::BroadcastMinimum4DSlow(
          input_shape1, kernels::getTensorData<float>(input_data1), input_shape2,
          kernels::getTensorData<float>(input_data2), output_shape,
          kernels::getTensorData<float>(output_data));
      }
    }
    break;
#endif // DIS_FLOAT
    default:
      assert(false && "Unsupported type.");
  }
}

} // namespace luci_interpreter
