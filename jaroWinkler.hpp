#ifndef JAROWINKLER_HPP
#define JAROWINKLER_HPP

#include <string_view>
#include <algorithm>
#include <vector>

template <typename T>
double jaroDistance(std::basic_string_view<T> a, std::basic_string_view<T> b);

template <typename T>
double jaroWinklerDistance(std::basic_string_view<T> a, std::basic_string_view<T> b);


constexpr double JARO_WEIGHT_STRING_A(1.0/3.0);
constexpr double JARO_WEIGHT_STRING_B(1.0/3.0);
constexpr double JARO_WEIGHT_TRANSPOSITIONS(1.0/3.0);

constexpr size_t JARO_WINKLER_PREFIX_SIZE(4);
constexpr double JARO_WINKLER_SCALING_FACTOR(0.1);
constexpr double JARO_WINKLER_BOOST_THRESHOLD(0.7);

template <typename T>
double jaroDistance(std::basic_string_view<T> a, std::basic_string_view<T> b)
{
    // Register strings length.
    int aLength = static_cast<int>(a.size()); // TODO: ssize_t??
    int bLength = static_cast<int>(b.size()); // TODO: ssize_t??

    // If one string has null length, we return 0.
    if (aLength == 0 || bLength == 0)
    {
        return 0.0;
    }

    // Calculate max length range.
    int maxRange = std::max(0, std::max(aLength, bLength) / 2 - 1);

    // Creates 2 vectors of integers.
    std::vector<bool> aMatch(aLength, false);
    std::vector<bool> bMatch(bLength, false);

    // Calculate matching characters.
    int matchingCharacters = 0;
    for (int aIndex = 0; aIndex < aLength; ++aIndex)
    {
        // Calculate window test limits (limit inferior to 0 and superior to bLength).
        int minIndex = std::max(aIndex - maxRange, 0);
        int maxIndex = std::min(aIndex + maxRange + 1, bLength);

        if (minIndex >= maxIndex)
        {
            // No more common character because we don't have characters in b to test with characters in a.
            break;
        }

        for (int bIndex = minIndex; bIndex < maxIndex; ++bIndex)
        {
            if (!bMatch.at(bIndex) && a.at(aIndex) == b.at(bIndex))
            {
                // Found some new match.
                aMatch[aIndex] = true;
                bMatch[bIndex] = true;
                ++matchingCharacters;
                break;
            }
        }
    }

    // If no matching characters, we return 0.
    if (matchingCharacters == 0)
    {
        return 0.0;
    }

    // Calculate character transpositions.
    std::vector<int> aPosition(matchingCharacters, 0);
    std::vector<int> bPosition(matchingCharacters, 0);

    for (int aIndex = 0, positionIndex = 0; aIndex < aLength; ++aIndex)
    {
        if (aMatch.at(aIndex))
        {
            aPosition[positionIndex] = aIndex;
            ++positionIndex;
        }
    }

    for (int bIndex = 0, positionIndex = 0; bIndex < bLength; ++bIndex)
    {
        if (bMatch.at(bIndex))
        {
            bPosition[positionIndex] = bIndex;
            ++positionIndex;
        }
    }

    // Counting half-transpositions.
    int transpositions = 0;
    for (int index(0); index < matchingCharacters; ++index)
    {
        if (a.at(aPosition.at(index)) != b.at(bPosition.at(index)))
        {
            ++transpositions;
        }
    }

    // Calculate Jaro distance.
    return (
        JARO_WEIGHT_STRING_A * matchingCharacters / aLength +
        JARO_WEIGHT_STRING_B * matchingCharacters / bLength +
        JARO_WEIGHT_TRANSPOSITIONS * (matchingCharacters - transpositions / 2) / matchingCharacters
    );
}

template <typename T>
double jaroWinklerDistance(std::basic_string_view<T> a, std::basic_string_view<T> b)
{
    {
        // Calculate Jaro distance.
        double distance = jaroDistance(a, b);

        if (distance > JARO_WINKLER_BOOST_THRESHOLD)
        {
            // Calculate common string prefix.
            int commonPrefix = 0;
            for (int index = 0, indexEnd = static_cast<int>(std::min(std::min(a.size(), b.size()), JARO_WINKLER_PREFIX_SIZE)); // TODO: use ssize_t?
                 index < indexEnd; ++index)
            {
                if (a.at(index) == b.at(index))
                {
                    ++commonPrefix;
                }
                else
                {
                    break;
                }
            }

            // Calculate Jaro-Winkler distance.
            distance += JARO_WINKLER_SCALING_FACTOR * commonPrefix * (1.0 - distance);
        }

        return distance;
    }
}



#endif // JAROWINKLER_HPP

