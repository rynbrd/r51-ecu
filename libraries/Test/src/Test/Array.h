#ifndef _R51_TEST_ARRAY_H_
#define _R51_TEST_ARRAY_H_

#include <Arduino.h>

namespace R51 {

// A dynamic array.
template <typename T>
class Array {
    public:
        // Construct an array. Optionally reserve capacity.
        explicit Array(size_t reserve = 0);
        virtual ~Array();

        // Return the size of the array.
        size_t size() const { return size_; }

        // Return the capacity of the array.
        size_t capacity() const { return capacity_; }

        // Ensure the array has at least the requested capacity.
        void reserve(size_t capacity);

        // Push an element to the end of the array. Allocates additional
        // capacity if necessary.
        void push(const T& element);

        // Return a pointer to the underlying array.
        const T* data() const { return elements_; };

        // Clear the contents of the array. This resets the size of the array
        // to 0 and retains the array's existing capacity.
        void clear() { size_ = 0; };

        // Return the element at the given index.
        T& operator[](size_t i) { return elements_[i]; }

    private:
        size_t size_;
        size_t capacity_;
        T* elements_;
};

template <typename T>
Array<T>::Array(size_t reserve) :
    size_(0), capacity_(max((size_t)1,reserve)), elements_(new T[capacity_]) {}

template <typename T>
Array<T>::~Array() {
    delete[] elements_;
}

template <typename T>
void Array<T>::reserve(size_t capacity) {
    if (capacity_ >= capacity) {
        return;
    }

    T* new_elements = new T[capacity];
    for (size_t i = 0; i < size_; i++) {
        new_elements[i] = elements_[i];
    }
    delete[] elements_;
    elements_ = new_elements;
    capacity_ = capacity;
}

template <typename T>
void Array<T>::push(const T& element) {
    if (size_ == capacity_) {
        reserve(capacity_*2);
    }
    elements_[size_++] = element;
}

}  // namespace R51

#endif  // _R51_TEST_ARRAY_H_
