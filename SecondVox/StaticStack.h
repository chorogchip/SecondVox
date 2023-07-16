#pragma once

#include "Macros.h"

namespace vox::data
{

template<typename T, int sz>
class StaticStack
{
private:
    int size_;
    T data_[sz];
public:
    M_FORCE_INLINE StaticStack(): size_(0), data_{} {}
    M_FORCE_INLINE T *GetDataPtr() const { return &data_[0]; }
    M_FORCE_INLINE void Clear() { size_ = 0; }
    M_FORCE_INLINE void PushBack(T x) { data_[size_++] = x; };
    M_FORCE_INLINE const T GetBack() const { return data_[size_ - 1]; }
    M_FORCE_INLINE void PopBack() { size_--; }
    M_FORCE_INLINE bool IsEmpty() const { return size_ == 0; }
};



}