#ifndef VARIANTFUNCTIONS_H
#define VARIANTFUNCTIONS_H

#include <QString>
#include <QStringList>

/**
 * @brief Algorithms assigned to variant 3.
 */
class VariantFunctions
{
public:
    /**
     * @brief Encrypts Latin letters with the Vigenere cipher.
     *
     * Letter case is preserved. Characters outside the Latin alphabet are
     * copied unchanged and do not advance the key position.
     *
     * @param text Plain text.
     * @param key Key containing Latin letters only.
     * @param error Receives a validation error, if any.
     * @return Cipher text or an empty string when validation fails.
     */
    static QString vigenereEncrypt(const QString &text,
                                   const QString &key,
                                   QString *error);

    /**
     * @brief Decrypts Latin letters encrypted by the Vigenere cipher.
     * @param text Cipher text.
     * @param key Key containing Latin letters only.
     * @param error Receives a validation error, if any.
     * @return Plain text or an empty string when validation fails.
     */
    static QString vigenereDecrypt(const QString &text,
                                   const QString &key,
                                   QString *error);

    /**
     * @brief Calculates a SHA-512 digest.
     * @param text Input text encoded as UTF-8.
     * @return Lowercase hexadecimal digest.
     */
    static QString sha512(const QString &text);

    /**
     * @brief Result of root search using the bisection method.
     */
    struct BisectionResult
    {
        bool success = false;
        double root = 0.0;
        int iterations = 0;
        QString error;
    };

    /**
     * @brief Finds a root of x^3 - x - 2 on an interval.
     * @param left Left interval boundary.
     * @param right Right interval boundary.
     * @param epsilon Desired absolute precision.
     * @return Root search result.
     */
    static BisectionResult bisection(double left,
                                     double right,
                                     double epsilon);

    /**
     * @brief Result of a shortest-path search.
     */
    struct ShortestPathResult
    {
        bool success = false;
        double distance = 0.0;
        QStringList path;
        QString error;
    };

    /**
     * @brief Uses Dijkstra's algorithm on an undirected weighted graph.
     *
     * Each edge must use the format `from,to,weight`, for example `A,B,2.5`.
     *
     * @param start Start vertex name.
     * @param finish Finish vertex name.
     * @param edgeDefinitions Graph edges.
     * @return Search result with total distance and vertex sequence.
     */
    static ShortestPathResult shortestPath(
        const QString &start,
        const QString &finish,
        const QStringList &edgeDefinitions);

private:
    static QString vigenereTransform(const QString &text,
                                     const QString &key,
                                     int direction,
                                     QString *error);
    static double equation(double value);
};

#endif // VARIANTFUNCTIONS_H
