#pragma once
#include <string>
#include <vector>
#include <map>
#include <opencv2/opencv.hpp>
#include "BarcodeReader.h"
#include <compare>

// Тип проблемы
enum class ProblemType {
    IMAGE_QUALITY,
    DECODER_CONFIG,
    BARCODE_TYPE,
    CAMERA_ISSUE,
    LIGHTING,
    FOCUS_BLUR,
    PERSPECTIVE,
    DAMAGED_BARCODE,
    SIZE_RESOLUTION,
    UNKNOWN
};

struct ProblemDetail {
    ProblemType type;
    std::string description;
    std::string cause;
    std::string recommendation;
    double confidence;
    cv::Mat visualization;

    // Конструктор по умолчанию
    ProblemDetail() noexcept
        : type(ProblemType::UNKNOWN), confidence(0.0) {}

    // Конструктор с параметрами
    ProblemDetail(ProblemType t, std::string desc, std::string c,
                  std::string rec, double conf, cv::Mat viz = cv::Mat()) noexcept
        : type(t), description(std::move(desc)), cause(std::move(c)),
        recommendation(std::move(rec)), confidence(conf),
        visualization(std::move(viz)) {}

    // Кастомный конструктор копирования
    ProblemDetail(const ProblemDetail& other)
        : type(other.type),
        description(other.description),
        cause(other.cause),
        recommendation(other.recommendation),
        confidence(other.confidence)
    {
        // cv::Mat имеет "умное" управление памятью (copy-on-write),
        // но если нужно гарантировать отдельную копию — используем clone()
        visualization = other.visualization.empty() ? cv::Mat() : other.visualization.clone();
    }

    // Кастомный оператор присваивания копированием
    ProblemDetail& operator=(const ProblemDetail& other) {
        if (this != &other) {
            type = other.type;
            description = other.description;
            cause = other.cause;
            recommendation = other.recommendation;
            confidence = other.confidence;
            visualization = other.visualization.empty() ? cv::Mat() : other.visualization.clone();
        }
        return *this;
    }

    // Конструктор перемещения (noexcept)
    ProblemDetail(ProblemDetail&& other) noexcept = default;

    // Оператор присваивания перемещением (noexcept)
    ProblemDetail& operator=(ProblemDetail&& other) noexcept = default;

    ~ProblemDetail() {
        if (!visualization.empty()) {
            visualization.release();
        }
    }

    auto operator<=>(const ProblemDetail& other) const = default;
};


struct FailureAnalysis {
    std::vector<ProblemDetail> problems;
    ProblemDetail primaryProblem;
    double overallScore = 0.0;
    bool isFixable = false;
    std::string summary;
    std::map<std::string, double, std::less<>> metrics;

    // Конструктор по умолчанию
    FailureAnalysis() noexcept = default;

    // Кастомный конструктор копирования
    FailureAnalysis(const FailureAnalysis& other)
        : problems(other.problems), // vector копирует элементы (использует коп. конструктор ProblemDetail)
        primaryProblem(other.primaryProblem),
        overallScore(other.overallScore),
        isFixable(other.isFixable),
        summary(other.summary),
        metrics(other.metrics) {}

    // Кастомный оператор присваивания копированием
    FailureAnalysis& operator=(const FailureAnalysis& other) {
        if (this != &other) {
            problems = other.problems;          // копирование вектора
            primaryProblem = other.primaryProblem; // копирование ProblemDetail
            overallScore = other.overallScore;
            isFixable = other.isFixable;
            summary = other.summary;
            metrics = other.metrics;            // копирование map
        }
        return *this;
    }

    // Конструктор перемещения (noexcept)
    FailureAnalysis(FailureAnalysis&& other) noexcept = default;

    // Оператор присваивания перемещением (noexcept)
    FailureAnalysis& operator=(FailureAnalysis&& other) noexcept = default;

    ~FailureAnalysis() {
        problems.clear();
        metrics.clear();
        summary.clear();
    }

    // Методы для форматирования
    std::string toHtmlReport() const;
    std::string toJson() const;
    std::string toPlainText() const;

    // Методы анализа изображения
    static double calculateSharpness(const cv::Mat& gray);
    static double calculateNoiseLevel(const cv::Mat& gray);
};


class BarcodeReader;

// 👇 Дружественная функция объявляется здесь же
FailureAnalysis analyzeDecodingFailure(
    const BarcodeReader& decoder,
    const cv::Mat& failedImage,
    const std::string& expectedResult);
