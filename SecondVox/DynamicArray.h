#pragma once

#include <cstdint>
#include <memory>

#include "Macros.h"

namespace vox::data
{

#define MEM_ALLOC(size) (malloc(size))
#define MEM_REALLOC(data, size) (realloc((data), (size)))
#define MEM_DELETE(ptr) (free(ptr))

class DynamicArray {
private:
    int32_t size_mult_;
    int32_t capacity_mult_;
    unsigned char *data_;
public:
    M_FORCE_INLINE int32_t GetRawSize() const { return size_mult_; }
    M_FORCE_INLINE int32_t GetRawCapacity() const { return capacity_mult_; }
    M_FORCE_INLINE void *GetRawDataPtr() const { return data_; }
    M_FORCE_INLINE void *GetRawDataEndPtr() const { return data_ + size_mult_; }
    M_FORCE_INLINE DynamicArray():
        size_mult_(0), capacity_mult_(0), data_(nullptr)
    {}
    M_FORCE_INLINE DynamicArray(int32_t initial_raw_capacity):
        size_mult_(0), capacity_mult_(initial_raw_capacity),
        data_((unsigned char*)MEM_ALLOC(initial_raw_capacity))
    {}
    M_FORCE_INLINE DynamicArray(const void* data, int32_t raw_size):
        size_mult_(raw_size), capacity_mult_(raw_size),
        data_((unsigned char*)MEM_ALLOC(raw_size))
    {
        memcpy(data_, data, raw_size);
    }
    M_FORCE_INLINE void Init(int32_t initial_raw_capacity)
    {
        size_mult_ = 0;
        capacity_mult_ = initial_raw_capacity;
        data_ = (unsigned char*)MEM_ALLOC(initial_raw_capacity);
    }
    M_FORCE_INLINE void Init(const void* data, int32_t raw_size)
    {
        size_mult_ = raw_size;
        capacity_mult_ = raw_size;
        data_ = (unsigned char*)MEM_ALLOC(raw_size);
        memcpy(data_, data, raw_size);
    }
    void Reserve(int32_t raw_capacity)
    {
        if (capacity_mult_ < raw_capacity)
            data_ = (unsigned char*)MEM_REALLOC(data_, raw_capacity);
    }
    M_FORCE_INLINE void Clear()
    {
        size_mult_ = 0;
    }
    M_FORCE_INLINE void ClearStrict()
    {
        MEM_DELETE(data_);
        size_mult_ = 0;
        capacity_mult_ = 0;
        data_ = nullptr;
    }

    template<typename T>
    M_FORCE_INLINE void PushBack_CapacityEnsured(const T& data)
    {
        *(T*)(data_ + size_mult_) = data;
        size_mult_ += sizeof(T);
    }
    template<typename T>
    M_FORCE_INLINE void PushBack(const T& data)
    {
        if (capacity_mult_ < size_mult_ + sizeof(T))
            data_ = MEM_REALLOC(data_, capacity_mult_ << 1);
        this->PushBack_CapacityEnsured(data);
    }
    template<typename T>
    M_FORCE_INLINE void PopBack()
    {
        size_mult_ -= sizeof(T);
    }
    template<typename T>
    M_FORCE_INLINE const T& GetBack() const
    {
        return *(const T*)(data_ + size_mult_ - sizeof(T));
    }
    template<typename T>
    M_FORCE_INLINE T& GetBack()
    {
        return const_cast<T&>(static_cast<const DynamicArray*>(this)->GetBack<T>());
    }
    template<typename T>
    M_FORCE_INLINE const T& Get(int32_t index) const
    {
        return *(const T*)(data_ + index * sizeof(T));
    }
    template<typename T>
    M_FORCE_INLINE const T& operator[](int32_t index) const
    {
        return this->Get(index);
    }
};

#undef MEM_ALLOC
#undef MEM_REALLOC
#undef MEM_DELETE


