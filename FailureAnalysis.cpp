#include "FailureAnalysis.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

double FailureAnalysis::calculateSharpness(const cv::Mat& gray) {
    cv::Mat lap;
    cv::Laplacian(gray, lap, CV_64F);
    cv::Scalar mean;
    cv::Scalar stddev;
    cv::meanStdDev(lap, mean, stddev);
    return stddev[0];
}

double FailureAnalysis::calculateNoiseLevel(const cv::Mat& gray) {
    cv::Mat blurred;
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 0);
    cv::Mat diff;
    cv::absdiff(gray, blurred, diff);
    return cv::mean(diff)[0];
}

// --- Дружественная функция ---
FailureAnalysis analyzeDecodingFailure(const BarcodeReader& /*decoder*/,
                                       const cv::Mat& failedImage,
                                       const std::string& /*expectedResult*/) {
    FailureAnalysis analysis;
    analysis.overallScore = 0.0;
    analysis.isFixable = true;

    if (failedImage.empty()) {
        ProblemDetail problem;
        problem.type = ProblemType::IMAGE_QUALITY;
        problem.description = "Пустое изображение";
        problem.cause = "Изображение не загружено или камера не передает данные";
        problem.recommendation = "Проверьте источник изображения или перезапустите камеру";
        problem.confidence = 100.0;

        analysis.problems.push_back(problem);
        analysis.primaryProblem = problem;
        analysis.summary = "Нет изображения для анализа";
        return analysis;
    }

    cv::Mat gray;
    if (failedImage.channels() == 3)
        cv::cvtColor(failedImage, gray, cv::COLOR_BGR2GRAY);
    else
        gray = failedImage.clone();

    cv::Scalar mean;
    cv::Scalar stddev;
    cv::meanStdDev(gray, mean, stddev);
    analysis.metrics["brightness"] = mean[0];
    analysis.metrics["contrast"] = stddev[0];

    if (mean[0] < 30) {
        ProblemDetail problem{ProblemType::LIGHTING,
                              "Изображение слишком темное",
                              "Недостаточное освещение",
                              "Увеличьте яркость или экспозицию камеры",
                              (30.0 - mean[0]) / 30.0 * 100.0};
        analysis.problems.push_back(problem);
    }

    if (stddev[0] < 15) {
        ProblemDetail problem{ProblemType::IMAGE_QUALITY,
                              "Низкий контраст",
                              "Плохо различимы границы штрихов",
                              "Увеличьте контрастность",
                              (15.0 - stddev[0]) / 15.0 * 100.0};
        analysis.problems.push_back(problem);
    }

    double sharpness = FailureAnalysis::calculateSharpness(gray);
    analysis.metrics["sharpness"] = sharpness;
    if (sharpness < 10.0) {
        ProblemDetail problem{ProblemType::FOCUS_BLUR,
                              "Изображение размыто",
                              "Камера не в фокусе или движение при съемке",
                              "Настройте фокус или используйте штатив",
                              (10.0 - sharpness) / 10.0 * 100.0};
        analysis.problems.push_back(problem);
    }

    double noise = FailureAnalysis::calculateNoiseLevel(gray);
    analysis.metrics["noise"] = noise;
    if (noise > 15.0) {
        ProblemDetail problem{ProblemType::IMAGE_QUALITY,
                              "Высокий уровень шума",
                              "Высокий ISO или плохие условия",
                              "Уменьшите ISO или используйте фильтрацию",
                              noise / 30.0 * 100.0};
        analysis.problems.push_back(problem);
    }

    // --- Итоговый выбор основной проблемы ---
    std::sort(analysis.problems.begin(), analysis.problems.end());
    if (!analysis.problems.empty()) {
        analysis.primaryProblem = analysis.problems[0];
        double score = 100.0;
        for (const auto& p : analysis.problems)
            score -= p.confidence * 0.3;
        analysis.overallScore = std::max(0.0, score);
        analysis.isFixable = analysis.primaryProblem.type != ProblemType::DAMAGED_BARCODE;
        std::ostringstream ss;
        ss << "Обнаружено " << analysis.problems.size()
           << " проблем. Основная: " << analysis.primaryProblem.description;
        analysis.summary = ss.str();
    } else {
        analysis.primaryProblem = {ProblemType::UNKNOWN,
                                   "Причина неудачи не определена",
                                   "Неизвестно",
                                   "Попробуйте другой ракурс или освещение",
                                   0.0};
        analysis.overallScore = 50.0;
        analysis.isFixable = false;
        analysis.summary = "Проблемы не обнаружены, но декодирование не удалось";
    }

    return analysis;
}
