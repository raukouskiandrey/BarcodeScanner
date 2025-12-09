// ImageBuffer.h
#pragma once
#include <vector>
#include <stdexcept>

// Универсальный шаблонный контейнер-буфер
// T - тип элементов (например, cv::Mat или BarcodeResult)
// Ограничение задаётся в конструкторе (по умолчанию 10)
template<typename T>
class ImageBuffer {
private:
    std::vector<T> buffer;
    size_t maxSize; // максимальное количество элементов

public:
    // Конструктор с указанием лимита
    explicit ImageBuffer(size_t limit = 10) : maxSize(limit) {}

    // Добавление элемента
    void add(const T& item) {
        if (buffer.size() >= maxSize) {
            buffer.erase(buffer.begin()); // удаляем самый старый
        }
        buffer.push_back(item);
    }

    // Получение элемента по индексу
    T& get(size_t index) {
        if (index >= buffer.size()) {
            throw std::out_of_range("Индекс вне диапазона буфера");
        }
        return buffer[index];
    }

    // Размер буфера
    size_t size() const { return buffer.size(); }

    // Очистка буфера
    void clear() { buffer.clear(); }

    // Проверка заполненности
    bool isFull() const { return buffer.size() == maxSize; }

    // Итераторы
    typename std::vector<T>::iterator begin() { return buffer.begin(); }
    typename std::vector<T>::iterator end() { return buffer.end(); }
    typename std::vector<T>::const_iterator begin() const { return buffer.begin(); }
    typename std::vector<T>::const_iterator end() const { return buffer.end(); }

    // 🔹 Перегрузка оператора <<
    ImageBuffer<T>& operator<<(const T& item) {
        add(item);
        return *this;
    }
};
