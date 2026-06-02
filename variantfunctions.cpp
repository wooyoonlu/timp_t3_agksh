#include "variantfunctions.h"

#include <QCryptographicHash>
#include <QHash>
#include <QSet>
#include <QVector>
#include <QtGlobal>

#include <cmath>
#include <limits>

QString VariantFunctions::vigenereEncrypt(const QString &text,
                                          const QString &key,
                                          QString *error)
{
    return vigenereTransform(text, key, 1, error);
}

QString VariantFunctions::vigenereDecrypt(const QString &text,
                                          const QString &key,
                                          QString *error)
{
    return vigenereTransform(text, key, -1, error);
}

QString VariantFunctions::vigenereTransform(const QString &text,
                                            const QString &key,
                                            int direction,
                                            QString *error)
{
    if (error) {
        error->clear();
    }

    if (key.isEmpty()) {
        if (error) {
            *error = "key must not be empty";
        }
        return {};
    }

    QVector<int> shifts;
    for (const QChar character : key) {
        const ushort code = character.toLower().unicode();
        if (code < 'a' || code > 'z') {
            if (error) {
                *error = "key must contain Latin letters only";
            }
            return {};
        }
        shifts.append(code - 'a');
    }

    QString result;
    result.reserve(text.size());
    int keyIndex = 0;

    for (const QChar character : text) {
        const ushort code = character.unicode();
        const bool isUppercase = code >= 'A' && code <= 'Z';
        const bool isLowercase = code >= 'a' && code <= 'z';

        if (!isUppercase && !isLowercase) {
            result.append(character);
            continue;
        }

        const ushort base = isUppercase ? 'A' : 'a';
        const int offset = code - base;
        const int shift = shifts.at(keyIndex % shifts.size());
        const int transformed = (offset + direction * shift + 26) % 26;
        result.append(QChar(base + transformed));
        ++keyIndex;
    }

    return result;
}

QString VariantFunctions::sha512(const QString &text)
{
    return QCryptographicHash::hash(text.toUtf8(), QCryptographicHash::Sha512)
        .toHex();
}

VariantFunctions::BisectionResult VariantFunctions::bisection(double left,
                                                              double right,
                                                              double epsilon)
{
    BisectionResult result;

    if (!std::isfinite(left) || !std::isfinite(right)
        || !std::isfinite(epsilon)) {
        result.error = "all numbers must be finite";
        return result;
    }

    if (left >= right) {
        result.error = "left boundary must be less than right boundary";
        return result;
    }

    if (epsilon <= 0.0) {
        result.error = "epsilon must be greater than zero";
        return result;
    }

    double leftValue = equation(left);
    const double rightValue = equation(right);
    if (leftValue == 0.0) {
        result.success = true;
        result.root = left;
        return result;
    }
    if (rightValue == 0.0) {
        result.success = true;
        result.root = right;
        return result;
    }
    if ((leftValue < 0.0) == (rightValue < 0.0)) {
        result.error = "function values at interval boundaries must have "
                       "different signs";
        return result;
    }

    const int maxIterations = 10000;
    while ((right - left) / 2.0 > epsilon
           && result.iterations < maxIterations) {
        const double middle = (left + right) / 2.0;
        const double middleValue = equation(middle);

        if (middleValue == 0.0) {
            left = middle;
            right = middle;
            break;
        }

        if ((leftValue < 0.0) != (middleValue < 0.0)) {
            right = middle;
        } else {
            left = middle;
            leftValue = middleValue;
        }
        ++result.iterations;
    }

    if (result.iterations == maxIterations) {
        result.error = "iteration limit exceeded";
        return result;
    }

    result.success = true;
    result.root = (left + right) / 2.0;
    return result;
}

double VariantFunctions::equation(double value)
{
    return value * value * value - value - 2.0;
}

VariantFunctions::ShortestPathResult VariantFunctions::shortestPath(
    const QString &start,
    const QString &finish,
    const QStringList &edgeDefinitions)
{
    struct Edge
    {
        QString destination;
        double weight = 0.0;
    };

    ShortestPathResult result;
    if (start.trimmed().isEmpty() || finish.trimmed().isEmpty()) {
        result.error = "start and finish vertices must not be empty";
        return result;
    }

    QHash<QString, QVector<Edge>> graph;
    graph.insert(start, {});
    graph.insert(finish, {});

    for (const QString &edgeDefinition : edgeDefinitions) {
        const QStringList parts = edgeDefinition.split(',', Qt::KeepEmptyParts);
        if (parts.size() != 3) {
            result.error = "each edge must use the format from,to,weight";
            return result;
        }

        const QString from = parts.at(0).trimmed();
        const QString to = parts.at(1).trimmed();
        bool weightOk = false;
        const double weight = parts.at(2).trimmed().toDouble(&weightOk);

        if (from.isEmpty() || to.isEmpty()) {
            result.error = "vertex names must not be empty";
            return result;
        }
        if (!weightOk || !std::isfinite(weight) || weight < 0.0) {
            result.error = "edge weight must be a non-negative number";
            return result;
        }

        graph[from].append({to, weight});
        graph[to].append({from, weight});
    }

    const double infinity = std::numeric_limits<double>::infinity();
    QHash<QString, double> distances;
    QHash<QString, QString> previous;
    QSet<QString> visited;

    for (auto iterator = graph.cbegin(); iterator != graph.cend(); ++iterator) {
        distances.insert(iterator.key(), infinity);
    }
    distances[start] = 0.0;

    while (visited.size() < graph.size()) {
        QString current;
        double currentDistance = infinity;

        for (auto iterator = distances.cbegin();
             iterator != distances.cend();
             ++iterator) {
            if (!visited.contains(iterator.key())
                && iterator.value() < currentDistance) {
                current = iterator.key();
                currentDistance = iterator.value();
            }
        }

        if (current.isEmpty() || current == finish) {
            break;
        }

        visited.insert(current);
        for (const Edge &edge : graph.value(current)) {
            const double alternative = currentDistance + edge.weight;
            if (alternative < distances.value(edge.destination, infinity)) {
                distances[edge.destination] = alternative;
                previous[edge.destination] = current;
            }
        }
    }

    if (!std::isfinite(distances.value(finish, infinity))) {
        result.error = "no path between the requested vertices";
        return result;
    }

    QString current = finish;
    while (!current.isEmpty()) {
        result.path.prepend(current);
        if (current == start) {
            break;
        }
        current = previous.value(current);
    }

    if (result.path.isEmpty() || result.path.first() != start) {
        result.error = "no path between the requested vertices";
        result.path.clear();
        return result;
    }

    result.success = true;
    result.distance = distances.value(finish);
    return result;
}
