#pragma once

#include <cstdint>
#include <memory>
#include <iostream>

#include "Macros.h"
#include "MemoryManager.h"

namespace vox::data
{

#define MEM_ALLOC(size) (mem::MMMemAlloc(size))
#define MEM_REALLOC(data, size) (mem::MMMemRealloc((data), (size)))
#define MEM_DELETE(ptr) (mem::MMMemFree(ptr))

template<typename T>
class DynamicArray {
private:
    T *data_;
    int32_t size_;
    int32_t capacity_;
public:
    DynamicArray(const DynamicArray<T>&) = delete;
    DynamicArray<T> &operator=(const DynamicArray<T>&) = delete;
    M_FORCE_INLINE int32_t GetSize() const { return size_; }
    M_FORCE_INLINE int32_t GetCapacity() const { return capacity_; }
    M_FORCE_INLINE T *GetDataPtr() const { return data_; }
    M_FORCE_INLINE T *GetDataEndPtr() const { return data_ + size_; }
    M_FORCE_INLINE DynamicArray():
        size_(0), capacity_(0), data_(nullptr)
    {}
    M_FORCE_INLINE DynamicArray(int32_t initial_capacity):
        size_(0), capacity_(initial_capacity),
        data_((T*)MEM_ALLOC(initial_capacity * sizeof(T)))
    {}
    M_FORCE_INLINE DynamicArray(const void* data, int32_t size):
        size_(size), capacity_(size),
        data_((T*)MEM_ALLOC(size * sizeof(T)))
    {
        memcpy(data_, data, size * sizeof(T));
    }
    M_FORCE_INLINE void Init(int32_t initial_capacity)
    {
        size_ = 0;
        capacity_ = initial_capacity;
        data_ = (T*)MEM_ALLOC(initial_capacity * sizeof(T));
    }
    M_FORCE_INLINE void Init(const void* data, int32_t size)
    {
        size_ = size;
        capacity_ = size;
        data_ = (T*)MEM_ALLOC(size * sizeof(T));
        memcpy(data_, data, size * sizeof(T));
    }
    M_FORCE_INLINE void Reserve(int32_t capacity)
    {
        if (capacity_ < capacity)
        {
            data_ = (T*)MEM_REALLOC(data_, capacity * sizeof(T));
            capacity_ = capacity;
        }
    }
    M_FORCE_INLINE void Clear()
    {
        size_ = 0;
    }
    M_FORCE_INLINE void ClearStrict()
    {
        size_ = 0;
        capacity_ = 0;
        MEM_DELETE(data_);
        data_ = nullptr;
    }

    M_FORCE_INLINE void PushBack_CapacityEnsured(const T& data)
    {
        data_[size_++] = data;
    }
    M_FORCE_INLINE void PushBack(const T& data)
    {
        if (capacity_ == size_)
        {
            capacity_ *= 2;
            data_ = (T*)MEM_REALLOC(data_, capacity_ * sizeof(T));
        }
        data_[size_++] = data;
    }
    M_FORCE_INLINE T& PopBack()
    {
        return data_[size_--];
    }
    M_FORCE_INLINE const T& GetBack() const
    {
        return data_[size_ - 1];
    }
    M_FORCE_INLINE T& GetBack()
    {
        return data_[size_ - 1];
    }
    M_FORCE_INLINE const T& Get(int32_t index) const
    {
        return data_[index];
    }
    M_FORCE_INLINE T& Get(int32_t index)
    {
        return data_[index];
    }
    M_FORCE_INLINE const T& operator[](int32_t index) const
    {
        return data_[index];
    }
    M_FORCE_INLINE T& operator[](int32_t index)
    {
        return data_[index];
    }

    M_FORCE_INLINE void LoadFromFile(FILE* fp)
    {
        fread(&size_, sizeof(size_), 1, fp);
        this->Reserve(size_);
        fread(this->GetDataPtr(), sizeof(T), size_, fp);
    }

    M_FORCE_INLINE void SaveToFile(FILE* fp) const
    {
        fwrite(&size_, sizeof(size_), 1, fp);
        fwrite(this->GetDataPtr(), sizeof(T), size_, fp);
    }

    template<typename T2>
    M_FORCE_INLINE DynamicArray<T2> &ClearAndCollapse()
    {
        static_assert(sizeof(T) % sizeof(T2) == 0);
        this->Clear();
        capacity_ *= sizeof(T) / sizeof(T2);
        return *(DynamicArray<T2> *)this;
    }
};

}

#undef MEM_ALLOC
#undef MEM_REALLOC
#undef MEM_DELETE


